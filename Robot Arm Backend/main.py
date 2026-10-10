import json
import os
from http.server import BaseHTTPRequestHandler, HTTPServer


controller_state = {}
arm_status = ""
calibration_data = None
arm_mode = os.getenv("ARM_MODE", "Normal")


class Handler(BaseHTTPRequestHandler):

    def do_GET(self):

        if self.path == "/mode":
            self.send_response(200)
            self.send_header("Mode", arm_mode)
            self.end_headers()

            self.wfile.write(b"true")

        elif self.path == "/controller":
            response = json.dumps(controller_state).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(response)))
            self.end_headers()
            self.wfile.write(response)

        elif self.path == "/status":
            response = arm_status.encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(response)))
            self.end_headers()
            self.wfile.write(response)

        elif self.path == "/calibration":
            if calibration_data is None:
                self.send_error(404, "Calibration not available")
                return

            response = json.dumps(calibration_data, separators=(",", ":")).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(response)))
            self.end_headers()
            self.wfile.write(response)

        else:
            self.send_error(404)

    def do_POST(self):
        if self.path == "/controller":
            target = controller_state
        elif self.path == "/status":
            content_length = int(self.headers.get("Content-Length", 0))
            if content_length <= 0 or content_length > 4096:
                self.send_error(400, "Invalid arm status size")
                return

            global arm_status
            arm_status = self.rfile.read(content_length).decode("utf-8")
            self.send_response(204)
            self.end_headers()
            return
        elif self.path == "/calibration":
            content_length = int(self.headers.get("Content-Length", 0))
            if content_length <= 0 or content_length > 4096:
                self.send_error(400, "Invalid calibration size")
                return

            try:
                payload = json.loads(self.rfile.read(content_length))
                joint_limits = payload["jointLimits"]
                joint_homes = payload["jointHomes"]
            except (json.JSONDecodeError, UnicodeDecodeError, KeyError, TypeError):
                self.send_error(400, "Calibration must contain jointLimits and jointHomes")
                return

            if (
                not isinstance(joint_limits, list)
                or len(joint_limits) != 4
                or any(
                    not isinstance(limits, list)
                    or len(limits) != 2
                    or not all(isinstance(value, int) and 0 <= value <= 180 for value in limits)
                    for limits in joint_limits
                )
                or not isinstance(joint_homes, list)
                or len(joint_homes) != 4
                or not all(isinstance(value, int) and 0 <= value <= 180 for value in joint_homes)
            ):
                self.send_error(400, "Calibration values must be four 0-180 limits and home positions")
                return

            global calibration_data
            calibration_data = {
                "jointLimits": joint_limits,
                "jointHomes": joint_homes,
            }
            self.send_response(204)
            self.end_headers()
            return
        else:
            self.send_error(404)
            return

        content_length = int(self.headers.get("Content-Length", 0))
        if content_length <= 0 or content_length > 4096:
            self.send_error(400, "Invalid controller state size")
            return

        try:
            payload = json.loads(self.rfile.read(content_length))
        except (json.JSONDecodeError, UnicodeDecodeError):
            self.send_error(400, "Controller state must be JSON")
            return

        if not isinstance(payload, dict):
            self.send_error(400, "Controller state must be an object")
            return

        target.update(payload)
        self.send_response(204)
        self.end_headers()


server = HTTPServer(("0.0.0.0", 8000), Handler)

server.serve_forever()
