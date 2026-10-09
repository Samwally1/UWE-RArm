from http.server import BaseHTTPRequestHandler, HTTPServer


class Handler(BaseHTTPRequestHandler):

    def do_GET(self):

        if self.path == "/mode":
            self.send_response(200)
            self.send_header("Mode", "Training")
            self.end_headers()

            self.wfile.write(b"true")

        else:
            self.send_error(404)


server = HTTPServer(("0.0.0.0", 8000), Handler)

server.serve_forever()