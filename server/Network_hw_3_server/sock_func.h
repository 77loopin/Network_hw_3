#pragma once
#include "main.h"

UINT WINAPI MainReceiver(LPVOID arg);
UINT WINAPI MsgSender(LPVOID arg);
//UINT WINAPI AcceptReceiver(LPVOID arg);
SOCKET getConnection(char* ip, int port);
int sendToClient(SOCKET sock, int flag, int size, char* Data);
int sendToAllClient(SOCKET, int, int, char*);
BOOL addUser(SOCKET);
BOOL delUser(UserList*);
BOOL checkNick(char*);
struct tm* gettime();