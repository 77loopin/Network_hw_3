#pragma once
#include "std_lib.h"

SOCKET getConnection(char* ip, int port);
int sendToServer(SOCKET sock, int flag, int size, char* Data);