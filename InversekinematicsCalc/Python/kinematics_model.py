"""Reusable inverse and forward kinematics for the UWE robot arm.

The equations and link dimensions are taken from InversekinematicsCalc.py.
All distances are in millimetres and angles are in radians.
"""

from dataclasses import dataclass
import math


L1 = 74.00
L2 = 145.95
L3 = 183.70
L4 = 63.00


@dataclass(frozen=True)
class ArmPose:
    base: tuple[float, float, float]
    shoulder: tuple[float, float, float]
    elbow: tuple[float, float, float]
    wrist: tuple[float, float, float]
    tool: tuple[float, float, float]
    angles: tuple[float, float, float, float]


def inverse_kinematics(x, y, z, tool_angle, elbow_branch=1):
    """Return an ArmPose, or raise ValueError when the target is unreachable."""
    if elbow_branch not in (-1, 1):
        raise ValueError("elbow_branch must be 1 or -1")

    reach = math.hypot(x, y)
    base_angle = math.atan2(x, y)
    wrist_r = reach - L4 * math.cos(tool_angle)
    wrist_z = z - L4 * math.sin(tool_angle)
    shoulder_to_wrist = math.hypot(wrist_r, wrist_z - L1)
    minimum_reach = abs(L2 - L3)
    maximum_reach = L2 + L3

    if wrist_r < 0:
        raise ValueError("The wrist would need to pass through the base axis.")
    if not minimum_reach <= shoulder_to_wrist <= maximum_reach:
        raise ValueError(
            f"Wrist distance {shoulder_to_wrist:.1f} mm is outside the valid "
            f"range {minimum_reach:.1f}–{maximum_reach:.1f} mm."
        )
    if shoulder_to_wrist < 1e-9:
        raise ValueError("The wrist is at the shoulder, so the pose is undefined.")

    midpoint_distance = (
        L2 ** 2 - L3 ** 2 + shoulder_to_wrist ** 2
    ) / (2 * shoulder_to_wrist)
    perpendicular = math.sqrt(max(0, L2 ** 2 - midpoint_distance ** 2))
    mid_axis_angle = math.atan2(wrist_z - L1, wrist_r)
    shoulder_angle = mid_axis_angle + math.atan2(
        elbow_branch * perpendicular, midpoint_distance
    )
    elbow_r = L2 * math.cos(shoulder_angle)
    elbow_z = L1 + L2 * math.sin(shoulder_angle)
    forearm_angle = math.atan2(wrist_z - elbow_z, wrist_r - elbow_r)
    elbow_angle = math.pi + forearm_angle - shoulder_angle
    wrist_angle = tool_angle + 3 * math.pi / 2 - shoulder_angle - elbow_angle

    def to_xyz(radial, height):
        return (radial * math.sin(base_angle), radial * math.cos(base_angle), height)

    return ArmPose(
        base=(0.0, 0.0, 0.0),
        shoulder=(0.0, 0.0, L1),
        elbow=to_xyz(elbow_r, elbow_z),
        wrist=to_xyz(wrist_r, wrist_z),
        tool=(x, y, z),
        angles=(base_angle + math.pi / 2, shoulder_angle, elbow_angle, wrist_angle),
    )


def radial_reach(x, y):
    """Return horizontal distance from the base axis in millimetres."""
    return math.hypot(x, y)
