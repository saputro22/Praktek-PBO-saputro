import os
import socket
import mimetypes

PORT = 8080
BUFFER_SIZE = 1024
FOLDER_DOCUMENT = "D:\Bab9_DynamicWebServer_C\dokumen"

def parse_request_line(request):
    lines = request.split("\r\n")
    request_line = lines[0]
    words = request_line.split(" ")
    method = words[0]
    uri = words[1]
    http_version = words[2]
    if uri.startswith('/'):
        uri = uri[1:]
    if not uri:
        uri = "index.html"
    return method, uri, http_version

def handle_method(method, uri, http_version):
    file_url = os.path.join(FOLDER_DOCUMENT, uri)
    if not os.path.exists(file_url):
        not_found_response = (
            f"{http_version} 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            f"Content-Length: {len('<h1>404 Not Found</h1>')}\r\n"
            "\r\n"
            "<h1>404 Not Found</h1>"
        )
        return not_found_response.encode(), len(not_found_response)
    else:
        mime_type, _ = mimetypes.guess_type(uri)
        if mime_type is None:
            mime_type = "text/html"
        with open(file_url, "rb") as file:
            file_content = file.read()
        response_line = (
            f"{http_version} 200 OK\r\n"
            f"Content-Type: {mime_type}\r\n"
            f"Content-Length: {len(file_content)}\r\n\r\n"
        )
        response = response_line.encode() + file_content
        return response, len(response)

def handle_client(sock_client):
    request = sock_client.recv(BUFFER_SIZE).decode('utf-8', errors='ignore')
    print("-----------------------------------------------")
    print("Request dari browser :\r\n\r\n", request)
    method, uri, http_version = parse_request_line(request)
    response, response_size = handle_method(method, uri, http_version)
    
    if response:
        sock_client.sendall(response)
    else:
        print("Response data is NULL")
    
    print("-----------------------------------------------")
    print(f"Method : {method} URI : {uri}")
    print("Response ke browser :\n", response.decode('utf-8', errors='ignore'))
    sock_client.close()

def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(('', PORT))
    server_socket.listen(3)
    print(f"Server siap IP : 127.0.0.1 Port : {PORT}")
    
    try:
        while True:
            client_socket, addr = server_socket.accept()
            handle_client(client_socket)
    except KeyboardInterrupt:
        print("Server dihentikan.")
    finally:
        server_socket.close()

if __name__ == "__main__":
    main()
