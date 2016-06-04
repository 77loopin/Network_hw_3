#pragma once
#include "main.h"

UINT WINAPI MainReceiver(LPVOID arg);
UINT WINAPI MsgSender(LPVOID arg);
int sendToClient(SOCKET sock, int flag, int size, char* Data);
int sendToAllClient(SOCKET, int, int, char*);
BOOL addUser(SOCKET);
BOOL delUser(UserList*);
UserList* searchNick(char*);
struct tm* gettime();