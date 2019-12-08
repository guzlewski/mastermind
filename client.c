#include "shared.h"
#include "colors.h"

char ipckey[STRING_MAX_SIZE];
char nickname[NICKNAME_MAX_LENGTH];
long hash;
int msqid, pid;

void AddGame();
void PlayGame();
void ListGames();
void GetInfo(GameInfo *info, int id);
void KillServer();
unsigned long Hash(unsigned char *str);
int InputData(signed char data[4], char *info);
int InputInt(char *message);
key_t GetKey(int argc, char **argv);
void OpenQueue(key_t key, char *k);
void ClearConsole();
void Menu();
int Choose();
void PrintError(char *function);
void StringCopy(char *dest, int destsize, char *source, int sourcesize);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf(WHITE_TEXT("Missing parameters.\n"));
        printf(WHITE_TEXT("Usage:\n"));
        printf(WHITE_TEXT("\t%s KEY NICK_NAME\n"), argv[0]);
        exit(0);
    }

    StringCopy(ipckey, STRING_MAX_SIZE, argv[1], strlen(argv[1]));
    StringCopy(nickname, NICKNAME_MAX_LENGTH, argv[2], strlen(argv[2]));

    OpenQueue(GetKey(argc, argv), argv[1]);
    pid = (int)getpid();
    hash = Hash((unsigned char *)nickname);

    while (1)
    {
        int choose = Choose();

        switch (choose)
        {
        case 1:
            AddGame();
            break;
        case 2:
            PlayGame();
            break;
        case 3:
            ListGames();
            break;
        case 4:
            exit(0);
            break;
        case 5:
            KillServer();
            break;
        }
    }

    return 0;
}

void AddGame()
{
    Request request;
    request.mtype = SERVER_ID;
    request.pid = pid;
    request.type = addGame;

    if (!InputData(request.data, YELLOW_TEXT("Input Pattern") WHITE_TEXT(" (format 'c1 c2 c3 c4')") YELLOW_TEXT(": ")))
        return;

    StringCopy(request.author, NICKNAME_MAX_LENGTH, nickname, strlen(nickname));

    if (msgsnd(msqid, (Request *)&request, sizeof(request), 0) == -1)
        PrintError("msgsnd()");

    Response response;
    if (msgrcv(msqid, (struct Response *)&response, sizeof(response), pid, 0) == -1)
        PrintError("msgrcv()");

    if (response.gameid != -1)
        printf(WHITE_TEXT("Game submited with id = ") GREEN_TEXT("%d\n"), response.gameid + 1);
    else
        printf(RED_TEXT("Can't add new game") WHITE_TEXT(" - games table is full = ") YELLOW_TEXT("%d") WHITE_TEXT(".\n"), MAX_GAMES);

    getchar();
    getchar();
}

void PlayGame()
{
    int id, round;

    while (1)
    {
        if (!(id = InputInt(WHITE_TEXT("Enter game id: "))))
            return;

        GameInfo info;
        GetInfo(&info, id);

        if (info.gameid == -1)
        {
            printf(WHITE_TEXT("Game ") YELLOW_TEXT("%d") RED_TEXT(" not exists!\n"), id);
            getchar();
            getchar();
        }
        else if (info.state != 0)
        {
            if (info.state == 1)
                printf(WHITE_TEXT("You ") GREEN_TEXT("WON") WHITE_TEXT(" this game, choose another one.\n"));
            else
                printf(WHITE_TEXT("You ") RED_TEXT("LOST") WHITE_TEXT(" this game, choose another one.\n"));

            getchar();
            getchar();
        }
        else
        {
            round = info.attempts;
            break;
        }
    }

    Request request;
    request.mtype = SERVER_ID;
    request.pid = pid;
    request.type = playGame;
    request.gameid = id - 1;
    request.hash = hash;

    while (1)
    {
        round++;
        char buffer[STRING_MAX_SIZE];

        sprintf(buffer, WHITE_TEXT("Game ") YELLOW_TEXT("%d") WHITE_TEXT(" round ") YELLOW_TEXT("%d\n") YELLOW_TEXT("Input Pattern") WHITE_TEXT(" (format 'c1 c2 c3 c4')") YELLOW_TEXT(": "), id, round);

        if (!InputData(request.data, buffer))
            return;

        if (msgsnd(msqid, (Request *)&request, sizeof(request), 0) == -1)
            PrintError("msgsnd()");

        Response response;
        if (msgrcv(msqid, (struct Response *)&response, sizeof(response), pid, 0) == -1)
            PrintError("msgrcv()");

        if (response.state == 1)
            printf(WHITE_TEXT("You won game id ") GREEN_TEXT("%d") WHITE_TEXT(" with ") GREEN_TEXT("%d") WHITE_TEXT(" attempts!\n"), response.gameid + 1, response.attempts);
        else if (response.state == -1)
            printf(WHITE_TEXT("You lost game id ") RED_TEXT("%d") WHITE_TEXT(".\n"), response.gameid + 1);
        else if (response.attempts == -1)
            printf(RED_TEXT("Can't add new player") WHITE_TEXT(", players table is full = ") YELLOW_TEXT("%d") WHITE_TEXT(".\n"), MAX_PLAYERS);
        else
            printf(WHITE_TEXT("Game ") YELLOW_TEXT("%d") WHITE_TEXT(" round ") YELLOW_TEXT("%d") WHITE_TEXT(" results: ") YELLOW_TEXT("%d %d %d %d\n"), response.gameid + 1, response.attempts, response.data[0], response.data[1], response.data[2], response.data[3]);

        getchar();
        getchar();

        if (response.state != 0 || response.attempts == -1)
            break;
    }
}

void ListGames()
{
    ClearConsole();

    Request request;
    request.mtype = SERVER_ID;
    request.pid = pid;
    request.type = listGames;
    request.hash = hash;

    if (msgsnd(msqid, (Request *)&request, sizeof(request), 0) == -1)
        PrintError("msgsnd()");

    int count = 0;
    GameInfo info;

    while (1)
    {
        if (msgrcv(msqid, (GameInfo *)&info, sizeof(info), pid, 0) == -1)
            PrintError("msgrcv()");

        if (info.gameid == -1)
            break;

        count++;
        printf(WHITE_TEXT("Game ") YELLOW_TEXT("%d") WHITE_TEXT(" round ") YELLOW_TEXT("%d") WHITE_TEXT(" by ") YELLOW_TEXT("%s") WHITE_TEXT(" result: "), info.gameid + 1, info.attempts, info.author);
        if (info.state == 0)
            printf(YELLOW_TEXT("READY\n"));

        else if (info.state == 1)
            printf(GREEN_TEXT("WON\n"));

        else if (info.state == -1)
            printf(RED_TEXT("LOST\n"));
    }

    if (count != 0)
        printf("\n");

    printf(WHITE_TEXT("Total games: "));

    if (count != 0)
        printf(GREEN_TEXT("%d\n"), count);
    else
        printf(RED_TEXT("%d\n"), count);

    getchar();
    getchar();
}

void GetInfo(GameInfo *info, int id)
{
    Request request;
    request.mtype = SERVER_ID;
    request.pid = pid;
    request.type = getGame;
    request.gameid = id - 1;
    request.hash = hash;

    if (msgsnd(msqid, (Request *)&request, sizeof(request), 0) == -1)
        PrintError("msgsnd()");

    if (msgrcv(msqid, info, sizeof(*info), pid, 0) == -1)
        PrintError("msgrcv()");
}

void KillServer()
{
    Request request;
    request.mtype = SERVER_ID;
    request.type = closeServer;

    if (msgsnd(msqid, (Request *)&request, sizeof(request), 0) == -1)
        PrintError("msgsnd()");

    exit(0);
}

unsigned long Hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

int InputData(signed char data[4], char *info)
{
    char buffer0[STRING_MAX_SIZE], buffer1[STRING_MAX_SIZE], buffer2[STRING_MAX_SIZE], buffer3[STRING_MAX_SIZE];

    do
    {
        ClearConsole();
        printf("%s", info);

        if (scanf("%s", buffer0) == EOF)
            exit(0);

        if (strlen(buffer0) == 1 && buffer0[0] == BACK)
            return 0;

        if (scanf("%s %s %s", buffer1, buffer2, buffer3) == EOF)
            exit(0);

        data[0] = atoi(buffer0);
        data[1] = atoi(buffer1);
        data[2] = atoi(buffer2);
        data[3] = atoi(buffer3);

    } while (data[0] < 1 || data[0] > 6 || data[1] < 1 || data[1] > 6 || data[2] < 1 || data[2] > 6 || data[3] < 1 || data[3] > 6);

    return 1;
}

int InputInt(char *message)
{
    int value;
    char buffer[STRING_MAX_SIZE];

    do
    {
        ClearConsole();
        printf("%s", message);

        if (scanf("%s", buffer) == EOF)
            exit(0);

        if (strlen(buffer) == 1 && buffer[0] == BACK)
            return 0;

        value = atoi(buffer);
    } while (value < 1);

    return value;
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

void OpenQueue(key_t key, char *k)
{
    if ((msqid = msgget(key, 0664)) == -1)
        PrintError("msgget()");
}

void ClearConsole()
{
    printf("\033[H\033[J");
}

void Menu()
{
    ClearConsole();
    printf(YELLOW_TEXT("MENU\n\n"));
    printf(WHITE_TEXT("Logged as ") GREEN_TEXT("%s\n\n"), nickname);
    printf(YELLOW_TEXT("1. ") WHITE_TEXT("Submit new puzzle.\n"));
    printf(YELLOW_TEXT("2. ") WHITE_TEXT("Solve puzzle.\n"));
    printf(YELLOW_TEXT("3. ") WHITE_TEXT("List puzzles.\n"));
    printf(YELLOW_TEXT("4. ") WHITE_TEXT("Exit.\n"));
    printf(YELLOW_TEXT("5. ") WHITE_TEXT("Exit and close server.\n\n"));
    printf(WHITE_TEXT("Option: "));
}

int Choose()
{
    char x;
    while (1)
    {
        Menu();

        x = getchar();

        if (x == EOF)
            exit(0);

        if (x >= '1' && x <= '5')
            return x - '0';
    }
}

void PrintError(char *function)
{
    char buffer[256];
    snprintf(buffer, 256, WHITE_TEXT("client: ") RED_TEXT("%s") WHITE_TEXT(" error"), function);
    perror(buffer);

    exit(0);
}

void StringCopy(char *dest, int destsize, char *source, int sourcesize)
{
    int min;
    if (destsize > sourcesize)
        min = sourcesize;
    else
        min = destsize - 1;

    for (int i = 0; i < min; i++)
        dest[i] = source[i];

    dest[min] = '\0';
}
