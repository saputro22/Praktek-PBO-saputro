#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define FOLDER_DOCUMENT "dokumen/" // Path ke file resource
#define CGI_PATH ".\\cgi\\cgi.exe" // Path ke program CGI

// untuk menjalankannya
// gcc .\webserver.c -o .\webserver -lws2_32
// .\webserver.exe


void parse_request_line(
    char *request, 
    char *method, 
    char *uri, 
    char *http_version, 
    char *query_string, 
    char *post_data) {
    char request_message[BUFFER_SIZE];
    char request_line[BUFFER_SIZE];
    char *words[3];
    // Baca baris pertama dari rangkaian data request
    strcpy(request_message, request);
    char *line = strtok(request_message, "\r\n");
    if (line == NULL) {
        fprintf(stderr, "Error: Invalid request line\n");
        return;
    }
    strcpy(request_line, line);
    
    // Pilah request line berdasarkan spasi
    int i = 0;
    char *token = strtok(request_line, " ");
    while (token != NULL && i < 3) {
        words[i++] = token;
        token = strtok(NULL, " ");
    }
    
    // Pastikan ada 3 komponen dalam request line
    if (i < 3) {
        fprintf(stderr, "Error: Request line tidak lengkap\n");
        return;
    }
    // kata 1 : Method, kata 2 : uri, kata 3 : versi HTTP
    strcpy(method, words[0]);
    strcpy(uri, words[1]);
    strcpy(http_version, words[2]);
   
    // Hapus tanda / pada URI
    if (uri[0] == '/') {
        memmove(uri, uri + 1, strlen(uri));
    }
    // Cek apakah ada query string
    char *query_start = strchr(uri, '?');
    if (query_start != NULL) {
        // Pisahkan query string dari URI *query_start = '\0';
        // Salin query string
        strcpy(query_string, query_start + 1);
    } else {
    // Tidak ada query string
    query_string[0] = '\0';
    }
    //Cek apakah ada body data
    char *body_start = strstr(request, "\r\n\r\n");
    if (body_start != NULL) {
        // Pindahkan pointer ke awal body data
        body_start += 4; // Melewati CRLF CRLF
        // Salin data POST dari body
        strcpy(post_data, body_start);
    } else {
        post_data[0] = '\0'; // Tidak ada body data
    }
    // Jika URI kosong, maka isi URI dengan resource default
    // yaitu index.html
    if (strlen(uri) == 0) {
        strcpy(uri, "index.html");
    }
}
void get_mime_type(char *file, char *mime) {
    const char *dot = strrchr(file, '.');

    // Jika dot NULL maka isi variabel mime dengan "text/html"
    // Jika dot tidak NULL maka isi variabel mime dengan MIME type
    if (dot == NULL) {
        strcpy(mime, "text/html");
    } else if (strcmp(dot, ".html") == 0) {
        strcpy(mime, "text/html");
    } else if (strcmp(dot, ".css") == 0) {
        strcpy(mime, "text/css");
    } else if (strcmp(dot, ".js") == 0) {
        strcpy(mime, "application/javascript");
    } else if (strcmp(dot, ".jpg") == 0) {
        strcpy(mime, "image/jpeg");
    } else if (strcmp(dot, ".png") == 0) {
        strcpy(mime, "image/png");
    } else if (strcmp(dot, ".gif") == 0) {
        strcpy(mime, "image/gif");
    } else if (strcmp(dot, ".ico") == 0) {
        strcpy(mime, "image/x-icon");
    } else {
        strcpy(mime, "text/html");
    }
}

void handle_method(
    char **response,
    int *response_size,
    char *method,
    char *uri,
    char *http_version,
    char *query_string,
    char *post_data) {

    char fileURL[256];
    snprintf(fileURL, sizeof(fileURL), "%s/%s", FOLDER_DOCUMENT, uri);
    printf(fileURL);
    FILE *file = fopen(fileURL, "rb");

    // Jika file resource tidak ditemukan,
    // maka kirimkan status 404 ke browser
    if (!file) {
        char not_found[BUFFER_SIZE];
        snprintf(not_found, sizeof(not_found),
            "%s 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %lu\r\n"
            "\r\n"
            "<h1>404 Not Found</h1>",
            http_version,
            strlen("<h1>404 Not Found</h1>")
        );
        *response = (char *)malloc(strlen(not_found) + 1);
        strcpy(*response, not_found);
        *response_size = strlen(not_found);
    } else {
        char *content;
        size_t content_length;

        // Jika ekstensi file adalah .php, jalankan CGI
        const char *extension = strrchr(uri, '.');
        if (extension && strcmp(extension, ".php") == 0) {
            // Menjalankan CGI
            char command[BUFFER_SIZE];
            snprintf(command, sizeof(command),
                "%s --target=%s --method=%s --data_query_string=\"%s\" --data_post=\"%s\"",
                CGI_PATH, fileURL, method, query_string, post_data);
            FILE *fp = popen(command, "r");
            if (fp == NULL) {
                perror("popen");
                exit(EXIT_FAILURE);
            }

            // Baca output dari program CGI
            char cgi_output[BUFFER_SIZE];
            size_t output_len = 0;
            while (fgets(cgi_output + output_len, sizeof(cgi_output) - output_len, fp) != NULL) {
                output_len += strlen(cgi_output + output_len);
            }
            pclose(fp);

            // Buat response header
            char response_header[BUFFER_SIZE];
            snprintf(response_header, sizeof(response_header),
                "%s 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %lu\r\n\r\n",
                http_version, output_len);

            // Alokasikan memory untuk response
            *response_size = strlen(response_header) + output_len;
            *response = (char *)malloc(*response_size + 1);
            strcpy(*response, response_header);
            strcat(*response, cgi_output);
        } else {
            // Jika file resource ditemukan, dan ekstensi file bukan .php
            char response_line[BUFFER_SIZE];
            char mimeType[32];
            get_mime_type(uri, mimeType);

            // Buat response header dengan status code 200 OK
            // dan content-type sesuai dengan MIME file yang dibaca
            snprintf(response_line, sizeof(response_line),
                "%s 200 OK\r\n"
                "Content-Type: %s\r\n\r\n", http_version, mimeType);

            // Baca file resource yang ada di server
            fseek(file, 0, SEEK_END);
            long fsize = ftell(file);
            fseek(file, 0, SEEK_SET);
            *response = (char *)malloc(fsize + strlen(response_line) + 1);
            strcpy(*response, response_line);
            *response_size = fsize + strlen(response_line);
            char *msg_body = *response + strlen(response_line);
            fread(msg_body, fsize, 1, file);
        }
        fclose(file);
    }
}

void handle_client(int sock_client) {
    char request[BUFFER_SIZE];
    char *response = NULL;
    int request_size;
    int response_size = 0;
    char method[16], uri[256], query_string[512], post_data[512], http_version[16];

    request_size = recv(sock_client, request, BUFFER_SIZE - 1, 0);
    if (request_size < 0) {
        perror("Proses baca dari client gagal");
        closesocket(sock_client);
        return;
    }

    // Pastikan string null-terminated
    request[request_size] = '\0';
    printf("\n-----------------------------------------------\n");
    printf("Request dari browser:\r\n\r\n%s", request);

    parse_request_line(request, method, uri, http_version, query_string, post_data);
    handle_method(&response, &response_size, method, uri, http_version, query_string, post_data);

    printf("\n-----------------------------------------------\n");
    printf("Method: %s URI: %s\n", method, uri);
    printf("Response ke browser:\n%s\n", response);

    if (response != NULL) {
        if (send(sock_client, response, response_size, 0) < 0) {
            perror("Proses kirim data ke client gagal");
        }
        free(response);
    } else {
        printf("Response data is NULL\n");
    }

    closesocket(sock_client);
}

int main() {
    WSADATA wsaData;
    int sock_server, sock_client;
    struct sockaddr_in serv_addr;
    int opt = 1;
    int addrlen = sizeof(serv_addr);

    // Inisialisasi Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("Inisialisasi Winsock gagal");
        return EXIT_FAILURE;
    }

    if ((sock_server = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("Inisialisasi socket server gagal");
        WSACleanup();
        return EXIT_FAILURE;
    }

    if (setsockopt(sock_server, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt gagal");
        closesocket(sock_server);
        WSACleanup();
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sock_server, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("proses bind gagal");
        closesocket(sock_server);
        WSACleanup();
        return EXIT_FAILURE;
    }

    if (listen(sock_server, 3) < 0) {
        perror("proses listen gagal");
        closesocket(sock_server);
        WSACleanup();
        return EXIT_FAILURE;
    }

    printf("Server siap IP: %s Port: %d\n", "127.0.0.1", PORT);

    while (1) {
        if ((sock_client = accept(sock_server, (struct sockaddr *)&serv_addr, (socklen_t *)&addrlen)) < 0) {
            perror("proses accept gagal");
            closesocket(sock_server);
            WSACleanup();
            return EXIT_FAILURE;
        }
        handle_client(sock_client);
    }

    closesocket(sock_server);
    WSACleanup();
    return 0;
}
