#include "network.h"
#include "world.h"
#include "engine.h"
#include "visibility.h"
#include "stack.h"

int fdlist[11];
int netClient;
int netServer;
int num_clients;
int maxfd;
int identity;
int server_socket;
fd_set master;
fd_set readers;

/* this function is straight from Beej's guide to network programming
http://beej.us/guide/bgnet/output/html/multipage/advanced.html#sendall */
int sendall(int s, char *buf, int len)
{
  int total = 0;        // how many bytes we've sent
  int bytesleft = len; // how many we have left to send
  int n;

  while (total < len) {
    n = send(s, buf + total, bytesleft, 0);

    if (n == -1) { break; }

    total += n;
    bytesleft -= n;
  }

  len = total; // return number actually sent here

  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int get_all(int s, char *buf, int len)
{
  int total = 0;        // how many bytes we've got
  int bytesleft = len; // how many we have left to get
  int n;

  while (total < len) {
    n = recv(s, buf + total, bytesleft, 0);

    if (n == -1) { break; }

    total += n;
    bytesleft -= n;
  }

  len = total; // return number actually sent here

  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int server_setup()
{
  int server_sockfd, len, result;
  struct sockaddr_in server_address;

  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(PORT);
  len = sizeof(server_address);

  result = bind(server_sockfd, (struct sockaddr *)&server_address, len);

  if (result < 0) {
    perror("bind failed: ");
  }

  listen(server_sockfd, 5);

  maxfd = server_sockfd;
  FD_ZERO(&master);
  FD_ZERO(&readers);
  FD_SET(server_sockfd, &master);

  return server_sockfd;
}

int client_setup()
{
  int sockfd, len, result;
  struct sockaddr_in address;
  //char buf[MESSAGE_LENGTH];
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons(PORT);
  len = sizeof(address);

  result = connect(sockfd, (struct sockaddr *)&address, len);

  if (result < 0) {
    perror("failed to connect: ");
  }

  maxfd = sockfd;
  FD_ZERO(&master);
  FD_ZERO(&readers);
  FD_SET(sockfd, &master);

  return sockfd;
}

int get_stuff_from_server()
{
  struct timeval tv;
  char result[MESSAGE_LENGTH], message[MESSAGE_LENGTH];
  memset(result, 0, MESSAGE_LENGTH);
  memset(message, 0, MESSAGE_LENGTH);
  strcpy(result, "not done");

  while (strcmp(result, "done") != 0) {
    tv.tv_usec = 50000;
    tv.tv_sec = 0;
    readers = master;
    select(maxfd + 1, &readers, NULL, NULL, &tv);

    if (FD_ISSET(server_socket, &readers)) {
      memset(message, 0, MESSAGE_LENGTH);
      read(server_socket, message, MESSAGE_LENGTH);
      process_server_message(message, result);
    } else {
      strcpy(result, "done");
    }
  }

  return 0;
}

int process_server_message(char *message, char *result)
{
  float px, py, pz, degx, degy;
  int x, y, z, id;
  char buf[MESSAGE_LENGTH];

  if (message == NULL || result == NULL) {
    fprintf(stderr, "NULL while attempting to process server message.\n");
  }

  sscanf(message, "%s", buf);

  if (strcmp(buf, "done") == 0) {
    strcpy(result, buf);
    return 0;
  }

  if (strcmp(buf, "player") == 0) {
    sscanf(message, "player %d %f %f %f %f %f", &id, &px, &py, &pz, &degx, &degy);
    setPlayerPosition(id, px, py, pz, degx, degy);
    printf("m: %s\n", message);
  }

  if (strcmp(buf, "mob") == 0) {
    sscanf(message, "mob %d %f %f %f %f %f", &id, &px, &py, &pz, &degx, &degy);
    setMobPosition(id, px, py, pz, degx, degy);
  }

  if (strcmp(buf, "sun") == 0) {
    sscanf(message, "sun %f %f %f", &px, &py, &pz);
    setLightPosition(px, py, pz);
  }

  if (strcmp(buf, "dig") == 0) {
    sscanf(message, "dig %d %d %d", &x, &y, &z);
    world[x][y][z] = EMPTY;
    printf("m: %s\n", message);
    trimout();
  }

  if (strcmp(buf, "cloud") == 0) {
    sscanf(message, "cloud %d %d %d %d", &x, &y, &z, &id);
    world[x][y][z] = id;
  }

  return 0;
}

int load_game_over_network(int sockfd)
{
  int x = 0, y = 0, z = 0, t = 0;
  float px, py, pz, rx, ry;
  int player_id;

  struct timeval tv;
  char buf[MESSAGE_LENGTH], message[MESSAGE_LENGTH];

  while (1) {
    tv.tv_usec = 50;
    tv.tv_sec = 0;
    readers = master;
    select(maxfd + 1, &readers, NULL, NULL, &tv);

    if (FD_ISSET(sockfd, &readers)) {
      memset(buf, 0, MESSAGE_LENGTH);

      if (get_all(sockfd, message, MESSAGE_LENGTH) < 0) {
        perror("read failed: ");
      }

      sscanf(message, "%s", buf);
      printf("received \"%s\"\n", message);

      if (strcmp(buf, "world") == 0) {
        sscanf(message, "world %d", &identity);
      } else if (strcmp(buf, "player") == 0) {
        sscanf(message, "player %d %f %f %f %f %f", &player_id, &px, &py, &pz, &rx, &ry);
        createPlayer(player_id, px, py, pz, rx, ry);
      } else if (strcmp(buf, "b") == 0) {
        if (sscanf(message, "b %d %d %d %d", &x, &y, &z, &t) <= 0) {
          perror("scanf: ");
          fprintf(stderr, "%d %d %d %d\n", x, y, z, t);
        }

        // printf("%d,%d,%d=%d",x,y,z,t);
        if (x == -1) {
          return 0;
        }

        world[x][y][z] = t;
      }
    } else {
      fprintf(stderr, "waiting for connection\n");
      break;
    }
  }

  return 1;
}

int send_game_over_network(int sockfd)
{
  int i, j, k;
  float px, py, pz;
  float rx, ry, rz;
  char buf[MESSAGE_LENGTH];
  memset(buf, 0, MESSAGE_LENGTH);
  // num_clients = 0 initially, and this is currently used as an
  // identity for clients joining, so we'll make it non-zero to
  // start. Maybe that'll be a problem!
  sprintf(buf, "world %d", num_clients + 1);
  printf("wrote \"%s\"\n", buf);
  sendall(sockfd, buf, MESSAGE_LENGTH);

  for (i = 0; i <= num_clients; i++) {
    memset(buf, 0, MESSAGE_LENGTH);

    if (i == 0) {
      getViewPosition(&px, &py, &pz);
      getViewOrientation(&rx, &ry, &rz);
      sprintf(buf, "player %d %f %f %f %f %f", 0, px, py, pz, ry, rx);
    } else {
      sprintf(buf, "player %d %f %f %f %f %f", i, playerPosition[i][0], playerPosition[i][1], playerPosition[i][2], playerPosition[i][3], playerPosition[i][4]);
    }

    printf("wrote \"%s\"\n", buf);
    sendall(sockfd, buf, MESSAGE_LENGTH);
  }

  //sendall(sockfd, buf, MESSAGE_LENGTH / 2);
  //printf("wrote \"%s\"\n", buf);

  for (i = 0; i < WORLDX; i++) {
    printf("%d", i);

    for (j = 0; j < WORLDY; j++) {
      for (k = 0; k < WORLDZ; k++) {
        if (world[i][j][k] != EMPTY) {
          memset(buf, 0, MESSAGE_LENGTH);
          sprintf(buf, "b %d %d %d %d", i, j, k, world[i][j][k]);
          printf("sending \"%s\"\n", buf);
          sendall(sockfd, buf, MESSAGE_LENGTH);
          //printf("ijk: %d,%d,%d\n", i, j, k);
        }
      }
    }
  }

  printf("\n");
  memset(buf, 0, MESSAGE_LENGTH);
  sprintf(buf, "b -1 -1 -1 -1");
  sendall(sockfd, buf, MESSAGE_LENGTH);
  printf("-1's\n");

  return 0;
}

int get_stuff_from_client()
{
  int client_len, i, activity;
  struct sockaddr_in client_address;
  struct timeval tv;
  char result[MESSAGE_LENGTH], message[MESSAGE_LENGTH];
  strcpy(result, "not done");

  while (strcmp(result, "done") != 0) {
    tv.tv_usec = 50000;
    tv.tv_sec = 0;
    readers = master;

    if (select(maxfd + 1, &readers, NULL, NULL, &tv) < 0) {
      perror("select failed: ");
    }

    // printf("well I'm here\n");
    activity = 0;

    if (FD_ISSET(server_socket, &readers)) {
      // printf("and I'm trying\n");
      activity = 1;
      client_len = sizeof(client_address);
      fdlist[num_clients] = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
      FD_SET(fdlist[num_clients], &master);

      if (fdlist[num_clients] > maxfd) {
        maxfd = fdlist[num_clients];
      }

      send_game_over_network(fdlist[num_clients]);
      num_clients++;
      createPlayer(num_clients, -50, -80, -50, 0, 0);
    }

    //  printf("oh I left\n");
    for (i = 0; i < num_clients; i++) {
      //printf("in here %d\n", i);
      if (FD_ISSET(fdlist[i], &readers)) {
        activity = 1;

        //printf("in FD_ISSET\n");
        // <0 indicates error
        if ((client_len = get_all(fdlist[i], message, MESSAGE_LENGTH)) < 0) {
          // closing connections is probably overall broken
          //printf("in <0 branch; message: %s\n", message);
          if (client_len < 0) {
            perror("failure during read: ");
          // this used to be an else-block:
          // } else {
            printf("closing connection\n");
            FD_CLR(fdlist[i], &master);
            close(fdlist[i]);
            fdlist[i] = fdlist[num_clients - 1];
            hidePlayer(num_clients);
            num_clients--;
            // maxfd = 0;

            // this code for reusing ids likely needs some more thought
            // for (j = 0; j < num_clients; j++) {
            //  if (fdlist[j] > maxfd) {
            //    maxfd = fdlist[j];
            //  }
            // }
          }
        } else {
          //printf("processing message from client: '%s'", message);
          process_client_message(message);
        }
      }
    }

    if (activity == 0) {
      strcpy(result, "done");
    }
  }

  return 0;
}

int process_client_message(char *message)
{
  float px, py, pz, degx, degy;
  int dx, dy, dz;
  int id;
  char buf[MESSAGE_LENGTH];

  if (message == NULL) {
    fprintf(stderr, "NULL while attempting to process client message.\n");
  }

  sscanf(message, "%s", buf);

  if (strcmp(buf, "done") == 0) {
    return 0;
  }

  if (strcmp(buf, "player") == 0) {
    sscanf(message, "player %d %f %f %f %f %f", &id, &px, &py, &pz, &degx, &degy);
    setPlayerPosition(id, px, py, pz, degx, degy);
    player_flag[id] = 0;
    return 0;
  }

  if (strcmp(buf, "dig") == 0) {
    sscanf(message, "dig %d %d %d", &dx, &dy, &dz);
    printf("got dig: %d %d %d\n", dx, dy, dz);
    world[dx][dy][dz] = EMPTY;
    trimout();
    return 0;
  }

  fprintf(stderr, "unexpected message from client: %s\n", buf);

  return 0;
}


int send_stuff_to_clients()
{
  float px, py, pz, rx, ry, rz;
  int i, j;
  char buf[MESSAGE_LENGTH];
  Stack s;
  new_stack(&s);

  for (i = 0; i < PLAYER_COUNT; i++) {
    if (player_flag[i]) {
      player_flag[i] = 0;

      if (i == 0) {
        getViewPosition(&px, &py, &pz);
        getViewOrientation(&rx, &ry, &rz);
        sprintf(buf, "player %d %f %f %f %f %f", 0, px, py, pz, ry, rx);
      } else {
        sprintf(buf, "player %d %f %f %f %f %f", i, playerPosition[i][0], playerPosition[i][1], playerPosition[i][2], playerPosition[i][3], playerPosition[i][4]);
      }

      push(s, buf);
    }
  }

  if (sun_flag) {
    sun_flag = 0;
    getLightPosition(&px, &py, &pz);
    sprintf(buf, "sun %d %d %d", (int)px, (int)py, (int)pz);
    push(s, buf);
  }

  if (clouds_flag) {
    clouds_flag = 0;

    for (i = 0; i < WORLDX; i++) {
      for (j = 0; j < WORLDZ; j++) {
        if (world[i][96][j] == WHITE) {
          sprintf(buf, "cloud %d 96 %d %d", i, j, WHITE);
          push(s, buf);
        }
      }
    }
  }

  for (i = 0; i < MOB_COUNT; i++) {
    if (mobflag[i]) {
      mobflag[i] = 0;
      sprintf(buf, "mob %d %f %f %f %f %f", i, mobPosition[i][0], mobPosition[i][1], mobPosition[i][2], mobPosition[i][3], mobPosition[i][4]);
      push(s, buf);
    }
  }

  if (digflag[0]) {
    digflag[0] = 0;
    sprintf(buf, "dig %d %d %d", digflag[1], digflag[2], digflag[3]);
    push(s, buf);
  }

  while (pop(s, buf) == 0) {
    for (i = 0; i < num_clients; i++) {
      sendall(fdlist[i], buf, MESSAGE_LENGTH);
    }
  }

  for (i = 0; i < num_clients; i++) {
    sendall(fdlist[i], "done", MESSAGE_LENGTH);
  }

  kill_stack(&s);

  return 0;
}

int send_stuff_to_server()
{
  float px, py, pz, rx, ry, rz;
  char buf[MESSAGE_LENGTH];

  if (player_flag[identity]) {
    player_flag[identity] = 0;
    getViewPosition(&px, &py, &pz);
    getViewOrientation(&rx, &ry, &rz);
    memset(buf, 0, MESSAGE_LENGTH);
    sprintf(buf, "player %d %f %f %f %f %f", identity, px, py, pz, ry, rx);
    sendall(server_socket, buf, MESSAGE_LENGTH);
  }

  if (digflag[0]) {
    digflag[0] = 0;
    memset(buf, 0, MESSAGE_LENGTH);
    printf("sending dig %d %d %d\n", digflag[1], digflag[2], digflag[3]);
    sprintf(buf, "dig %d %d %d", digflag[1], digflag[2], digflag[3]);
    sendall(server_socket, buf, MESSAGE_LENGTH);
  }

  sendall(server_socket, "done", MESSAGE_LENGTH);

  return 0;
}
