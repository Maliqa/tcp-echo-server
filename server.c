#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFSIZE 1024

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFSIZE];

    // 1. Buat socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. Set opsi SO_REUSEADDR agar port bisa langsung dipakai ulang
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 3. Bind ke alamat & port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 4. Listen
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Listening di port %d...\n", PORT);

    // 5. Loop utama: terima koneksi
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("[SERVER] Client terhubung dari %s:%d\n",
               client_ip, ntohs(address.sin_port));

        // 6. Echo loop: baca dari client, kirim balik
        ssize_t n;
        while ((n = read(client_fd, buffer, BUFSIZE)) > 0) {
            printf("[SERVER] Menerima %zd byte\n", n);
            if (write(client_fd, buffer, n) < 0) {
                perror("write");
                break;
            }
        }

        printf("[SERVER] Client terputus.\n");
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
