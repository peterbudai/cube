#include "uart.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "io.h"

pthread_t uart_thread;
int uart_socket = -1;

static void* uart_run(void* args __attribute__((unused))) {
	while(true) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int client_socket = accept(uart_socket, (struct sockaddr*)&client_addr, &client_addr_len);
		printf("UART peer connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

		while(true) {
			char buf[UART_BUFFER_SIZE];
			size_t ilen = 0;
			ssize_t olen = -1;

			// Wait for network event to happen
			fd_set read_fd, write_fd, error_fd;
			FD_ZERO(&read_fd);
			FD_ZERO(&write_fd);
			FD_ZERO(&error_fd);
			FD_SET(client_socket, &read_fd);
			if(!uart_empty(UART_OUTPUT)) {
				FD_SET(client_socket, &write_fd);
			}
			FD_SET(client_socket, &error_fd);

			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 100000;
			olen = select(client_socket + 1, &read_fd, &write_fd, &error_fd, &timeout);
			if(olen < 0) {
				printf("Error while waiting for UART data: %d\n", errno);
				break;
			}
			if(olen == 0) {
				continue;
			}

			// Handle network error
			if(FD_ISSET(client_socket, &error_fd)) {
				printf("Error in UART connection\n");
				break;
			}

			// Transfer from outside to MCU
			if(FD_ISSET(client_socket, &read_fd)) {
				ilen = sizeof(buf);
				olen = recv(client_socket, buf, ilen, MSG_DONTWAIT);
				if(olen < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
					printf("Error on receiving UART data: %d\n", errno);
					break;
				}
				if(olen == 0) {
					printf("UART connection terminated\n");
					break;
				}
				if(olen > 0) {
					uart_push_back(UART_INPUT, (uint8_t*)buf, (size_t)olen);
				}
			}

			// Transfer from MCU to outside
			if(FD_ISSET(client_socket, &write_fd)) {
				ilen = uart_peek_front(UART_OUTPUT, (uint8_t*)buf, sizeof(buf));
				olen = send(client_socket, buf, ilen, MSG_DONTWAIT);
				if(olen < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
					printf("Error on sending UART data: %d\n", errno);
					break;
				}
				if(olen > 0) {
					uart_pop_front(UART_INPUT, (size_t)olen);
				}
			}
		}
		close(client_socket);
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
