#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFSIZE 1024

// Handler untuk membersihkan child process yang sudah selesai
// (mencegah zombie process)
static void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

// Fungsi yang dijalankan oleh tiap proses anak
static void handle_client(int client_fd, struct sockaddr_in *addr) {
    char buffer[BUFSIZE];
    char client_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &addr->sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("[CHILD %d] Client dari %s:%d\n",
           getpid(), client_ip, ntohs(addr->sin_port));

    ssize_t n;
    while ((n = read(client_fd, buffer, BUFSIZE)) > 0) {
        printf("[CHILD %d] Menerima %zd byte\n", getpid(), n);
        if (write(client_fd, buffer, n) < 0) {
            perror("write");
            break;
        }
    }

    printf("[CHILD %d] Client terputus.\n", getpid());
    close(client_fd);
    exit(EXIT_SUCCESS);   // penting: child harus exit, bukan return
}

int main(void) {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Pasang handler SIGCHLD untuk reaping child
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // 1. Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket"); exit(EXIT_FAILURE); }

    // 2. SO_REUSEADDR
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt"); exit(EXIT_FAILURE);
    }

    // 3. Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }

    // 4. Listen
    if (listen(server_fd, 10) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    printf("[SERVER PID %d] Listening di port %d (mode multi-client)\n",
           getpid(), PORT);

    // 5. Loop accept
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        // 6. Fork untuk menangani client ini
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            // ===== Proses anak =====
            close(server_fd);   // anak tidak butuh listen socket
            handle_client(client_fd, &client_addr);
            // handle_client() akan exit(), jadi tidak sampai ke sini
        } else {
            // ===== Proses induk =====
            close(client_fd);   // induk tidak butuh socket client
            printf("[SERVER] Fork child PID %d\n", pid);
        }
    }

    close(server_fd);
    return 0;
}
