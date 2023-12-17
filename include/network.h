#ifndef _NETWORK
#define _NETWORK
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define MESSAGE_LENGTH 128
#define RESPONSE_LENGTH 128
#define MAX_CLIENTS 100
#define SERVER 0
#define PORT 54321

extern int fdlist[11];
extern int maxfd;
extern int num_clients;
extern int server_socket;
extern int identity;
extern fd_set master;
extern fd_set readers;
extern int netClient;
extern int netServer;

int send_game_over_network(int sockfd);
int load_game_over_network(int sockfd);

int server_setup();
int client_setup();

int get_stuff_from_server();
int get_stuff_from_client();

int send_stuff_to_clients();
int send_stuff_to_server();

int process_server_message(char *message, char *result);
int process_client_message(char *message);
#endif
