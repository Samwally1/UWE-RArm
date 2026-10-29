from http.server import HTTPServer

from api.handlers import Handler


if __name__ == "__main__":
    server = HTTPServer(("0.0.0.0", 8000), Handler)
    server.serve_forever()
