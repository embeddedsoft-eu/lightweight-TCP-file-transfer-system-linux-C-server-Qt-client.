/**
 * TCP File Server
 * 
 * A multi-process server that accepts file uploads via TCP.
 * Each connection is handled by a separate child process.
 * 
 * Usage: ./file_server [-p PORT]
 * 
 * Default port: 4445
 */

// Убираем _GNU_SOURCE отсюда - он уже определен в командной строке
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#include "file_handler.h"
#include "signal_handler.h"

#define DEFAULT_PORT 4445
#define BACKLOG 5
#define BUFFER_SIZE 65536
#define TIMEOUT_SECONDS 30

// Объявляем функцию перед использованием
static void handle_tcp_session(int client_socket, const char *client_ip, int client_port);

/**
 * Handle a single TCP session
 */static void handle_tcp_session(int client_socket, const char *client_ip, int client_port) {
    unsigned char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received;
    ssize_t total_received = 0;
    int packet_count = 0;

    printf("\n--- New connection: %s:%d ---\n", client_ip, client_port);
    printf("Waiting for data...\n");
    fflush(stdout);

    // Устанавливаем таймаут между пакетами
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Общий таймаут сессии
    alarm(TIMEOUT_SECONDS);

    while (total_received < (ssize_t)(sizeof(buffer) - 1)) {
        bytes_received = recv(client_socket,
                              buffer + total_received,
                              sizeof(buffer) - total_received - 1,
                              0);

        if (bytes_received < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Inter-packet timeout (%d seconds) - ending transmission\n", 3);
                break;  // Таймаут между пакетами - конец передачи
            }
            printf("Error: %s\n", strerror(errno));
            break;
        }

        if (bytes_received == 0) {
            printf("Connection closed by client after %d packets\n", packet_count);
            break;  // Клиент закрыл соединение - конец передачи
        }

        packet_count++;
        total_received += bytes_received;
        printf("Packet %d: +%ld bytes = %ld total\n",
               packet_count, (long)bytes_received, (long)total_received);

        // Сбрасываем общий таймаут при получении данных
        alarm(TIMEOUT_SECONDS);
    }

    alarm(0);

    if (total_received > 0) {
        printf("\n=== Transfer complete: %ld bytes in %d packets ===\n",
               (long)total_received, packet_count);

        // Покажем первые символы
        printf("Data starts with: ");
        for (int i = 0; i < 32 && i < total_received; i++) {
            if (buffer[i] >= 32 && buffer[i] <= 126)
                printf("%c", buffer[i]);
            else
                printf("\\x%02x", buffer[i]);
        }
        printf("\n");
        fflush(stdout);

        save_to_file(buffer, (int)total_received);
    }

    printf("--- Connection closed: %s:%d ---\n\n", client_ip, client_port);
    fflush(stdout);
    close(client_socket);
}
/**
 * Parse command line arguments
 */
static int parse_arguments(int argc, char *argv[], int *port) {
    *port = DEFAULT_PORT;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("TCP File Server\n");
            printf("Usage: %s [-p PORT]\n", argv[0]);
            printf("  -p PORT  Specify port number (default: %d)\n", DEFAULT_PORT);
            return -1;
        }
        
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            *port = atoi(argv[i + 1]);
            if (*port <= 0 || *port > 65535) {
                fprintf(stderr, "Invalid port number: %s\n", argv[i + 1]);
                return -1;
            }
            i++;
        }
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    int listener_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int port;
    pid_t child_pid;
    
    // Отключаем буферизацию вывода
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    if (parse_arguments(argc, argv, &port) < 0) {
        return 1;
    }
    
    printf("========================================\n");
    printf("TCP File Server v1.0\n");
    printf("Listening on port: %d\n", port);
    printf("Storage: /var/www/embeddedsoft/test/\n");
    printf("========================================\n");
    
    init_signal_handlers();
    
    listener_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_socket < 0) {
        perror("socket creation failed");
        return 1;
    }
    
    int reuse = 1;
    setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(listener_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(listener_socket);
        return 1;
    }
    
    if (listen(listener_socket, BACKLOG) < 0) {
        perror("listen failed");
        close(listener_socket);
        return 1;
    }
    
    printf("[SERVER STARTED] Waiting for connections on port %d...\n\n", port);
    
    while (1) {
        client_socket = accept(listener_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (client_socket < 0) {
            if (errno == EINTR) continue;
            perror("accept failed");
            continue;
        }
        
        // Сохраняем IP и порт ДО fork()
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);
        
        child_pid = fork();
        
        if (child_pid < 0) {
            perror("fork failed");
            close(client_socket);
            continue;
        }
        
        if (child_pid > 0) {
            // Parent
            close(client_socket);
            printf("Created child %d for %s:%d\n", child_pid, client_ip, client_port);
        } else {
            // Child
            close(listener_socket);
            set_current_socket(client_socket, listener_socket);
            
            // Передаем IP как строку и порт как число
            handle_tcp_session(client_socket, client_ip, client_port);
            
            shutdown(client_socket, SHUT_RDWR);
            close(client_socket);
            _exit(0);
        }
    }
    
    close(listener_socket);
    return 0;
}
