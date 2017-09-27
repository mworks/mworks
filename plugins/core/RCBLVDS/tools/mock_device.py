from __future__ import print_function, unicode_literals
try:
    import http.server as http_server
except ImportError:
    # Python 2.7
    import BaseHTTPServer as http_server


root_path = '/'
intan_status_path = '/intan_status.html'

OK = 200
NO_CONTENT = 204
NOT_FOUND = 404
METHOD_NOT_ALLOWED = 405


class RCBLVDSRequestHandler(http_server.BaseHTTPRequestHandler):

    def do_GET(self):
        if self.path == root_path:
            self.send_response(METHOD_NOT_ALLOWED)
            self.send_header('Allow', 'POST')
            self.end_headers()
        elif self.path != intan_status_path:
            self.send_error(NOT_FOUND)
        else:
            self.send_response(OK)
            self.send_header('Content-Type', 'text/plain;charset=utf-8')
            self.end_headers()
            self.wfile.write(('This is a simulated RCS-LVDS '
                              'device\n').encode('utf-8'))

    def do_POST(self):
        if self.path == intan_status_path:
            self.send_response(METHOD_NOT_ALLOWED)
            self.send_header('Allow', 'GET')
            self.end_headers()
        elif self.path != root_path:
            self.send_error(NOT_FOUND)
        else:
            content_length = int(self.headers['Content-Length'])
            body = self.rfile.read(content_length)
            self.send_response(NO_CONTENT)
            self.log_message('Request body: %s' % body.decode('utf-8'))
            self.end_headers()


server = http_server.HTTPServer(('localhost', 8080), RCBLVDSRequestHandler)
try:
    server.serve_forever()
except KeyboardInterrupt:
    print()  # Add final newline
