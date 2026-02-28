import http.server
import socketserver

PORT = 8000
Handler = http.server.SimpleHTTPRequestHandler
# Ensure .wasm files are served with the correct MIME type
Handler.extensions_map.update({
    '.wasm': 'application/wasm',
})

print(f"Momentum Web Server running at http://localhost:{PORT}")
with socketserver.TCPServer(("", PORT), Handler) as httpd:
    httpd.serve_forever()