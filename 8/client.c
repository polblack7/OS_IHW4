#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

typedef struct {
    int fragment_number;
    char data[BUFFER_SIZE];
} Fragment;

void encrypt_text(char* text) {
    for (int i = 0; text[i] != '\0'; i++) {
        text[i] = (char)((int)text[i] - 64);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <fragment_number> <text>\n", argv[0]);
        return -1;
    }

    char* server_ip = argv[1];
    int port = atoi(argv[2]);
    int fragment_number = atoi(argv[3]);
    char text[BUFFER_SIZE];
    strncpy(text, argv[4], BUFFER_SIZE);

    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    Fragment fragment;
    fragment.fragment_number = fragment_number;
    encrypt_text(text);
    strncpy(fragment.data, text, BUFFER_SIZE);

    send(sock, &fragment, sizeof(Fragment), 0);
    printf("Fragment %d sent: %s\n", fragment_number, fragment.data);

    close(sock);
    return 0;
}
