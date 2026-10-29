"""Interactive desktop visualiser for the UWE robot-arm kinematics."""

import math
import json
import re
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from urllib.request import Request, urlopen
import tkinter as tk
from tkinter import messagebox, ttk

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

from kinematics_model import inverse_kinematics, radial_reach


class KinematicsApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("UWE Robot Arm Kinematics")
        self.minsize(900, 650)

        self.values = {
            "X (mm)": tk.DoubleVar(value=11.71),
            "Y (mm)": tk.DoubleVar(value=133.85),
            "Z (mm)": tk.DoubleVar(value=114.88),
            "Tool angle (°)": tk.DoubleVar(value=math.degrees(2)),
        }
        self.elbow_branch = tk.IntVar(value=1)
        self.status = tk.StringVar()
        self.radius = tk.StringVar()
        self.calibration_info = tk.StringVar(value="Calibration not loaded")
        self.calibration_worker = ThreadPoolExecutor(max_workers=1)
        self.joint_limits = {
            f"S{joint}": (tk.StringVar(value="-360"), tk.StringVar(value="360"))
            for joint in range(1, 5)
        }
        self.joint_readout = tk.StringVar()

        controls = ttk.Frame(self, padding=12)
        controls.pack(side=tk.LEFT, fill=tk.Y)
        plot_frame = ttk.Frame(self, padding=(0, 12, 12, 12))
        plot_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        ttk.Label(controls, text="Target position", font=("TkDefaultFont", 12, "bold")).pack(anchor=tk.W)
        slider_definitions = [
            ("X (mm)", self.values["X (mm)"], -400, 400),
            ("Y (mm)", self.values["Y (mm)"], -400, 400),
            ("Z (mm)", self.values["Z (mm)"], -300, 500),
            ("Tool angle (°)", self.values["Tool angle (°)"], -180, 180),
        ]
        for label, variable, low, high in slider_definitions:
            row = ttk.Frame(controls)
            row.pack(fill=tk.X, pady=(10, 0))
            ttk.Label(row, text=label).pack(anchor=tk.W)
            scale = ttk.Scale(row, from_=low, to=high, variable=variable, command=self.update_plot)
            scale.pack(fill=tk.X)
            ttk.Label(row, textvariable=variable).pack(anchor=tk.E)

        ttk.Label(controls, textvariable=self.radius).pack(anchor=tk.W, pady=(16, 0))
        ttk.Radiobutton(controls, text="Elbow up", variable=self.elbow_branch, value=1, command=self.update_plot).pack(anchor=tk.W)
        ttk.Radiobutton(controls, text="Elbow down", variable=self.elbow_branch, value=-1, command=self.update_plot).pack(anchor=tk.W)

        ttk.Separator(controls).pack(fill=tk.X, pady=14)
        ttk.Label(controls, text="Joint-angle limits (degrees)", font=("TkDefaultFont", 10, "bold")).pack(anchor=tk.W)
        for joint, (minimum, maximum) in self.joint_limits.items():
            row = ttk.Frame(controls)
            row.pack(fill=tk.X, pady=(5, 0))
            ttk.Label(row, text=joint, width=5).pack(side=tk.LEFT)
            ttk.Entry(row, textvariable=minimum, width=7).pack(side=tk.LEFT)
            ttk.Label(row, text="to").pack(side=tk.LEFT, padx=3)
            ttk.Entry(row, textvariable=maximum, width=7).pack(side=tk.LEFT)
        ttk.Button(controls, text="Apply joint limits", command=self.apply_joint_limits).pack(anchor=tk.W, pady=(8, 0))
        ttk.Label(controls, textvariable=self.joint_readout, justify=tk.LEFT).pack(anchor=tk.W, pady=(10, 0))
        ttk.Label(controls, textvariable=self.status, wraplength=245).pack(anchor=tk.W, pady=(16, 0))

        calibration_frame = ttk.LabelFrame(plot_frame, text="Arm calibration", padding=8)
        calibration_frame.pack(anchor=tk.E, fill=tk.X, pady=(0, 8))
        ttk.Label(calibration_frame, textvariable=self.calibration_info,
                  justify=tk.RIGHT, anchor=tk.E).pack(anchor=tk.E)
        self.reload_button = ttk.Button(calibration_frame, text="Reload calibration",
                                        command=self.load_calibration)
        self.reload_button.pack(anchor=tk.E, pady=(5, 0))

        self.figure = Figure(figsize=(7, 6), dpi=100)
        self.axis = self.figure.add_subplot(111, projection="3d")
        self.canvas = FigureCanvasTkAgg(self.figure, master=plot_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        self.update_plot()
        self.after(0, self.load_calibration)

    @staticmethod
    def fetch_calibration():
        """Use the same HOST and /calibration endpoint as the firmware."""
        secrets = Path(__file__).resolve().parents[1] / "Robot Arm Firmware/src/Secrets.h"
        configuration = secrets.read_text(encoding="utf-8")
        host = re.search(r'^\s*(?:#define\s+HOST\s+|(?:const\s+)?(?:char\s*\*|String)\s+HOST\s*=\s*)"([^"\r\n]+)"',
                         configuration, re.MULTILINE)
        if not host:
            raise ValueError("HOST was not found in firmware Secrets.h")
        url = "https://" + host.group(1) + "/calibration"
        request = Request(url, headers={
            "User-Agent": "KinematicsApp/1.0",
            "Accept": "application/json",
        })
        with urlopen(request, timeout=8) as response:
            payload = json.loads(response.read(4097))
        limits = payload["jointLimits"]
        homes = payload.get("jointHomes", payload.get("joint0s"))
        def valid(value):
            return type(value) is int and 0 <= value <= 180
        if (not isinstance(limits, list) or len(limits) != 4
                or any(not isinstance(pair, list) or len(pair) != 2
                       or not all(valid(v) for v in pair) for pair in limits)
                or not isinstance(homes, list) or len(homes) != 4
                or not all(valid(v) for v in homes)):
            raise ValueError("Expected four joint ranges and homes in 0-180 degrees")
        return url, limits, homes

    def load_calibration(self):
        self.reload_button.configure(state=tk.DISABLED)
        self.calibration_info.set("Loading firmware server calibration...")
        future = self.calibration_worker.submit(self.fetch_calibration)
        self.after(100, self.finish_calibration, future)

    def finish_calibration(self, future):
        if not future.done():
            self.after(100, self.finish_calibration, future)
            return
        self.reload_button.configure(state=tk.NORMAL)
        try:
            url, limits, homes = future.result()
        except Exception as error:
            self.calibration_info.set(
                f"Calibration unavailable: {error}\nUsing current joint-angle limits."
            )
            return
        lines = [url]
        for index, (pair, home) in enumerate(zip(limits, homes), start=1):
            low, high = sorted(pair)
            minimum, maximum = self.joint_limits[f"S{index}"]
            minimum.set(str(low))
            maximum.set(str(high))
            inversion = " (inverted)" if pair[0] > pair[1] else ""
            lines.append(f"S{index}: {low}-{high} degrees | home {home} degrees{inversion}")
        self.calibration_info.set("\n".join(lines))
        self.update_plot()

    def apply_joint_limits(self):
        """Validate the entered physical joint ranges before using them."""
        try:
            limits = {
                joint: (float(minimum.get()), float(maximum.get()))
                for joint, (minimum, maximum) in self.joint_limits.items()
            }
        except ValueError:
            messagebox.showerror("Invalid joint limits", "Enter numeric angle limits in degrees.")
            return

        invalid = [joint for joint, (low, high) in limits.items() if not math.isfinite(low) or not math.isfinite(high) or low > high]
        if invalid:
            messagebox.showerror(
                "Invalid joint limits",
                "Each minimum must be finite and no greater than its maximum.\n"
                f"Check: {', '.join(invalid)}",
            )
            return
        self.update_plot()

    def update_plot(self, *_):
        x = self.values["X (mm)"].get()
        y = self.values["Y (mm)"].get()
        z = self.values["Z (mm)"].get()
        angle = math.radians(self.values["Tool angle (°)"].get())
        self.radius.set(f"Radial reach r: {radial_reach(x, y):.1f} mm")
        self.axis.clear()

        try:
            pose = inverse_kinematics(x, y, z, angle, self.elbow_branch.get())
            points = [pose.base, pose.shoulder, pose.elbow, pose.wrist, pose.tool]
            px, py, pz = zip(*points)
            self.axis.plot(px, py, pz, "o-", linewidth=4, markersize=7, label="Arm")
            self.axis.scatter(x, y, z, color="tab:green", marker="x", s=100, label="Geometric target")
            self.status.set("VALID — target is within the geometric reach of the arm.")
            joint_angles = [math.degrees(value) for value in pose.angles]
            self.joint_readout.set("\n".join(
                f"S{index}: {value:.1f} degrees"
                for index, value in enumerate(joint_angles, start=1)
            ))
            outside_limits = []
            for index, value in enumerate(joint_angles, start=1):
                joint = f"S{index}"
                low, high = (float(part.get()) for part in self.joint_limits[joint])
                if not low <= value <= high:
                    outside_limits.append(f"{joint} ({value:.1f} degrees)")

            if outside_limits:
                self.axis.scatter(x, y, z, color="tab:orange", marker="x", s=100, label="Outside joint limits")
                self.status.set("INVALID: joint limit exceeded: " + ", ".join(outside_limits))
            else:
                self.status.set("VALID: target is within geometric and joint-angle limits.")
        except ValueError as error:
            self.axis.scatter(x, y, z, color="tab:red", marker="x", s=100, label="Invalid target")
            self.axis.scatter(0, 0, 0, color="black", s=45, label="Base")
            self.joint_readout.set("S1-S4: unavailable")
            self.status.set(f"INVALID — {error}")

        limit = 430
        self.axis.set(xlim=(-limit, limit), ylim=(-limit, limit), zlim=(-300, 550))
        self.axis.set_box_aspect((1, 1, 1))
        self.axis.set_xlabel("X (mm)")
        self.axis.set_ylabel("Y (mm)")
        self.axis.set_zlabel("Z (mm)")
        self.axis.set_title("Robot arm inverse-kinematics visualiser")
        self.axis.legend(loc="upper left")
        self.canvas.draw_idle()


if __name__ == "__main__":
    KinematicsApp().mainloop()
