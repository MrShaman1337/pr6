#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_BUFFER 1024
#define SECRET_MIN 1
#define SECRET_MAX 100

void handle_client(int client_fd, const char *client_ip) {
    char buffer[MAX_BUFFER];
    int secret_number = rand() % (SECRET_MAX - SECRET_MIN + 1) + SECRET_MIN;

    printf("Клиент подключился: %s, загаданное число: %d\n", client_ip, secret_number);

    while (1) {
        memset(buffer, 0, MAX_BUFFER);

        if (recv(client_fd, buffer, MAX_BUFFER, 0) <= 0) {
            printf("Клиент отключился: %s\n", client_ip);
            break;
        }

        printf("%s: %s", client_ip, buffer);
        int guess = atoi(buffer);

        if (guess == secret_number) {
            snprintf(buffer, MAX_BUFFER, "Правильно, загаданный номер - %d.\n", secret_number);
            send(client_fd, buffer, strlen(buffer), 0);
            break;
        }

        const char *response = guess < secret_number ? "Горячо\n" : "Холодно\n";
        send(client_fd, response, strlen(response), 0);
    }

    close(client_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Порт: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Неудачное создание сокета");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Неудачная привязка порта");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) == -1) {
        perror("Неудачное подключение к порту");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("Сервер работает на порте %d...\n", port);

    srand(time(NULL));

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("Неудачное подключение");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        handle_client(client_fd, client_ip);
    }

    close(server_fd);
    return EXIT_SUCCESS;
}