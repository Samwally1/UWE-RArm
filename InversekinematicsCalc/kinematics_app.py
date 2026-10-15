"""Interactive desktop visualiser for the UWE robot-arm kinematics."""

import math
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
        self.scales = {}
        self.limits = {}
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
            self.scales[label] = scale
            self.limits[label] = (tk.StringVar(value=str(low)), tk.StringVar(value=str(high)))
            ttk.Label(row, textvariable=variable).pack(anchor=tk.E)

        ttk.Separator(controls).pack(fill=tk.X, pady=14)
        ttk.Label(controls, text="Slider limits", font=("TkDefaultFont", 10, "bold")).pack(anchor=tk.W)
        for label, _, _, _ in slider_definitions:
            row = ttk.Frame(controls)
            row.pack(fill=tk.X, pady=(5, 0))
            ttk.Label(row, text=label, width=16).pack(side=tk.LEFT)
            minimum, maximum = self.limits[label]
            ttk.Entry(row, textvariable=minimum, width=7).pack(side=tk.LEFT)
            ttk.Label(row, text="to").pack(side=tk.LEFT, padx=3)
            ttk.Entry(row, textvariable=maximum, width=7).pack(side=tk.LEFT)
        ttk.Button(controls, text="Apply limits", command=self.apply_limits).pack(anchor=tk.W, pady=(8, 0))

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

        self.figure = Figure(figsize=(7, 6), dpi=100)
        self.axis = self.figure.add_subplot(111, projection="3d")
        self.canvas = FigureCanvasTkAgg(self.figure, master=plot_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        self.update_plot()

    def apply_limits(self):
        """Apply the user-entered slider bounds and clamp current values."""
        try:
            new_limits = {
                label: (float(minimum.get()), float(maximum.get()))
                for label, (minimum, maximum) in self.limits.items()
            }
        except ValueError:
            messagebox.showerror("Invalid limits", "Enter numeric minimum and maximum values.")
            return

        invalid = [label for label, (low, high) in new_limits.items() if low >= high]
        if invalid:
            messagebox.showerror(
                "Invalid limits",
                "Each minimum must be smaller than its maximum.\n"
                f"Check: {', '.join(invalid)}",
            )
            return

        for label, (low, high) in new_limits.items():
            self.scales[label].configure(from_=low, to=high)
            self.values[label].set(min(high, max(low, self.values[label].get())))
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

        invalid = [joint for joint, (low, high) in limits.items() if low >= high]
        if invalid:
            messagebox.showerror(
                "Invalid joint limits",
                "Each minimum must be smaller than its maximum.\n"
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
