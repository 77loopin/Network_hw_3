#pragma once
#include "std_lib.h"
#include "std_error.h"
#include "data_type.h"
#include "dlg_func.h"
#include "sock_func.h"

// window »ý¼º
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern HINSTANCE hInst;
extern CRITICAL_SECTION cs;
extern WSADATA wsa_data;
extern FD_SET fdSet;

extern UserList* userListHeader;
extern UserList userList;
extern senderArgument sendArgument;
extern SOCKET serverSocket;

extern HANDLE hMainRecv;
extern HANDLE hSender;
extern HANDLE hSendEvent;
extern HANDLE hSendOverEvent;

