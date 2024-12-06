#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 10
#define PORT 8080
#define BUFFER_SIZE 1024

struct Client {
    int socket;
    char username[50];  
};

struct Client clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *serveClient(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];
    char username[50];

    // Get the username from the client
    recv(client_socket, username, sizeof(username), 0);
    printf("Client connected with username: %s\n", username);

    // Store the client's username
    pthread_mutex_lock(&mutex);
    strcpy(clients[num_clients].username, username);
    clients[num_clients].socket = client_socket;
    ++num_clients;
    pthread_mutex_unlock(&mutex);

    while (1) {
        // Receive message from client
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
          
            break;
        }

        // Process received message and forward to all clients
        buffer[bytes_received] = '\0';
        printf("Received message from %s: %s\n", username, buffer);

        // Broadcast the message to all clients
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < num_clients; ++i) {
           
            if (clients[i].socket != client_socket) {
               
                char message_with_username[BUFFER_SIZE];
                snprintf(message_with_username, sizeof(message_with_username), "%s: %s", username, buffer);
                send(clients[i].socket, message_with_username, strlen(message_with_username), 0);
            }
        }
        pthread_mutex_unlock(&mutex);

        // If the client sends "bye", disconnect and notify all others
        if (strcmp(buffer, "bye") == 0) {
            char disconnect_message[BUFFER_SIZE];
            snprintf(disconnect_message, sizeof(disconnect_message), "%s has disconnected.", username);

          
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < num_clients; ++i) {
                if (clients[i].socket != client_socket) {
                    send(clients[i].socket, disconnect_message, strlen(disconnect_message), 0);
                }
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
    }


    pthread_mutex_lock(&mutex);
    for (int i = 0; i < num_clients; ++i) {
        if (clients[i].socket == client_socket) {
            while (i < num_clients - 1) {
                clients[i] = clients[i + 1];
                ++i;
            }
            break;
        }
    }
    --num_clients;
    pthread_mutex_unlock(&mutex);

    close(client_socket);
    return NULL;
}

int main() {
    int server_socket;
    struct sockaddr_in server_addr;

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for connections...\n");

    while (1) {
        // Accept incoming connections
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        // Create a new thread to handle the client
        pthread_t thread;
        pthread_create(&thread, NULL, serveClient, &client_socket);
    }

    close(server_socket);
    return 0;
}
