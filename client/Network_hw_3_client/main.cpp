// 2016�� 1�б� ��Ʈ��ũ���α׷� ���� 3��
// ���� : �����
// �й� : 101969

#pragma comment(lib, "ws2_32")
#include "main.h"

/*
CRITICAL_SECTION cs; // critical section�� ���� �ϱ� ���� CS ���� ����
HINSTANCE hInst;
WSADATA wsa_data;
*/

HINSTANCE hInst;
CRITICAL_SECTION cs;
WSADATA wsa_data;
SOCKET serverSocket;
UserList userList;
UserList* userListHeader = NULL;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int i;
	// Store instance
	hInst = hInstance;

	// init critical section
	InitializeCriticalSection(&cs);
	// WINSOCK init
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		err_quit("WSAStartup()");
	}

	memset(&userList, 0, sizeof(userList));


	// run DialogBox
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)main_DlgProc);

	// delete critical section
	DeleteCriticalSection(&cs);
	// WINSOCK 
	WSACleanup();
	return 0;
}

