#pragma once
#include "main.h"

UINT WINAPI MainThread(LPVOID arg);
UINT WINAPI MainReceiver(LPVOID arg);
UINT WINAPI MsgSender(LPVOID arg);
SOCKET getConnection(char* ip, int port);
int sendToServer(SOCKET sock, int flag, int size, char* Data);
void addUser(char*);
int delUser(char*);
struct tm* gettime();