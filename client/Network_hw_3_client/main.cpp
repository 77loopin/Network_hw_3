// 2016년 1학기 네트워크프로그램 숙제 3번
// 성명 : 백승재
// 학번 : 101969

#pragma comment(lib, "ws2_32")
#include "main.h"

/*
CRITICAL_SECTION cs; // critical section에 접근 하기 위한 CS 변수 선언
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

