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
char userList[MAXUSER][MAXNICK + 1];


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// Store instance
	hInst = hInstance;

	// init critical section
	InitializeCriticalSection(&cs);
	// WINSOCK init
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		err_quit("WSAStartup()");
	}

	// run DialogBox
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, main_DlgProc);

	// delete critical section
	DeleteCriticalSection(&cs);
	// WINSOCK 
	WSACleanup();
	return 0;
}

