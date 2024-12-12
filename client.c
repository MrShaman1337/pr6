#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Данные подключения: %s <server_address> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_address = argv[1];
    int port = atoi(argv[2]);
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER];

    // Создаём сокет
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Неудачное создание сокета");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Преобразуем адрес сервера
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("Неверный IP-адрес");
        close(client_fd);
        return EXIT_FAILURE;
    }

    // Подключаемся к серверу
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Ошибка подключения");
        close(client_fd);
        return EXIT_FAILURE;
    }

    printf("Подключение к серверу %s:%d\n", server_address, port);

    while (1) {
        printf("Предположите число: ");
        fgets(buffer, MAX_BUFFER, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Удаляем символ новой строки

        // Отправляем попытку
        send(client_fd, buffer, strlen(buffer), 0);

        // Получаем ответ от сервера
        memset(buffer, 0, MAX_BUFFER);
        ssize_t received = recv(client_fd, buffer, MAX_BUFFER, 0);
        if (received <= 0) {
            printf("Сервер оборвал соединение.\n");
            break;
        }

        printf("Сервер: %s", buffer);

        if (strstr(buffer, "Грамотно!")) {
            break;
        }
    }

    close(client_fd);
    return EXIT_SUCCESS;
}
