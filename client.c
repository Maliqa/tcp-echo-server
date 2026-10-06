#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFSIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip> <pesan>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    const char *message = argv[2];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("[CLIENT] Terhubung ke %s:%d\n", server_ip, PORT);

    if (write(sock, message, strlen(message)) < 0) {
        perror("write");
        return 1;
    }

    char buffer[BUFSIZE] = {0};
    ssize_t n = read(sock, buffer, BUFSIZE - 1);
    if (n < 0) {
        perror("read");
        return 1;
    }
    buffer[n] = '\0';

    printf("[CLIENT] Balasan server: %s\n", buffer);

    close(sock);
    return 0;
}
