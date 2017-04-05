#include "uart.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

pthread_t uart_thread;
int uart_socket = -1;

static void* uart_run(void* args) {
	while(true) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int client_socket = accept(uart_socket, (struct sockaddr*)&client_addr, &client_addr_len);
		printf("UART peer connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

		char client_message[512];
		while(recv(client_socket, client_message, sizeof(client_message), 0) > 0) {
		}
	}

	return NULL;
}

void uart_init(int argc, char** argv) {
	int uart_port = 0;
	for(int i = 1; i < argc; ++i) {
		if(strcmp(argv[i], "-u") == 0) {
			if(i + 1 >= argc || (uart_port = atoi(argv[++i])) <= 0) {
				printf("Missing or invalid UART port number\n");
				exit(1);
			}
			continue;
		}
	}

	if(uart_port <= 0) {
		printf("UART not initialized\n");
		return;
	}

	struct sockaddr_in listen_addr;
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	listen_addr.sin_port = htons(uart_port);

	uart_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(uart_socket < 0) {
		printf("UART socket creation failed (%d)\n", uart_socket);
		exit(2);
	}

	int err = bind(uart_socket, (struct sockaddr*)&listen_addr, sizeof(listen_addr));
	if(err < 0) {
		printf("UART socket bind to port %d failed (%d)\n", uart_port, err);
		exit(2);
	}
	err = listen(uart_socket, 1);
	if(err < 0) {
		printf("UART socket listen failed (%d)\n", err);
		exit(2);
	}

	printf("UART listening on port %d\n", uart_port);
}

void uart_start(void) {
	if(uart_socket >= 0) {
		pthread_create(&uart_thread, NULL, uart_run, NULL);
	}
}
