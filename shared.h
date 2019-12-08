#ifndef __SHARED_H
#define __SHARED_H

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_ID 1

#define ROUNDS 12
#define MAX_GAMES 100
#define MAX_PLAYERS 100

#define NICKNAME_MAX_LENGTH 20
#define STRING_MAX_SIZE 256
#define PROJ_ID 100

#define BACK 1

typedef enum Type
{
    addGame,
    playGame,
    getGame,
    listGames,
    closeServer
} Type;

typedef struct Request
{
    long mtype;
    Type type;
    int pid;
    int gameid;
    signed char data[4];
    char author[NICKNAME_MAX_LENGTH];
    long hash;
} Request;

typedef struct Response
{
    long mtype;
    int gameid;
    signed char attempts;
    signed char state;
    signed char data[4];
} Response;

typedef struct Player
{
    long hash;
    signed char game_attempts;
    signed char state;
} Player;

typedef struct Game
{
    signed char data[4];
    char author[NICKNAME_MAX_LENGTH];
    Player players[MAX_PLAYERS];
} Game;

typedef struct GameInfo
{
    long mtype;
    signed char gameid;
    signed char attempts;
    signed char state;
    char author[NICKNAME_MAX_LENGTH];
} GameInfo;

#endif