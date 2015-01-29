#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "message.h"
#include "linkedList.h"
#include "packet.h"
#include "game.h"

typedef struct game_t {
    pthread_mutex_t mutex;
    int socks[2];
    UserData *users[2];
    uint64_t reward;
    int total;
    int turn; // Player A = TURN_PLAYER, Player B = TURN_ENEMY
    int status;
} Game;

enum {
    STAT_WAITING,
    STAT_INGAME,
    STAT_END
};

LinkedList *games;

pthread_mutex_t mutex_games;

void game_init() {
    games = linkedList_init();
    pthread_mutex_init(&mutex_games, NULL);
}

void game_broadcast_join(Game *g) {
    Message_Join m;

    memset(&m, 0, sizeof(m));

    m.type = MES_JOIN;
    m.bet = (uint64_t) g->reward;

    /* Player A */
    m.turn = TURN_PLAYER;
    packet_send(g->socks[0], &m, sizeof(m));

    /* Player B */
    m.turn = TURN_ENEMY;
    packet_send(g->socks[1], &m, sizeof(m));
}

void game_broadcast_say(Game *g, uint64_t say) {
    Message_Say m;

    memset(&m, 0, sizeof(m));

    m.type = MES_SAY;
    m.total = (uint8_t) g->total;
    m.say = (uint8_t) say;

    /* Player A */
    m.turn = g->turn == TURN_PLAYER ? TURN_PLAYER : TURN_ENEMY;
    packet_send(g->socks[0], &m, sizeof(m));

    /* Player B */
    m.turn = g->turn == TURN_PLAYER ? TURN_ENEMY : TURN_PLAYER;
    packet_send(g->socks[1], &m, sizeof(m));
}

void game_broadcast_result(Game **game) {
    Message_Result m;
    Game *g = *game;

    if (g->status != STAT_INGAME)
        return;

    g->status = STAT_END;

    memset(&m, 0, sizeof(m));

    m.type = MES_RESULT;

    /* Player A */
    if (g->turn == TURN_PLAYER && g->total >= 30) {
        g->users[0]->chip += g->reward;

        m.status = STATUS_OK;
    } else {
        m.status = STATUS_NG;
    }

    m.chip = g->users[0]->chip;

    packet_send(g->socks[0], &m, sizeof(m));

    /* Player B */
    if (g->turn == TURN_ENEMY && g->total >= 30) {
        g->users[1]->chip += g->reward;

        m.status = STATUS_OK;
    } else {
        m.status = STATUS_NG;
    }

    m.chip = g->users[1]->chip;

    packet_send(g->socks[1], &m, sizeof(m));
}

void game_say(int sock, Game **game, uint64_t say) {
    Message_Say m;
    Game *g = *game;

    if (g == NULL)
        return;

    pthread_mutex_lock(&(*game)->mutex);

    memset(&m, 0, sizeof(m));

    switch (g->turn) {
        case TURN_PLAYER:
            if (g->socks[0] != sock) return;
            break;

        case TURN_ENEMY:
            if (g->socks[1] != sock) return;
            break;

        default:
            return;
    }

    g->total += say;

    game_broadcast_say(g, say);

    g->turn = g->turn == TURN_PLAYER ? TURN_ENEMY : TURN_PLAYER;

    if (g->total >= 30) {
        game_broadcast_result(game);
        free(g);
        *game = NULL;

        return;
    }

    pthread_mutex_unlock(&g->mutex);
}

Game *game_join(int sock, UserData *u, uint64_t bet) {
    Game *g = NULL;

    if (u == NULL || bet <= 1)
        return g;

    pthread_mutex_lock(&mutex_games);

    if (linkedList_pop(games, &g, sizeof(g)) < 0) {
        g = calloc(1, sizeof(Game));
        memset(g, 0, sizeof(Game));

        pthread_mutex_init(&g->mutex, NULL);
        g->socks[0] = sock;
        g->users[0] = u;
        g->reward = bet;
        g->total = 0;
        g->turn = TURN_PLAYER;
        g->status = STAT_WAITING;

        u->chip -= bet;

        linkedList_push(games, &g, sizeof(g));
    } else {
        g->socks[1] = sock;
        g->users[1] = u;
        g->reward += bet;
        g->status = STAT_INGAME;

        u->chip -= bet;

        game_broadcast_join(g);
    }

    pthread_mutex_unlock(&mutex_games);

    return g;
}

void game_surrender(int sock, Game **game) {
    Game *g = *game, dummy;

    if (g == NULL || g->status == STAT_END)
        return;

    pthread_mutex_lock(&mutex_games);

    if (g->status == STAT_WAITING) {
        g->users[0]->chip += g->reward;
        linkedList_pop(games, &dummy, sizeof(dummy));

    } else {
        g->total = 30;
        g->turn = sock == g->socks[0] ? TURN_ENEMY : TURN_PLAYER;

        game_broadcast_result(game);
    }

    free(g);
    *game = NULL;

    pthread_mutex_unlock(&mutex_games);
}

void game_free() {
    linkedList_free(games);
    pthread_mutex_destroy(&mutex_games);
}