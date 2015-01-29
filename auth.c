#include <stdio.h>
#include <string.h>
#include "auth.h"
#include "linkedList.h"
#include "config.h"

LinkedList *auth_users;

int auth_init();

int auth_free();

int auth_register(const char *username, const char *password);

int auth_request(const char *username, const char *password, UserData **userData);

int auth_login(const char *username, const char *password, UserData **userData);

void auth_dump();

int auth_init() {
    FILE *fd;
    char buf[MAX_BUF];

    auth_users = linkedList_init();

    if ((fd = fopen("auth.dat", "rb")) == NULL) return -1;

    while (fread(buf, sizeof(UserData), 1, fd))
        linkedList_push(auth_users, buf, sizeof(UserData));

    fclose(fd);

    return 0;
}

int auth_free() {
    FILE *fd;
    UserData *tmp;
    int i;

    if (auth_users == NULL) {
        printf("auth_free(): database has not been initialized.\n");
        return -1;
    }

    if ((fd = fopen("auth.dat", "wb+")) == NULL) return -1;

    for (i = 0; (tmp = linkedList_getIndexOf(auth_users, i)) != NULL; i++)
        fwrite(tmp, sizeof(UserData), 1, fd);

    fclose(fd);

    linkedList_free(auth_users);

    auth_users = NULL;

    return 0;
}

int auth_register(const char *username, const char *password) {
    UserData tmp, *res = NULL;
    size_t len_user, len_pass;

    if (auth_users == NULL) {
        printf("auth_register(): database has not been initialized.\n");
        return -1;
    }

    len_user = strlen(username);
    len_pass = strlen(password);

    if (len_user > 10 || len_pass > 32 || len_user == 0 || len_pass == 0) { // check string length
        printf("auth_register(): length error.\n");
        return -1;
    }

    if (auth_request(username, password, &res) != -2) { // check if username already used
        printf("auth_register(): username has been already used.\n");
        return -1;
    }

    strcpy(tmp.username, username);
    strcpy(tmp.password, password);
    tmp.chip = 100;

    linkedList_push(auth_users, &tmp, sizeof(UserData));

    return 0;
}

int auth_request(const char *username, const char *password, UserData **userData) {
    UserData *user;
    size_t len_user, len_pass;
    int i;

    if (auth_users == NULL) {
        printf("auth_request(): database has not been initialized.\n");
        return -1;
    }

    len_user = strlen(username);
    len_pass = strlen(password);

    if (len_user > 10 || len_pass > 32 || len_user == 0 || len_pass == 0) { // check string length
        printf("auth_request(): length error.\n");
        return -1;
    }

    for (i = 0; (user = linkedList_getIndexOf(auth_users, i)) != NULL; i++) {
        if (strcmp(user->username, username) == 0) {
            if (strcmp(user->password, password) == 0) {
                *userData = user;
                return 0;
            } else {
                printf("auth_request(): password is wrong\n");
                return -3;
            }
        }
    }

    printf("auth_request(): username not found\n");
    return -2;
}

int auth_login(const char *username, const char *password, UserData **userData) {
    switch (auth_request(username, password, userData)) {
        case 0:
            return 0;

        case -2:
            if (auth_register(username, password) == 0 && auth_request(username, password, userData) == 0)
                return 0;

        default:
            break;
    }

    return -1;
}

void auth_dump() {
    UserData *user;
    int i;

    if (auth_users == NULL) {
        printf("auth_dump(): database has not been initialized.\n");
        return;
    }

    for (i = 0; (user = linkedList_getIndexOf(auth_users, i)) != NULL; i++) {
        printf("[%d] %s:%s = $%lu\n", i, user->username, user->password, user->chip);
    }
}