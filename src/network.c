#include "network.h"
#include "world.h"
#include "engine.h"
#include "visibility.h"
#include "stack.h"

int fdlist[MAX_CLIENTS];
int netClient;
int netServer;
int num_clients;
int maxfd;
int minfd;
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

int get_all(int s, char *buf, int *len)
{
  int total = 0;        // how many bytes we've got
  int initial_len = *len; // how many bytes we were told we had to get
  int bytesleft = initial_len; // how many we still have left to get
  int n;

  while (total < initial_len) {
    n = recv(s, buf + total, bytesleft, 0);

    if (n == -1) { break; }

    total += n;
    bytesleft -= n;
  }

  *len = total; // return number actually got here

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

  for (int i = 0; i < MAX_CLIENTS; i++) {
    fdlist[i] = UNUSED_FD;
  }

  result = bind(server_sockfd, (struct sockaddr *)&server_address, len);

  if (result < 0) {
    perror("bind failed: ");
  }

  // backlog of 5 ought to be enough for my own use
  listen(server_sockfd, 5);

  maxfd = server_sockfd;
  minfd = maxfd;
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

  for (int i = 0; i < MAX_CLIENTS; i++) {
    fdlist[i] = UNUSED_FD;
  }

  result = connect(sockfd, (struct sockaddr *)&address, len);

  if (result < 0) {
    perror("failed to connect: ");
  }

  maxfd = sockfd;
  minfd = maxfd;
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
    tv.tv_usec = 200;
    tv.tv_sec = 0;
    readers = master;
    select(maxfd + 1, &readers, NULL, NULL, &tv);

    if (FD_ISSET(server_socket, &readers)) {
      memset(message, 0, MESSAGE_LENGTH);

      int get_length = MESSAGE_LENGTH;
      if (get_all(server_socket, message, &get_length) < 0) {
        perror("read failed: ");
      }

      // server went away; close server connection
      if (get_length == 0) {
        FD_CLR(server_socket, &master);
        int close_result;
        close_result = close(server_socket);
        printf("closing! status: %d\n", close_result);
        exit(1);
      }

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

  // good debugging tool
  // printf("m: %s\n", message);

  if (strcmp(buf, "done") == 0) {
    strcpy(result, buf);
    return 0;
  }

  if (strcmp(buf, "player") == 0) {
    sscanf(message, "player %d %f %f %f %f %f", &id, &px, &py, &pz, &degx, &degy);
    if (players[id].visible) {
      printf("received player %d %f %f %f %f %f\n", id, px, py, pz, degx, degy);
      setPlayerPosition(id, px, py, pz, degx, degy);
    } else if (id != identity) {
      printf("creating player %d %f %f %f %f %f\n", id, px, py, pz, degx, degy);
      // you're invisible; don't go creating yourself over and over because of it
      createPlayer(id, px, py, pz, degx, degy);
    }
  }

  if (strcmp(buf, "mob") == 0) {
    sscanf(message, "mob %d %f %f %f %f %f", &id, &px, &py, &pz, &degx, &degy);
    showMob(id);
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
  int id;

  struct timeval tv;
  char buf[MESSAGE_LENGTH], message[MESSAGE_LENGTH];

  while (1) {
    tv.tv_usec = 50;
    tv.tv_sec = 0;
    readers = master;
    select(maxfd + 1, &readers, NULL, NULL, &tv);

    if (FD_ISSET(sockfd, &readers)) {
      memset(buf, 0, MESSAGE_LENGTH);

      int get_length = MESSAGE_LENGTH;
      if (get_all(sockfd, message, &get_length) < 0) {
        perror("read failed: ");
      }

      // server went away; close server connection
      if (get_length == 0) {
        FD_CLR(sockfd, &master);
        int close_result;
        close_result = close(sockfd);
        printf("closing! status: %d\n", close_result);
        exit(1);
      }

      sscanf(message, "%s", buf);
      // good debug for world-building:
      // printf("received \"%s\"\n", message);

      if (strcmp(buf, "world") == 0) {
        sscanf(message, "world %d", &identity);
      } else if (strcmp(buf, "player") == 0) {
        sscanf(message, "player %d %f %f %f %f %f", &id, &px, &py, &pz, &rx, &ry);
        createPlayer(id, px, py, pz, rx, ry);
      } else if (strcmp(buf, "mob") == 0) {
        sscanf(message, "mob %d %f %f %f %f %f", &id, &px, &py, &pz, &rx, &ry);
        createMob(id, px, py, pz, rx, ry);
      } else if (strcmp(buf, "b") == 0) {
        if (sscanf(message, "b %d %d %d %d", &x, &y, &z, &t) <= 0) {
          perror("scanf: ");
          fprintf(stderr, "%d %d %d %d\n", x, y, z, t);
        }

        // printf("%d,%d,%d=%d",x,y,z,t);
        // 4 -1's terminates the block buffer
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

int send_game_over_network(int sockfd, int player_id)
{
  int i, j, k;
  float px, py, pz;
  float rx, ry, rz;
  char buf[MESSAGE_LENGTH];
  memset(buf, 0, MESSAGE_LENGTH);
  // write player id for them to take as their identity; should be non-zero because the server is 0
  sprintf(buf, "world %d", player_id);
  printf("wrote \"%s\"\n", buf);
  sendall(sockfd, buf, MESSAGE_LENGTH);

  for (i = 0; i < PLAYER_COUNT; i++) {
    if (players[i].visible || i == 0) { // i == 0 here to ensure server sends its position, despite not being locally visible
      memset(buf, 0, MESSAGE_LENGTH);
      if (i == 0) {
        getViewPosition(&px, &py, &pz);
        getViewOrientation(&rx, &ry, &rz);
        sprintf(buf, "player %d %f %f %f %f %f", 0, px, py, pz, ry, rx);
      } else {
        sprintf(buf, "player %d %f %f %f %f %f", i, players[i].pos.x, players[i].pos.y, players[i].pos.z, players[i].rot.x, players[i].rot.y);
      }

      printf("wrote \"%s\"\n", buf);
      sendall(sockfd, buf, MESSAGE_LENGTH);
    }
  }

  for (i = 0; i < MOB_COUNT; i++) {
    printf("mob %d\n", i);
    if (mobflag[i]) {
      memset(buf, 0, MESSAGE_LENGTH);
      sprintf(buf, "mob %d %f %f %f %f %f", i, mobPosition[i][0], mobPosition[i][1], mobPosition[i][2], mobPosition[i][3], mobPosition[i][4]);
      printf("wrote \"%s\"\n", buf);
      sendall(sockfd, buf, MESSAGE_LENGTH);
    }
  }

  //sendall(sockfd, buf, MESSAGE_LENGTH / 2);
  //printf("wrote \"%s\"\n", buf);

  for (i = 0; i < WORLDX; i++) {
    for (j = 0; j < WORLDY; j++) {
      for (k = 0; k < WORLDZ; k++) {
        if (world[i][j][k] != EMPTY) {
          memset(buf, 0, MESSAGE_LENGTH);
          sprintf(buf, "b %d %d %d %d", i, j, k, world[i][j][k]);
          // good debug for world-building
          // printf("sending \"%s\"\n", buf);
          sendall(sockfd, buf, MESSAGE_LENGTH);
          //printf("ijk: %d,%d,%d\n", i, j, k);
        }
      }
    }
  }

  memset(buf, 0, MESSAGE_LENGTH);
  sprintf(buf, "b -1 -1 -1 -1");
  sendall(sockfd, buf, MESSAGE_LENGTH);
  printf("-1's\n");

  return 0;
}

int get_stuff_from_client()
{
  int get_result, i, j, activity;
  int client_len;
  int next_client_fd = UNSET_NEXT_CLIENT_FD;
  struct sockaddr_in client_address;
  struct timeval tv;
  char result[MESSAGE_LENGTH], message[MESSAGE_LENGTH];
  strcpy(result, "not done");

  while (strcmp(result, "done") != 0) {
    tv.tv_usec = 200;
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
      for (j = 0; j < MAX_CLIENTS; j++) {
        if (fdlist[j] == UNUSED_FD) {
          next_client_fd = j;
          break;
        }
      }
      if (next_client_fd == UNSET_NEXT_CLIENT_FD) {
        perror("too many clients!");
        // no open slots; just gonna ignore the connection for now
        continue;
      }
      fdlist[next_client_fd] = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
      FD_SET(fdlist[next_client_fd], &master);

      if (fdlist[next_client_fd] > maxfd) {
        maxfd = fdlist[next_client_fd];
      }

      printf("new player! maxfd: %d, next_client_fd: %d, num_clients: %d, bumping to %d, creating player %d", maxfd, next_client_fd, num_clients, num_clients+1, next_client_fd+1);
      int new_player_id = next_client_fd + 1;
      send_game_over_network(fdlist[next_client_fd], new_player_id);
      num_clients++;
      showPlayer(new_player_id);
    }

    //  printf("oh I left\n");
    for (i = 0; i < MAX_CLIENTS; i++) {
      if (fdlist[i] != UNUSED_FD && FD_ISSET(fdlist[i], &readers)) {
        activity = 1;

        int get_length = MESSAGE_LENGTH;
        if ((get_result = get_all(fdlist[i], message, &get_length)) == 0) {
          if (get_length == 0) {

            printf("closing connection %d, setting fdlist[%d]=fdlist[%d], hiding player %d\n", i, i, num_clients - 1, num_clients);
            FD_CLR(fdlist[i], &master);
            close(fdlist[i]);
            fdlist[i] = UNUSED_FD;

            int player_id = i + 1;
            hidePlayer(player_id);
            setPlayerPosition(player_id, 0, 0, 0, 0, 0);
            players[player_id].flag = true;
            num_clients--;

            // reset maxfd
            maxfd = minfd;
            for (j = 0; j < MAX_CLIENTS; j++) {
              // the UNUSED_FD guard is not strictly needed, but is a kind of "imaginary guard"
              // representing that the unused values are never going to be valid maxfds
              if (fdlist[j] != UNUSED_FD && fdlist[j] > maxfd) {
                maxfd = fdlist[j];
              }
            }

            printf("at end of close, maxfd is %d, num_clients is %d\n", maxfd, num_clients);

          } else {
            printf("processing message from client: '%s'\n", message);
            process_client_message(message);
          }
        } else {
          if (get_result < 0) {
            perror("failure during read: ");
          } // implicit else: get_length was likely 0, meaning the client cleanly closed the connection
          printf("closing connection %d, setting fdlist[%d]=fdlist[%d], hiding player %d\n", i, i, num_clients - 1, num_clients);
          FD_CLR(fdlist[i], &master);
          int close_result;
          close_result = close(fdlist[i]);
          printf("closing! status: %d\n", close_result);
          fdlist[i] = UNUSED_FD;

          int player_id = i + 1;
          hidePlayer(player_id);
          setPlayerPosition(player_id, 0, 0, 0, 0, 0);
          players[player_id].flag = true;
          num_clients--;

          // reset maxfd
          maxfd = minfd;
          for (j = 0; j < MAX_CLIENTS; j++) {
            // the UNUSED_FD guard is not strictly needed, but is a kind of "imaginary guard"
            // representing that the unused values are never going to be a valid maxfd
            if (fdlist[j] != UNUSED_FD && fdlist[j] > maxfd) {
              maxfd = fdlist[j];
            }
          }

          printf("at end of close, maxfd is %d, num_clients is %d\n", maxfd, num_clients);
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
    printf("received player %d %f %f %f %f %f\n", id, px, py, pz, degx, degy);
    setPlayerPosition(id, px, py, pz, degx, degy);
    players[id].flag = true;
    return 0;
  }

  if (strcmp(buf, "dig") == 0) {
    sscanf(message, "dig %d %d %d", &dx, &dy, &dz);
    printf("got dig: %d %d %d\n", dx, dy, dz);
    world[dx][dy][dz] = EMPTY;
    digflag[0] = 1;
    digflag[1] = dx;
    digflag[2] = dy;
    digflag[3] = dz;
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
    if (players[i].flag) {
      players[i].flag = false;

      if (i == 0) {
        getViewPosition(&px, &py, &pz);
        getViewOrientation(&rx, &ry, &rz);
        printf("sending player %d %f %f %f %f %f\n", 0, px, py, pz, ry, rx);
        sprintf(buf, "player %d %f %f %f %f %f", 0, px, py, pz, ry, rx);
      } else {
        printf("relaying player %d %f %f %f %f %f\n", i, players[i].pos.x, players[i].pos.y, players[i].pos.z, players[i].rot.x, players[i].rot.y);
        sprintf(buf, "player %d %f %f %f %f %f", i, players[i].pos.x, players[i].pos.y, players[i].pos.z, players[i].rot.x, players[i].rot.y);
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
    for (i = 0; i < MAX_CLIENTS; i++) {
      if (fdlist[i] != UNUSED_FD) {
        sendall(fdlist[i], buf, MESSAGE_LENGTH);
      }
    }
  }

  for (i = 0; i < MAX_CLIENTS; i++) {
    if (fdlist[i] != UNUSED_FD) {
      sendall(fdlist[i], "done", MESSAGE_LENGTH);
    }
  }

  kill_stack(&s);

  return 0;
}

int send_stuff_to_server()
{
  float px, py, pz, rx, ry, rz;
  char buf[MESSAGE_LENGTH];

  if (players[identity].flag) {
    players[identity].flag = false;
    getViewPosition(&px, &py, &pz);
    getViewOrientation(&rx, &ry, &rz);
    memset(buf, 0, MESSAGE_LENGTH);
    printf("sending player %d %f %f %f %f %f\n", identity, px, py, pz, ry, rx);
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
