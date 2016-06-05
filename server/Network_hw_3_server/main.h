#pragma once
#include "std_lib.h"
#include "std_error.h"
#include "data_type.h"
#include "dlg_func.h"
#include "sock_func.h"

// window ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


extern HINSTANCE hInst; // Main Instance

extern CRITICAL_SECTION cs; // Critical Section ���� ����
extern FD_SET fdSet; // recv ����ȭ�� ���� FD_SET ����
extern WSADATA wsa_data; // WINSOCK ����


extern UserList* userListHeader; // ������ ����Ʈ�� header
extern UserList userList; // ������ ����Ʈ�� ù��° ��� (header����)

extern senderArgument sendArgument; // send���� �Ѱ��� arguemtn�� �����ϴ� ����ü ����
extern SOCKET serverSocket; // ������ socket ������ ��� ���� ����

// HANDLE ���
extern HANDLE hMainRecv; // Receiver Thread �ڵ�
extern HANDLE hSender; // Sender Thread �ڵ�
extern HANDLE hSendEvent; // �޽��� ������ �غ�Ǿ��ٴ� event
extern HANDLE hSendOverEvent; // �޽��� ������ �����ٴ� event

