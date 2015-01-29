#ifndef HEADER_GAME_H
#define HEADER_GAME_H

#include <unistd.h>
#include "auth.h"

typedef struct game_t Game;

void game_init();

Game *game_join(int sock, UserData *u, uint64_t bet);

void game_say(int sock, Game **game, uint64_t say);

void game_surrender(int sock, Game **game);

void game_free();

#endif