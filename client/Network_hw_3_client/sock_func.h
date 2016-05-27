#pragma once
#include "main.h"

SOCKET getConnection(char* ip, int port);
int sendToServer(SOCKET sock, int flag, int size, char* Data);