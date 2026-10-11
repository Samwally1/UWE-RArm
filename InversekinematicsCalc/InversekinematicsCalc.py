import sympy as sp
import math
import matplotlib.pyplot as plt

print()
print("Initialising Arm Lengths - l1, l2, l3, l4")
print()

l1 = sp.Symbol("l1")
l2 = sp.Symbol("l2")
l3 = sp.Symbol("l3")
l4 = sp.Symbol("l4")


print("Initialising Servo Angles - s1, s2, s3, s4")
print()

s1 = sp.Symbol("s1")
s2 = sp.Symbol("s2")
s3 = sp.Symbol("s3")
s4 = sp.Symbol("s4")


print("Input Variables - x, y, z, θ")
print()

x = sp.Symbol("x")
y = sp.Symbol("y")
z = sp.Symbol("z")
θ = sp.Symbol("θ")


# Substitute 1 or -1 to select between the two elbow positions.
Elbow_Branch = sp.Symbol("Elbow_Branch", real=True)


print("First Calculate Rotating Base Angle (s1)")
print()

s1_Calc = sp.Eq(s1, sp.atan2(x, y) + sp.pi/2)

sp.pprint(s1_Calc)
print()


print("As we have now calculated the rotation, we will calculate the reach of the arm\n"
      "and create a new axis along the arm span (r) to simplify calculations")
print()

Reach = sp.Symbol("Reach")

Reach_Calc = sp.Eq(Reach, sp.sqrt(x**2 + y**2))

sp.pprint(Reach_Calc)
print()


print("The coordinates of the wrist are directly driven by the inputs x, y, z, and θ\n"
      "We can therefore calculate the wrist coordinates (Wrist_r, Wrist_z)\n"
      "by subtracting the horizontal and vertical components of the final link")
print()

Wrist_r = sp.Symbol("Wrist_r")
Wrist_z = sp.Symbol("Wrist_z")

Wrist_r_Calc = sp.Eq(Wrist_r, Reach - (l4*sp.cos(θ)))

sp.pprint(Wrist_r_Calc)
print()

Wrist_z_Calc = sp.Eq(Wrist_z, z - (l4*sp.sin(θ)))

sp.pprint(Wrist_z_Calc)
print()


print("Calculating the straight-line distance from the shoulder to the wrist\n"
      "The shoulder is at r = 0 and z = l1")
print()

Shoulder_Wrist_Distance = sp.Symbol("Shoulder_Wrist_Distance")

Shoulder_Wrist_Distance_Calc = sp.Eq(
    Shoulder_Wrist_Distance,
    sp.sqrt(Wrist_r**2 + (Wrist_z - l1)**2)
)

sp.pprint(Shoulder_Wrist_Distance_Calc)
print()


print("Finding the point along the shoulder-to-wrist line\n"
      "where the perpendicular line through the elbow crosses it\n"
      "This is halfway along the line when l2 and l3 are equal")
print()

Midpoint_Distance = sp.Symbol("Midpoint_Distance")

Midpoint_Distance_Calc = sp.Eq(
    Midpoint_Distance,
    (l2**2 - l3**2 + Shoulder_Wrist_Distance**2)
    / (2 * Shoulder_Wrist_Distance)
)

sp.pprint(Midpoint_Distance_Calc)
print()


print("Calculating the coordinates of this crossing point")
print()

Midpoint_s1_wrist_r = sp.Symbol("Midpoint_s1_wrist_r")
Midpoint_s1_wrist_z = sp.Symbol("Midpoint_s1_wrist_z")

Midpoint_s1_wrist_r_Calc = sp.Eq(
    Midpoint_s1_wrist_r,
    Midpoint_Distance * Wrist_r / Shoulder_Wrist_Distance
)

sp.pprint(Midpoint_s1_wrist_r_Calc)
print()

Midpoint_s1_wrist_z_Calc = sp.Eq(
    Midpoint_s1_wrist_z,
    l1 + Midpoint_Distance * (Wrist_z - l1) / Shoulder_Wrist_Distance
)

sp.pprint(Midpoint_s1_wrist_z_Calc)
print()


print("Calculating the perpendicular distance from the crossing point to the elbow\n"
      "Use Pythagoras with l2 as the hypotenuse")
print()

BisectorLengh = sp.Symbol("BisectorLengh")

BisectorLengh_Calc = sp.Eq(
    BisectorLengh,
    sp.sqrt(l2**2 - Midpoint_Distance**2)
)

sp.pprint(BisectorLengh_Calc)
print()


print("Calculating the angle from the shoulder to the wrist")
print()

MidAxis_Angle = sp.Symbol("MidAxis_Angle")

MidAxis_Angle_Calc = sp.Eq(
    MidAxis_Angle,
    sp.atan2(Wrist_z - l1, Wrist_r)
)

sp.pprint(MidAxis_Angle_Calc)
print()


print("Calculating the shoulder servo angle (s2)\n"
      "Add the elbow offset angle to the shoulder-to-wrist angle")
print()

s2_Calc = sp.Eq(
    s2,
    MidAxis_Angle + sp.atan2(Elbow_Branch * BisectorLengh, Midpoint_Distance)
)

sp.pprint(s2_Calc)
print()


print("Calculating the elbow coordinates\n"
      "Move l2 from the shoulder at the calculated shoulder angle")
print()

Elbow_r = sp.Symbol("Elbow_r")
Elbow_z = sp.Symbol("Elbow_z")

Elbow_r_Calc = sp.Eq(Elbow_r, l2 * sp.cos(s2))

sp.pprint(Elbow_r_Calc)
print()

Elbow_z_Calc = sp.Eq(Elbow_z, l1 + l2 * sp.sin(s2))

sp.pprint(Elbow_z_Calc)
print()


print("Calculating the forearm angle relative to horizontal\n"
      "Use the difference between the wrist coordinates and elbow coordinates")
print()

Forearm_Angle = sp.Symbol("Forearm_Angle")

Forearm_Angle_Calc = sp.Eq(Forearm_Angle, sp.atan2(
    Wrist_z - Elbow_z,
    Wrist_r - Elbow_r
))

sp.pprint(Forearm_Angle_Calc)
print()


print("Calculating the elbow servo angle (s3)\n"
      "Subtract the shoulder angle from the forearm angle and add the 180 degree offset")
print()

s3_Calc = sp.Eq(s3, sp.pi + Forearm_Angle - s2)

sp.pprint(s3_Calc)
print()


print("As one of the inputs is θ for the angle of the final limb relative to the horizon\n"
      "we can work out s4 based on the other servo angles and θ")
print()

s4_Calc = sp.Eq(s4, θ + 3*sp.pi/2 - s2 - s3)

sp.pprint(s4_Calc)
print()


print("Compiling the equations\n"
      "Substitute the arm lengths and intermediate equations")
print()

# Arm lengths in millimetres, converted from the measurements in metres.
# Use millimetres for x, y and z as well.
Length_Values = {
    l1: 74.00,
    l2: 145.95,
    l3: 183.70,
    l4: 53.00,
}

Calculation_Order = [
    Reach_Calc,
    Wrist_r_Calc,
    Wrist_z_Calc,
    Shoulder_Wrist_Distance_Calc,
    Midpoint_Distance_Calc,
    Midpoint_s1_wrist_r_Calc,
    Midpoint_s1_wrist_z_Calc,
    BisectorLengh_Calc,
    MidAxis_Angle_Calc,
    s1_Calc,
    s2_Calc,
    Elbow_r_Calc,
    Elbow_z_Calc,
    Forearm_Angle_Calc,
    s3_Calc,
    s4_Calc,
]

Substitutions = Length_Values.copy()

print("// Copy the include and M_PI definition to the top of your C file.")
print("#include <math.h>")
print("#ifndef M_PI")
print("#define M_PI 3.14159265358979323846")
print("#endif")
print()
print("// Put these assignments inside your function.")
print("// Define x, y, z, theta and Elbow_Branch beforehand.")
print("// Elbow_Branch must be 1 or -1. All angles are in radians.")
print()

for Equation in Calculation_Order:

    Substitutions[Equation.lhs] = Equation.rhs.xreplace(Substitutions)

    Expression = Equation.rhs.subs(Length_Values)

    # Use an ASCII variable name in the generated C code.
    Expression = Expression.subs(θ, sp.Symbol("theta"))

    C_Code = sp.ccode(Expression, assign_to=str(Equation.lhs))

    print("double " + C_Code)

print()

# Keep these for the numerical calculations later.

s1_Compiled = sp.Eq(s1, Substitutions[s1])
s2_Compiled = sp.Eq(s2, Substitutions[s2])
s3_Compiled = sp.Eq(s3, Substitutions[s3])
s4_Compiled = sp.Eq(s4, Substitutions[s4])

# ---------------------------------------------------------------------------
# Forward kinematics: servo angles -> tool position and tool angle
# ---------------------------------------------------------------------------

print("Forward kinematics")
print("Calculate x, y, z and the tool angle from s1, s2, s3 and s4")
print("All input servo angles are in radians; positions are in millimetres.")
print()

FK_Base_Angle = sp.Symbol("FK_Base_Angle")
FK_Forearm_Angle = sp.Symbol("FK_Forearm_Angle")
FK_Tool_Angle = sp.Symbol("FK_Tool_Angle")
FK_Elbow_r = sp.Symbol("FK_Elbow_r")
FK_Elbow_z = sp.Symbol("FK_Elbow_z")
FK_Wrist_r = sp.Symbol("FK_Wrist_r")
FK_Wrist_z = sp.Symbol("FK_Wrist_z")
FK_Tool_r = sp.Symbol("FK_Tool_r")
FK_x = sp.Symbol("FK_x")
FK_y = sp.Symbol("FK_y")
FK_z = sp.Symbol("FK_z")

print("Remove the servo offsets to find the absolute link angles")
print()

FK_Base_Angle_Calc = sp.Eq(FK_Base_Angle, s1 - sp.pi/2)
FK_Forearm_Angle_Calc = sp.Eq(FK_Forearm_Angle, s2 + s3 - sp.pi)
FK_Tool_Angle_Calc = sp.Eq(FK_Tool_Angle, s2 + s3 + s4 - 3*sp.pi/2)

for Equation in [FK_Base_Angle_Calc, FK_Forearm_Angle_Calc, FK_Tool_Angle_Calc]:

    sp.pprint(Equation)
    print()

print("Move from the shoulder to the elbow using the upper-arm length")
print()

FK_Elbow_r_Calc = sp.Eq(FK_Elbow_r, l2 * sp.cos(s2))
FK_Elbow_z_Calc = sp.Eq(FK_Elbow_z, l1 + l2 * sp.sin(s2))

for Equation in [FK_Elbow_r_Calc, FK_Elbow_z_Calc]:

    sp.pprint(Equation)
    print()

print("Move from the elbow to the wrist using the forearm length")
print()

FK_Wrist_r_Calc = sp.Eq(FK_Wrist_r, FK_Elbow_r + l3 * sp.cos(FK_Forearm_Angle))
FK_Wrist_z_Calc = sp.Eq(FK_Wrist_z, FK_Elbow_z + l3 * sp.sin(FK_Forearm_Angle))

for Equation in [FK_Wrist_r_Calc, FK_Wrist_z_Calc]:

    sp.pprint(Equation)
    print()

print("Move from the wrist to the tool tip using the final link length")
print()

FK_Tool_r_Calc = sp.Eq(FK_Tool_r, FK_Wrist_r + l4 * sp.cos(FK_Tool_Angle))
FK_z_Calc = sp.Eq(FK_z, FK_Wrist_z + l4 * sp.sin(FK_Tool_Angle))

for Equation in [FK_Tool_r_Calc, FK_z_Calc]:

    sp.pprint(Equation)
    print()

print("Rotate the radial position into x and y using the base angle")
print()

FK_x_Calc = sp.Eq(FK_x, FK_Tool_r * sp.sin(FK_Base_Angle))
FK_y_Calc = sp.Eq(FK_y, FK_Tool_r * sp.cos(FK_Base_Angle))

for Equation in [FK_x_Calc, FK_y_Calc]:

    sp.pprint(Equation)
    print()

Forward_Calculation_Order = [
    FK_Base_Angle_Calc,
    FK_Forearm_Angle_Calc,
    FK_Tool_Angle_Calc,
    FK_Elbow_r_Calc,
    FK_Elbow_z_Calc,
    FK_Wrist_r_Calc,
    FK_Wrist_z_Calc,
    FK_Tool_r_Calc,
    FK_z_Calc,
    FK_x_Calc,
    FK_y_Calc,
]

# Keep forward substitutions separate so the servo angles remain inputs.
Forward_Substitutions = Length_Values.copy()

print("Compiling the forward equations as C")
print()
print("// Use the math.h include and M_PI definition printed above.")
print("// Put these assignments inside your forward-kinematics function.")
print("// Define s1, s2, s3 and s4 beforehand, in radians.")
print("// Outputs: FK_x, FK_y, FK_z in mm; FK_Tool_Angle in radians.")
print()

for Equation in Forward_Calculation_Order:

    Forward_Substitutions[Equation.lhs] = Equation.rhs.xreplace(Forward_Substitutions)

    Expression = Equation.rhs.subs(Length_Values)
    C_Code = sp.ccode(Expression, assign_to=str(Equation.lhs))

    print("double " + C_Code)

print()

FK_x_Compiled = sp.Eq(FK_x, Forward_Substitutions[FK_x])
FK_y_Compiled = sp.Eq(FK_y, Forward_Substitutions[FK_y])
FK_z_Compiled = sp.Eq(FK_z, Forward_Substitutions[FK_z])
FK_Tool_Angle_Compiled = sp.Eq(FK_Tool_Angle, Forward_Substitutions[FK_Tool_Angle])

# ---------------------------------------------------------------------------
# Run the calculations with example inputs
# ---------------------------------------------------------------------------

print("Calculating servo angles for the example inputs")
print()

Input_Values = {
    Elbow_Branch: 1,
    **Length_Values,
    x: 11.71,
    y: 133.85,
    z: 114.88,
    θ: 2,
}

# θ is in radians.
# For example, use -sp.pi/2 for -90 degrees.


def Get_Value(Expression):

    Value = complex(
        Expression.xreplace(Substitutions).subs(Input_Values).evalf()
    )

    if not math.isfinite(Value.real) or not math.isfinite(Value.imag):

        raise ValueError("The calculation produced an undefined value.")

    if abs(Value.imag) > 0.000001:

        raise ValueError(
            "The calculation produced complex coordinates. "
            "Check whether the target is reachable."
        )

    return Value.real


try:

    if any(float(Input_Values[Length]) <= 0 for Length in (l1, l2, l3, l4)):

        raise ValueError("All arm lengths must be greater than zero.")

    Distance_Value = Get_Value(Shoulder_Wrist_Distance)

    Minimum_Reach = abs(float(Input_Values[l2] - Input_Values[l3]))
    Maximum_Reach = float(Input_Values[l2] + Input_Values[l3])

    if Distance_Value < Minimum_Reach or Distance_Value > Maximum_Reach:

        raise ValueError(
            f"The wrist is unreachable: shoulder-to-wrist distance "
            f"is {Distance_Value:.3f} mm, but must be between "
            f"{Minimum_Reach:.3f} and {Maximum_Reach:.3f} mm."
        )

    if Distance_Value < 0.000000001:

        raise ValueError(
            "The wrist is at the shoulder. "
            "This special position does not have a unique shoulder angle."
        )

    Servo_Angles = {}

    for Equation in [s1_Compiled, s2_Compiled, s3_Compiled, s4_Compiled]:

        Angle_Radians = Get_Value(Equation.rhs)
        Angle_Degrees = math.degrees(Angle_Radians)

        Servo_Angles[Equation.lhs] = Angle_Radians

        print(f"{Equation.lhs} = {Angle_Degrees:.6f} degrees")
        print()

    print("These are geometric angles using your servo offsets.")
    print("They may be outside the movement limits of your physical servos.")
    print()


    print("Forward calculation using the resulting servo angles")
    print()

    for Equation in [FK_x_Compiled, FK_y_Compiled, FK_z_Compiled, FK_Tool_Angle_Compiled]:

        # Substitute servo values directly, without the inverse substitutions.
        Value = float(Equation.rhs.subs(Servo_Angles).evalf())

        if Equation.lhs == FK_Tool_Angle:

            print(f"Tool angle = {math.degrees(Value):.6f} degrees")

        else:

            print(f"{Equation.lhs} = {Value:.6f} mm")

    print()

    # -----------------------------------------------------------------------
    # Visualise the calculated arm in 3D
    # -----------------------------------------------------------------------

    print("Creating a 3D visualisation of the arm")
    print()

    # At x = y = 0, base rotation is undefined by the target alone.
    # Choose 0 here so the arm can still be plotted.

    if float(Input_Values[x]) == 0 and float(Input_Values[y]) == 0:

        Base_Angle = 0.0

    else:

        Base_Angle = Servo_Angles[s1] - math.pi/2

    Shoulder_Height = float(Input_Values[l1])

    # Reconstruct the arm from the calculated servo angles.
    # This also checks that the angles place the tool at the target.

    Upper_Arm_Angle = Servo_Angles[s2]

    Forearm_Angle_Value = (
        Servo_Angles[s2] + Servo_Angles[s3] - math.pi
    )

    Tool_Angle = (
        Servo_Angles[s2] + Servo_Angles[s3] + Servo_Angles[s4] - 3*math.pi/2
    )

    Elbow_R = float(Input_Values[l2]) * math.cos(Upper_Arm_Angle)
    Elbow_Z = Shoulder_Height + float(Input_Values[l2]) * math.sin(Upper_Arm_Angle)

    Wrist_R = Elbow_R + float(Input_Values[l3]) * math.cos(Forearm_Angle_Value)
    Wrist_Z = Elbow_Z + float(Input_Values[l3]) * math.sin(Forearm_Angle_Value)

    Tool_R = Wrist_R + float(Input_Values[l4]) * math.cos(Tool_Angle)
    Tool_Z = Wrist_Z + float(Input_Values[l4]) * math.sin(Tool_Angle)

    Target_X = float(Input_Values[x])
    Target_Y = float(Input_Values[y])
    Target_Z = float(Input_Values[z])

    # Convert the radial coordinates back into x and y coordinates.

    Elbow_X = Elbow_R * math.sin(Base_Angle)
    Elbow_Y = Elbow_R * math.cos(Base_Angle)

    Wrist_X = Wrist_R * math.sin(Base_Angle)
    Wrist_Y = Wrist_R * math.cos(Base_Angle)

    Tool_X = Tool_R * math.sin(Base_Angle)
    Tool_Y = Tool_R * math.cos(Base_Angle)

    Position_Error = math.sqrt(
        (Tool_X - Target_X)**2
        + (Tool_Y - Target_Y)**2
        + (Tool_Z - Target_Z)**2
    )

    print(f"Calculated tool position: {Tool_X:.3f}, {Tool_Y:.3f}, {Tool_Z:.3f} mm")
    print(f"Distance from target: {Position_Error:.9f} mm")
    print()

    Points_X = [0, 0, Elbow_X, Wrist_X, Tool_X]
    Points_Y = [0, 0, Elbow_Y, Wrist_Y, Tool_Y]
    Points_Z = [0, Shoulder_Height, Elbow_Z, Wrist_Z, Tool_Z]

    Point_Names = ["Base", "Shoulder", "Elbow", "Wrist", "Tool tip"]

    Figure = plt.figure()
    Axis = Figure.add_subplot(111, projection="3d")

    Axis.plot(
        Points_X,
        Points_Y,
        Points_Z,
        marker="o",
        linewidth=4,
        markersize=7,
        label="Calculated arm"
    )

    Axis.scatter(
        Target_X,
        Target_Y,
        Target_Z,
        color="red",
        marker="x",
        s=100,
        label="Target"
    )

    for Index in range(len(Point_Names)):

        Axis.text(
            Points_X[Index],
            Points_Y[Index],
            Points_Z[Index],
            "  " + Point_Names[Index]
        )

    # Use equal scales so the link lengths look correct.

    Plot_X = Points_X + [Target_X]
    Plot_Y = Points_Y + [Target_Y]
    Plot_Z = Points_Z + [Target_Z]

    Plot_Span = max(
        max(Plot_X) - min(Plot_X),
        max(Plot_Y) - min(Plot_Y),
        max(Plot_Z) - min(Plot_Z),
        1
    )

    Plot_Radius = Plot_Span * 0.6

    Centre_X = (max(Plot_X) + min(Plot_X)) / 2
    Centre_Y = (max(Plot_Y) + min(Plot_Y)) / 2
    Centre_Z = (max(Plot_Z) + min(Plot_Z)) / 2

    Axis.set_xlim(Centre_X - Plot_Radius, Centre_X + Plot_Radius)
    Axis.set_ylim(Centre_Y - Plot_Radius, Centre_Y + Plot_Radius)
    Axis.set_zlim(Centre_Z - Plot_Radius, Centre_Z + Plot_Radius)

    Axis.set_box_aspect((1, 1, 1))

    Axis.set_xlabel("X (mm)")
    Axis.set_ylabel("Y (mm)")
    Axis.set_zlabel("Z (mm)")

    Axis.set_title("Robot Arm — Calculated Joint Positions")
    Axis.legend()

    plt.show()

except ValueError as Error:

    print(Error)
    print()