#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024
#define MAX_FRAGMENTS 100
#define MAX_CLIENTS 10

typedef struct {
    int fragment_number;
    char data[BUFFER_SIZE];
} Fragment;

Fragment fragments[MAX_FRAGMENTS];
int fragment_count = 0;
pthread_mutex_t lock;

void* handle_client(void* socket_desc) {
    int new_socket = *(int*)socket_desc;
    Fragment fragment;

    // Receive fragment
    read(new_socket, &fragment, sizeof(Fragment));

    // Store fragment in a thread-safe manner
    pthread_mutex_lock(&lock);
    fragments[fragment_count++] = fragment;
    pthread_mutex_unlock(&lock);

    // Print received fragment
    printf("Received fragment %d: %s\n", fragment.fragment_number, fragment.data);

    close(new_socket);
    pthread_exit(NULL);
}

void* handle_monitor(void* socket_desc) {
    int new_socket = *(int*)socket_desc;
    printf("Sending current state to monitor\n");
    while (1) {
        // Send the current state to the monitoring client
        pthread_mutex_lock(&lock);
        
        send(new_socket, &fragment_count, sizeof(int), 0);
        send(new_socket, fragments, sizeof(Fragment) * fragment_count, 0);
        pthread_mutex_unlock(&lock);

        sleep(1); // Update every second
    }

    close(new_socket);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[1]);
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);
    pthread_mutex_init(&lock, NULL);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue listening for new connections
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)&new_socket) < 0) {
            perror("could not create thread");
            close(new_socket);
            continue; // Continue listening for new connections
        }

        // Create a separate thread for handling each monitor
        if (pthread_create(&thread_id, NULL, handle_monitor, (void*)&new_socket) < 0) {
            perror("could not create monitor thread");
            close(new_socket);
            continue; // Continue listening for new connections
        }
    }

    pthread_mutex_destroy(&lock);
    return 0;
}
