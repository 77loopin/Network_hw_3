#pragma once
#include "main.h"

UINT WINAPI MainThread(LPVOID arg);
UINT WINAPI MainReceiver(LPVOID arg);
UINT WINAPI MsgSender(LPVOID arg);
SOCKET getConnection(char* ip, int port);
int sendToServer(SOCKET sock, int flag, int size, char* Data);
BOOL addUser(char*);
int delUser(char*);
int delAllUser();
UserList* searchNick(char*);
struct tm* gettime();