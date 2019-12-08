#include "shared.h"

Game games[MAX_GAMES];
int msqid, iterator;

void Listen();
void AddGame(Request *request);
void PlayGame(Request *request);
void ListGames(int pid, long hash);
int AddPlayer(Game *game, long hash);
int CheckAnswer(signed char answer[4], signed char correct[4], signed char result[4]);
void SendGameInfo(int gameid, int pid, long hash);
int Find(signed char tab[4], signed char value);
signed char GetAttempts(Game *game, long hash);
signed char GetState(Game *game, long hash);
key_t GetKey(int argc, char **argv);
void CreateQueue(key_t key);
void CleanOnExit();
void PrintError(char *function, char *name);

int main(int argc, char **argv)
{
    signal(SIGTERM, CleanOnExit);
    signal(SIGINT, CleanOnExit);

    CreateQueue(GetKey(argc, argv));

    memset(games, 0, sizeof(games));

    Listen();

    return 0;
}

void Listen()
{
    Request request;

    while (1)
    {
        if (msgrcv(msqid, (struct Request *)&request, sizeof(request), SERVER_ID, 0) == -1)
            PrintError("msgrcv()", "Listen()");

        if (request.type == addGame)
            AddGame(&request);
        else if (request.type == playGame)
            PlayGame(&request);
        else if (request.type == getGame)
            SendGameInfo(request.gameid, request.pid, request.hash);
        else if (request.type == listGames)
            ListGames(request.pid, request.hash);
        else if (request.type == closeServer)
            CleanOnExit();
    }
}

void AddGame(Request *request)
{
    Response response;
    response.mtype = request->pid;
    response.gameid = iterator;

    if (iterator < MAX_GAMES)
    {
        memcpy(games[iterator].data, request->data, sizeof(request->data));
        memcpy(games[iterator].author, request->author, sizeof(request->author));

        iterator++;

        printf("New game id %d: %d %d %d %d from %d\n", iterator - 1, games[iterator - 1].data[0], games[iterator - 1].data[1], games[iterator - 1].data[2], games[iterator - 1].data[3], request->pid);
    }
    else
    {
        response.gameid = -1;
    }

    if (msgsnd(msqid, (Response *)&response, sizeof(response), 0) == -1)
        PrintError("msgsnd()", "AddGame()");
}

void PlayGame(Request *request)
{
    Response response;
    response.mtype = request->pid;

    Game *current = &games[request->gameid];

    if (current->data[0] == 0)
    {
        response.gameid = -1;
    }
    else
    {
        int player = -1;

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (current->players[i].hash == request->hash)
            {
                current->players[i].game_attempts++;
                response.attempts = current->players[i].game_attempts;
                player = i;
            }
        }

        if (player == -1)
        {
            if ((player = AddPlayer(current, request->hash)) != -1)
                response.attempts = 1;
            else
                response.attempts = -1;
        }

        response.gameid = request->gameid;
        int win = CheckAnswer(request->data, current->data, response.data);

        if (win)
            current->players[player].state = 1;
        else if (current->players[player].game_attempts >= ROUNDS)
            current->players[player].state = -1;

        response.state = current->players[player].state;
    }

    printf("%d played %d game, round %d results %d %d %d %d\n", request->pid, request->gameid, response.attempts, response.data[0], response.data[1], response.data[2], response.data[3]);

    if (msgsnd(msqid, (Response *)&response, sizeof(response), 0) == -1)
        PrintError("msgsnd()", "CheckGame()");
}

void ListGames(int pid, long hash)
{
    GameInfo info;
    info.mtype = pid;
    for (int i = 0; i < MAX_GAMES; i++)
    {
        if (games[i].data[0] != 0)
            SendGameInfo(i, pid, hash);
    }

    info.gameid = -1;
    if (msgsnd(msqid, (GameInfo *)&info, sizeof(info), 0) == -1)
        PrintError("msgsnd()", "SendList(-1)");
}

int AddPlayer(Game *game, long hash)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (game->players[i].hash == 0)
        {
            game->players[i].hash = hash;
            game->players[i].game_attempts = 1;
            return i;
        }
    }

    return -1;
}

int CheckAnswer(signed char answer[4], signed char correct[4], signed char result[4])
{
    int good = 0;

    for (int i = 0; i < 4; i++)
    {
        if (answer[i] == correct[i])
            result[i] = 2;
        else if (Find(correct, answer[i]) != -1)
            result[i] = 1;
        else
            result[i] = 0;

        if (result[i] == 2)
            good++;
    }

    return (good == 4);
}

void SendGameInfo(int gameid, int pid, long hash)
{
    GameInfo info;
    info.mtype = pid;

    if (gameid >= MAX_GAMES || games[gameid].data[0] == 0)
    {
        info.gameid = -1;
    }
    else
    {
        info.gameid = gameid;
        info.attempts = GetAttempts(&games[gameid], hash);
        info.state = GetState(&games[gameid], hash);
        strcpy(info.author, games[gameid].author);
    }

    if (msgsnd(msqid, (GameInfo *)&info, sizeof(info), 0) == -1)
        PrintError("msgsnd()", "SendGameInfo()");
}

int Find(signed char tab[4], signed char value)
{
    for (int i = 0; i < 4; i++)
        if (tab[i] == value)
            return i;

    return -1;
}

signed char GetAttempts(Game *game, long hash)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (game->players[i].hash == hash)
            return game->players[i].game_attempts;

    return 0;
}

signed char GetState(Game *game, long hash)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (game->players[i].hash == hash)
            return game->players[i].state;

    return 0;
}

key_t GetKey(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("server: missing argument KEY\n");
        exit(0);
    }

    int t = atoi(argv[1]);

    if (t <= 0)
    {
        printf("server: wrong argument KEY\n");
        exit(0);
    }

    return (key_t)t;
}

void CreateQueue(key_t key)
{
    if ((msqid = msgget(key, 0664 | IPC_CREAT | IPC_EXCL)) == -1)
        PrintError("msgget()", "CreateQueue()");
}

void CleanOnExit()
{
    if (msqid > 0)
        if (msgctl(msqid, IPC_RMID, NULL) == -1)
            PrintError("msgctl()", "CleanOnExit()");

    exit(0);
}

void PrintError(char *function, char *name)
{
    char buffer[256];
    snprintf(buffer, 256, "server: %s error in %s", function, name);
    perror(buffer);
    exit(0);
}