#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *receiveMessages(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while (1) {
        // Receive messages from server
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
           
            break;
        }

        // Process received messages and print them
        buffer[bytes_received] = '\0';
        printf("%s\n\n", buffer);  
    }

    close(client_socket);
    return NULL;
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char username[50];

    // Ask user for a username
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0'; 

    // Create client socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Replace the IP address here with the server's IP address
    server_addr.sin_addr.s_addr = inet_addr(""); 

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Send the username to the server
    send(client_socket, username, strlen(username), 0);

    printf("Connected to server as %s.\n", username);

    // Create a thread to receive messages from the server
    pthread_t thread;
    pthread_create(&thread, NULL, receiveMessages, &client_socket);

    char message[BUFFER_SIZE];

    while (1) {    
        // Read user input and send message to server
        printf("Enter message: \n");        
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';

        // Send the message to the server
        if (strlen(message) == 0) {
            printf("No message entered.\n");
            continue;
        }

        send(client_socket, message, strlen(message), 0);
        
      
        if (strcmp(message, "bye") == 0) {
            break;
        }
    }

    close(client_socket);
    return 0;
}
