#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "auth.h"
#include "socket.h"
#include "message.h"
#include "packet.h"
#include "config.h"
#include "game.h"

sig_atomic_t running = 1;

void sigintHandler(int signum) {
    running = 0;
}

void *main_loop(void *args) {
    int sock, running_loop = 1;
    char buf[MAX_BUF];
    UserData *userData = NULL;
    Game *game = NULL;

    Message *message = (Message *) &buf;
    Message_Login *message_login = (Message_Login *) &buf;
    Message_Profile *message_profile = (Message_Profile *) &buf;
    Message_Join *message_join = (Message_Join *) &buf;
    Message_Say *message_say = (Message_Say *) &buf;

    pthread_detach(pthread_self());

    sock = *(int *) args;
    free(args);

    memset(buf, 0, sizeof(Message));
    message->type = MES_REQUEST_LOGIN;
    if (packet_send(sock, buf, sizeof(Message)) < 0)
        running_loop = 0;

    while (running && running_loop) {
        if (packet_receive(sock, buf) < 0) break;

        switch (message->type) {
            case MES_QUIT:
                running_loop = 0;
                break;

            case MES_LOGIN:
                if (auth_login(message_login->username, message_login->password, &userData) == 0) {
                    memset(buf, 0, sizeof(Message_Profile));
                    message_profile->type = MES_PROFILE;
                    message_profile->status = STATUS_OK;
                    strcpy(message_profile->username, userData->username);
                    strcpy(message_profile->password, userData->password);
                    message_profile->chip = userData->chip;
                    packet_send(sock, buf, sizeof(Message_Profile));

                    if (userData->chip == 0) {
                        memset(buf, 0, sizeof(Message));
                        message->type = MES_QUIT;
                        packet_send(sock, buf, sizeof(Message));
                    }

                } else {
                    memset(buf, 0, sizeof(Message_Profile));
                    message_profile->type = MES_PROFILE;
                    message_profile->status = STATUS_NG;
                    packet_send(sock, buf, sizeof(Message_Profile));

                    memset(buf, 0, sizeof(Message));
                    message->type = MES_QUIT;
                    packet_send(sock, buf, sizeof(Message));
                }
                break;

            case MES_JOIN:
                game = game_join(sock, userData, message_join->bet);
                break;

            case MES_SAY:
                game_say(sock, &game, message_say->say);
                break;

            case MES_RESULT:
                if (game != NULL)
                    game = NULL;
                break;

            case MES_REQUEST_LOGIN:
            case MES_PROFILE:
            default:
                printf("main_loop(): Unknown packet recieved. [%d]\n", message->type);
                break;
        }
    }

    if (game != NULL)
        game_surrender(sock, &game);

    socket_close(sock);

    return NULL;
}

int main(int argc, char *argv[]) {
    int sock;

    printf("Limit30 Server\n");

    if (argc != 2) {
        printf("Usage : %s port\n", argv[0]);
        return 0;
    }

    auth_init();
    auth_dump();

    game_init();

    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("signal()");
        return -1;
    }

    if ((sock = socket_listen(argv[1])) < 0)
        return -1;

    printf("Server is running.\n");

    while (running) {
        pthread_t thread;
        int *client_sock;

        client_sock = calloc(1, sizeof(int));

        if ((*client_sock = socket_accept(sock)) < 0) {
            free(client_sock);
            continue;
        }

        printf("[%d] accept\n", *client_sock);

        pthread_create(&thread, NULL, main_loop, client_sock);
    }

    printf("Server is shutting down.\n");

    socket_close(sock);

    auth_free();

    game_free();

    return 0;
}