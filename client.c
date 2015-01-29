#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "socket.h"
#include "packet.h"
#include "message.h"
#include "config.h"
#include "auth.h"

void do_quit();

void do_login();

void do_join();

void do_say();

UserData userData;
int sock;

sig_atomic_t running = 1;

void sigintHandler(int signum) {
    running = 0;
}

uint64_t inputNumber(uint64_t min, uint64_t max) {
    char buf[MAX_BUF];
    uint64_t i;

    do {
        printf("%lu - %lu: ", min, max);
        fgets(buf, MAX_BUF, stdin);
        sscanf(buf, "%lu", &i);
    } while (i > max || i < min);

    return i;
}

char *inputStrings(char *buf, const char *name, int min, int max) {
    size_t length;

    while (1) {
        printf("%s: ", name);
        fgets(buf, MAX_BUF, stdin);

        length = strlen(buf);
        buf[length - 1] = '\0';
        length--;

        if (length > max) {
            printf("%s must be less than %d characters.\n", name, max);
            continue;
        }

        if (length < min) {
            printf("%s must be more than %d characters.\n", name, min);
            continue;
        }

        break;
    }

    return buf;
}

void do_quit() {
    Message message;

    memset(&message, 0, sizeof(message));
    message.type = MES_QUIT;

    packet_send(sock, &message, sizeof(message));

    running = 0;
}

void do_login() {
    char buf[MAX_BUF];
    Message_Login message_login;

    memset(&message_login, 0, sizeof(message_login));
    message_login.type = MES_LOGIN;

    printf("[Login]\n");
    strcpy(message_login.username, inputStrings(buf, "Username", 1, 10));
    strcpy(message_login.password, inputStrings(buf, "Password", 1, 32));

    if (packet_send(sock, &message_login, sizeof(message_login)) < 0) running = 0;
}

void do_join() {
    Message_Join message_join;

    if (userData.chip == 0) {
        printf("You don't have enough chips!\n");
        do_quit();

        return;
    }

    memset(&message_join, 0, sizeof(message_join));
    message_join.type = MES_JOIN;

    printf("[Join]\nHow much do you want to bet?\n");
    message_join.bet = (uint64_t) inputNumber(1, userData.chip);

    printf("Waiting for other player...\n");
    if (packet_send(sock, &message_join, sizeof(message_join)) < 0) running = 0;
}

void do_say() {
    Message_Say message_say;

    memset(&message_say, 0, sizeof(message_say));
    message_say.type = MES_SAY;

    printf("[Say]\n");
    message_say.say = (uint8_t) inputNumber(1, 3);

    if (packet_send(sock, &message_say, sizeof(message_say)) < 0) running = 0;
}

void handler_request_login() {
    do_login();
}

void handler_profile(Message_Profile *profile) {
    char buf[MAX_BUF];

    if (profile->status == STATUS_OK) {
        strcpy(userData.username, profile->username);
        strcpy(userData.password, profile->password);
        userData.chip = profile->chip;

        printf("You are logged in as %s.\nChips: $%lu\n", userData.username, userData.chip);

        printf("Do you want to try Limit 30?\n");
        while (1) {
            printf("y/n:");
            fgets(buf, MAX_BUF, stdin);

            if (buf[0] == 'y')
                do_join();
            else if (buf[0] == 'n')
                do_quit();
            else
                continue;

            break;
        }
    } else {
        printf("Username or password is incorrect.\n");
    }
};

void handler_join(Message_Join *join) {
    printf("Player found!\nReward: $%lu\n", join->bet);

    if (join->turn == TURN_PLAYER)
        do_say();
}

void handler_say(Message_Say *say) {
    if (say->turn == TURN_ENEMY) {
        printf("Enemy says %d. (%d)\n", say->say, say->total);

        if (say->total < 30)
            do_say();

    } else {
        printf("I say %d. (%d)\n", say->say, say->total);
    }
}

void handler_result(Message_Result *result) {
    char buf[MAX_BUF];
    Message message;

    if (result->status == STATUS_OK)
        printf("You won the game!\n");
    else
        printf("You lose the game!\n");

    userData.chip = result->chip;
    printf("Chips: $%lu\n", userData.chip);

    memset(&message, 0, sizeof(message));
    message.type = MES_RESULT;
    packet_send(sock, &message, sizeof(message));

    printf("Do you want to try again?\n");
    while (1) {
        printf("y/n:");
        fgets(buf, MAX_BUF, stdin);

        if (buf[0] == 'y')
            do_join();
        else if (buf[0] == 'n')
            do_quit();
        else
            continue;

        break;
    }
}

void main_loop() {
    char buf[MAX_BUF];

    while (running) {
        if (packet_receive(sock, buf) < 0) {
            printf("Disconnected from server.\n");
            return;
        }

        switch (((Message *) &buf)->type) {
            case MES_QUIT:
                running = 0;
                break;

            case MES_REQUEST_LOGIN:
                handler_request_login();
                break;

            case MES_PROFILE:
                handler_profile((Message_Profile *) buf);
                break;

            case MES_JOIN:
                handler_join((Message_Join *) buf);
                break;

            case MES_SAY:
                handler_say((Message_Say *) buf);
                break;

            case MES_RESULT:
                handler_result((Message_Result *) buf);
                break;

            case MES_LOGIN:
            default:
                printf("main_loop(): Unknown packet recieved. [%d]\n", ((Message *) &buf)->type);
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    printf("Limit 30\n");

    if (argc != 3) {
        printf("Usage : %s host port\n", argv[0]);
        return 0;
    }

    if ((sock = socket_connect(argv[1], argv[2])) < 0)
        return -1;

    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("signal()");
        return -1;
    }

    main_loop();

    socket_close(sock);

    return 0;
}