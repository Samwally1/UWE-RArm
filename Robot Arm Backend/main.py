import json
from http.server import BaseHTTPRequestHandler, HTTPServer


controller_state = {}


class Handler(BaseHTTPRequestHandler):

    def do_GET(self):

        if self.path == "/mode":
            self.send_response(200)
            self.send_header("Mode", "Training")
            self.end_headers()

            self.wfile.write(b"true")

        elif self.path == "/controller":
            response = json.dumps(controller_state).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(response)))
            self.end_headers()
            self.wfile.write(response)

        else:
            self.send_error(404)

    def do_POST(self):
        if self.path != "/controller":
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

        controller_state.update(payload)
        self.send_response(204)
        self.end_headers()


server = HTTPServer(("0.0.0.0", 8000), Handler)

server.serve_forever()
