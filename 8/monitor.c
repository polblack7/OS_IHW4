#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define MAX_FRAGMENTS 100

typedef struct {
    int fragment_number;
    char data[BUFFER_SIZE];
} Fragment;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        return -1;
    }

    char* ip = argv[1];
    int port = atoi(argv[2]);
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    Fragment fragments[MAX_FRAGMENTS];
    int fragment_count = 0;
    int fragment_current_count = 0;
    while (1) {
        
        
        // Receive the current state from the server
        if (read(sock, &fragment_count, sizeof(int)) < 0) {
            perror("Read error");
            return -1;
        }
        if (read(sock, fragments, sizeof(Fragment) * fragment_count) < 0) {
            perror("Read error");
            return -1;
        }
        
        if (fragment_current_count != fragment_count) {
            // Display the current state
            printf("Current state:\n");
            printf("Fragment count: %d\n", fragment_count);
            for (int i = 0; i < fragment_count; i++) {
                printf("Fragment %d: %s\n", fragments[i].fragment_number, fragments[i].data);
            }
            fragment_current_count = fragment_count;
        }
        

        sleep(1); // Update every second

    }

    return 0;
}