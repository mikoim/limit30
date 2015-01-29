#ifndef HEADER_AUTH_H
#define HEADER_AUTH_H

#include <stdint.h>

typedef struct {
    char username[11];
    char password[33];
    uint64_t chip;
} UserData;

int auth_init();

int auth_free();

int auth_login(const char *username, const char *password, UserData **userData);

void auth_dump();

#endif