"""Tiny no-cache static server for the inventory UMAP viewer.

Usage:  python serve.py [port]      # default 8778
Then open http://localhost:8778/ in a browser.
"""
import sys
from http.server import SimpleHTTPRequestHandler
from pathlib import Path
import socketserver

PORT = next((int(a) for a in sys.argv[1:] if a.isdigit()), 8778)
HERE = Path(__file__).resolve().parent


class NoCacheHandler(SimpleHTTPRequestHandler):
    """Serve with no-cache headers so reloads always pick up fresh data.json."""

    def __init__(self, *a, **kw):
        super().__init__(*a, directory=str(HERE), **kw)

    def end_headers(self):
        self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0")
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        super().end_headers()


class Server(socketserver.TCPServer):
    allow_reuse_address = True


if __name__ == "__main__":
    with Server(("0.0.0.0", PORT), NoCacheHandler) as httpd:
        print(f"serving inventory UMAP viewer on http://localhost:{PORT}/")
        httpd.serve_forever()
