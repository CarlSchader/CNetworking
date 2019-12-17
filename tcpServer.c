#include "cnetworking.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h> // socket functions
#include <unistd.h> // POSIX constants and functions for sockaddr constants
#include <netinet/in.h> // library for sockaddr_in
#include <arpa/inet.h> // contains inet_ntoa
#include <pthread.h> // POSIX multithreading
#include <string.h>

/*
	Command line arguments: port number, TCP listening socket queue length
*/

int main(int argc, char *argv[]) { 
	int port = atoi(argv[1]);
	int listening_queue_length = atoi(argv[2]);

	int welcome_socket_fd = open_TCP_listening_socket(port, listening_queue_length);

	while (1) process_TCP_connection(welcome_socket_fd, TCP_thread_echo_server);
	return 0;
}