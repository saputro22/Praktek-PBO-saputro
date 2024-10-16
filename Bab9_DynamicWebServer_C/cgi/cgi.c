#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER 1024

const char* get_parameter(int argc, char *argv[], const char *arg_name) {
    size_t panjang_arg_name = strlen(arg_name);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], arg_name, panjang_arg_name) == 0) {
            return argv[i] + panjang_arg_name;
        }
    }
    return NULL;
}

// Fungsi untuk menjalankan skrip PHP
// dengan memasukkan variabel GET atau POST
void run_php_script(const char *target, const char *query_string, const char *post_data) {
    char command[MAX_BUFFER];
    FILE *fp;

    // Escape karakter khusus untuk string PHP
    char escaped_query_string[MAX_BUFFER];
    char escaped_post_data[MAX_BUFFER];

    // Mengganti karakter ' dan " agar tidak konflik dengan perintah
    snprintf(escaped_query_string, sizeof(escaped_query_string), "%s", query_string);
    snprintf(escaped_post_data, sizeof(escaped_post_data), "%s", post_data);

    // Mengganti " dengan \" di dalam string
    for (char *p = escaped_query_string; *p; p++) {
        if (*p == '"') {
            *p = '\'';
        }
    }
    for (char *p = escaped_post_data; *p; p++) {
        if (*p == '"') {
            *p = '\'';
        }
    }

    // Menjalankan skrip PHP dengan kedua GET dan POST
    snprintf(command, sizeof(command),
             "php -r \"parse_str('%s', $_GET);"
             "parse_str('%s', $_POST);"
             "include '%s';\"",
             escaped_query_string, escaped_post_data, target);

    printf("<p>Eksekusi perintah: %s</p>\n", command);
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    
    char result[MAX_BUFFER];
    while (fgets(result, sizeof(result), fp) != NULL) {
        printf("%s", result);
    }
    pclose(fp);
}



int main(int argc, char *argv[]) {
    char query_string[MAX_BUFFER] = {0};
    char post_data[MAX_BUFFER] = {0};
    const char *target = NULL;
    const char *method = NULL;
    const char *query_string_data = NULL;
    const char *post_data_data = NULL;

    target = get_parameter(argc, argv, "--target=");
    method = get_parameter(argc, argv, "--method=");
    query_string_data = get_parameter(argc, argv, "--data_query_string=");
    post_data_data = get_parameter(argc, argv, "--data_post=");

    printf("<html><body>\n");

    if (target != NULL && method != NULL) {
        if (query_string_data != NULL) {
            snprintf(query_string, sizeof(query_string), "%s", query_string_data);
        }
        if (post_data_data != NULL) {
            snprintf(post_data, sizeof(post_data), "%s", post_data_data);
        }

        if (strcmp(method, "POST") == 0) {
            printf("<p>POST data: %s</p>\n", post_data);
            run_php_script(target, "", post_data);
        } else if (strcmp(method, "GET") == 0) {
            printf("<p>GET query string: %s</p>\n", query_string);
            run_php_script(target, query_string, "");
        } else if (strcmp(method, "BOTH") == 0) {
            printf("<p>GET query string: %s</p>\n", query_string);
            printf("<p>POST data: %s</p>\n", post_data);
            run_php_script(target, query_string, post_data);
        } else {
            printf("<p>Tidak mendukung metode request: %s</p>\n", method);
        }
    } else {
        printf("<p>Target atau method tidak spesifik</p>\n");
    }

    printf("</body></html>\n");
    return 0;
}
// cara memanggil test_get.php
// Gcc cgi.c -o cgi
// ./cgi --target=../test_get.php --method=GET --data_query_string="nama=saputro&alamat=malang"
// cara memanggil test.post.php
// Gcc cgi.c -o cgi
//./cgi --target=../dokumen/test_post.php --method=POST --data_post="username=nasution@gmail.com&passwd=234"

//.\cgi.exe --target=../dokumen/test_get.php --method=GET --data_query_string="nama=nova&alamat=mlg"