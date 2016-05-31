#pragma once
#include "main.h"

UINT WINAPI MainReceiver(LPVOID arg);
UINT WINAPI MsgSender(LPVOID arg);
SOCKET getConnection(char* ip, int port);
int sendToServer(SOCKET sock, int flag, int size, char* Data);
struct tm* gettime();