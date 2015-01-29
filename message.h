#ifndef HEADER_MESSAGE_H
#define HEADER_MESSAGE_H

#include <stdint.h>

enum {
    MES_QUIT, // Message
    MES_REQUEST_LOGIN, // Message
    MES_LOGIN, // Message_Login
    MES_PROFILE, // Message_Profile
    MES_JOIN, // Message_Join
    MES_SAY, // Message_Say
    MES_RESULT // Message_Result
};

enum {
    STATUS_NG,
    STATUS_OK
};

enum {
    TURN_PLAYER,
    TURN_ENEMY
};

#pragma pack(1)

typedef struct {
    uint8_t type;
    uint8_t status;
    char data[1]; // Dummy
} Message;

typedef struct {
    uint8_t type; // This field must be MES_LOGIN.
    uint8_t status; // Unused
    char username[11];
    char password[33];
} Message_Login;

typedef struct {
    uint8_t type; // This field must be MES_PROFILE.
    uint8_t status;
    char username[11];
    char password[33];
    uint64_t chip; // 0 - 18446744073709551616
} Message_Profile;

typedef struct {
    uint8_t type; // This field must be MES_JOIN.
    uint8_t status; // Unused
    uint8_t turn;
    uint64_t bet; // 0 - 18446744073709551616
} Message_Join;

typedef struct {
    uint8_t type; // This field must be MES_SAY.
    uint8_t status; // Unused
    uint8_t turn;
    uint8_t total;
    uint8_t say; // 1 - 3
} Message_Say;

typedef struct {
    uint8_t type; // This field must be MES_RESULT.
    uint8_t status;
    uint64_t chip; // 0 - 18446744073709551616
} Message_Result;

#pragma pack(0)

#endif