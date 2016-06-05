#pragma once
#include "resource.h"
#include "std_lib.h"
#include "std_error.h"
#include "data_type.h"
#include "dlg_func.h"
#include "sock_func.h"


extern HINSTANCE hInst; // Main instance
extern CRITICAL_SECTION cs; // Critical Section ���� ����
extern WSADATA wsa_data; // WINSOCK ����
extern SOCKET serverSocket; // ������ ����� ���� ����

extern UserList* userListHeader; // ������ ����Ʈ�� header
extern UserList userList; // ������ ����Ʈ�� ù��° ��� (header����)
extern setupInfo setup; // ���� ���� ���¸� �����ϴ� setupInfo ����ü ����
extern senderArgument sendArgument; // send���� �Ѱ��� arguemtn�� �����ϴ� ����ü ����

extern HANDLE hMainRecv; // Receiver Thread �ڵ�
extern HANDLE hSender; // Sender Thread �ڵ�
extern HANDLE hSendEvent; // �޽��� ������ �غ�Ǿ��ٴ� event
extern HANDLE hSendOverEvent; // �޽��� ������ �����ٴ� event
extern HANDLE hMainThread; // Main Thread �ڵ�


extern HWND hEdit1; // ä�� �޽����� ����ϴ� Edit Control �ڵ�
extern HWND hEdit2; // ä�� �޽����� �Է��ϴ� Edit Control �ڵ�
extern HWND hList; // ������ ����ڸ� ǥ���ϴ� List Box �ڵ�

extern HWND hMenu; // �޴� �ڵ�

extern HWND hIp; // IP�� �Է��ϴ� �ڵ�
extern HWND hPort; // PORT�� �Է��ϴ� �ڵ�
extern HWND hNick; // �г����� �Է��ϴ� Edit control �ڵ�