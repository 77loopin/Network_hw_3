#pragma once
#include "std_lib.h"
#include "std_error.h"
#include "data_type.h"
#include "dlg_func.h"
#include "sock_func.h"


extern HINSTANCE hInst;
extern CRITICAL_SECTION cs;
extern WSADATA wsa_data;

extern UserList userList;
extern setupInfo setup;
extern senderArgument sendArgument;
extern SOCKET serverSocket;
extern UserList* userListHeader;
extern FD_SET fdSet;

extern HANDLE hMainRecv;
extern HANDLE hSender;
extern HANDLE hSendEvent;
extern HANDLE hSendOverEvent;
