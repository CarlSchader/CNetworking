#ifndef CNETWORKING_H
#define CNETWORKING_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h> // socket functions
#include <unistd.h> // POSIX constants and functions for sockaddr constants
#include <netinet/in.h> // library for sockaddr_in
#include <arpa/inet.h> // contains inet_ntoa
#include <pthread.h> // POSIX multithreading
#include <string.h>

struct TCP_thread_args {
	int connection_fd;
	int welcome_socket_fd;
};

void *TCP_thread_echo_server(void *thread_args) { // Function to process a single accepted connection socket. This function is supposed to be run on a thread using pthread. This specific function will echo a connection back to the client.
	struct TCP_thread_args *args = (struct TCP_thread_args *)thread_args;
	char client_message[2048] = {0}; // buffer needed to store the message.
	int message_size = 0;
	while (1) {
		if ((message_size = read(args->connection_fd, client_message, 2048)) <= 0) {
			perror("Error on read().\n");
			exit(EXIT_FAILURE);
			break;
		} // waits for a message to arrive from the connection and returns it. It stores the message in the second arg.
		else {
			printf("Message from client: %s\n", client_message);
			if (strstr(client_message, "CLOSE") != NULL) {
				close(args->connection_fd); // Closes the socket.
				printf("Connection closed.\n");
				break;
			}
			else if (strstr(client_message, "QUIT") != NULL) {
				printf("Shutting down server.\n");
				close(args->connection_fd); // Closes the socket.
				close(args->welcome_socket_fd);
				exit(0);
			}
			else if (write(args->connection_fd, client_message, message_size) < 0) {
				perror("Error on write().\n");
				exit(EXIT_FAILURE);
			} // write() sends a message to the client. it needs a fd, a message, and a message size.
		}
	}
	return NULL;
}

void process_TCP_connection(int welcome_socket_fd, void *(*thread_function)(void *)) { // The thread_function takes two arguments in its void *, 1 is an int that is the connection_fd. and 2 is an int that is the welcome socket fd
	struct sockaddr_in address; // This struct encapsulates the ip/port address that the server is listening on. The _in is for internet which specifies it uses IP.
	socklen_t address_length = sizeof(address);
	
	int connection_fd = -1;
	if ((connection_fd = accept(welcome_socket_fd, (struct sockaddr *)&address, &address_length)) < 0) {
		perror("Connection on accept() failed.\n");
		exit(EXIT_FAILURE);
	} // accepts new connections and creates a new socket for them and returns the file descriptor. It needs the welcome socket descriptor, its address and a point to it's address' length.
	// Accept also fills the passed in sockaddr_in with the new connection's ip/port address.
	else {
		printf("New connection made at: %s port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port)); // inet_ntoa returns a human readable string for the IP. ntohs returns an int  
		pthread_t connection_thread; // Create a new thread pointer.
		struct TCP_thread_args thread_args;
		thread_args.connection_fd = connection_fd;
		thread_args.welcome_socket_fd = welcome_socket_fd;
		pthread_create(&connection_thread, NULL, thread_function, &thread_args); // Begin processing the thread. 
	}
}


int open_TCP_listening_socket(int port_number, int listen_queue_length) { // opens a listening socket for TCP connections
	int welcome_socket_fd;
	if ((welcome_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Could not create welcome socket.\n");
		close(welcome_socket_fd);
		exit(EXIT_FAILURE);
	} // welcome_socket_fd is socket file descriptor. AF_INET is ipv4. SOCK_STREAM is for TCP. 0 is for IP protocal.

	// for some reason when you set socket option it doesn't always connect??? Thats what the below commented out code is for.

	// int option = 1;
	// if (setsockopt(welcome_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)) != 0) {
	// 	perror("Error on setsocketopt().\n");
	// 	close(welcome_socket_fd);
	// 	exit(EXIT_FAILURE);
	// } // Tsetsockopt sets options for the socket. In this case the socket option is to reuse the port number when the program exits. SOL_SOCKET is the level the option is being set on (ie TCP, SOCKET,...).

	struct sockaddr_in address; // This struct encapsulates the ip/port address that the server is listening on. The _in is for internet which specifies it uses IP.
	address.sin_family = AF_INET; // ipv4;
	address.sin_addr.s_addr = INADDR_ANY; // This says listen on all available IP addresses.
	address.sin_port = htons(port_number); // This is the port number. Has to be converted from an int to a "network short".
	socklen_t address_length = sizeof(address);

	if ((bind(welcome_socket_fd, (struct sockaddr *)&address, address_length)) < 0) {
		perror("Could not bind welcome socket to port.\n");
		close(welcome_socket_fd);
		exit(EXIT_FAILURE);
	} // socket_fd, (struct sockaddr *) because &address points to a sockaddr_in type. sockaddr is amore generic type that isn't necessarily IP., sizeof(address) is the size of the address in bytes.
	// bind binds the socket to the port number and IP address.

	if (listen(welcome_socket_fd, listen_queue_length) < 0) {
		perror("Error on listen().\n");
		close(welcome_socket_fd);
		exit(EXIT_FAILURE);
	} // The welcome socket enters a passive mode where it listens for TCP connections with a queue.
	return welcome_socket_fd;
}

#endif