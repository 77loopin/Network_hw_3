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
UserList* userListHeader;
UserList userList;
setupInfo setup;
SOCKET serverSocket;
FD_SET fdSet;
HANDLE hMainRecv;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int i;
	int retval;
	SOCKADDR_IN serverInfo;
	// Store instance
	hInst = hInstance;

	// init critical section
	InitializeCriticalSection(&cs);
	// WINSOCK init
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		err_quit("WSAStartup()");
	}

	memset(&userList, 0, sizeof(userList));
	memset(&serverInfo, 0, sizeof(serverInfo));
	userListHeader = &userList;
	
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_port = htons(9999);
	serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		err_quit("socket()");
	}

	retval = bind(serverSocket, (SOCKADDR*)&serverInfo, sizeof(serverInfo));
	if (retval == SOCKET_ERROR) {
		err_quit("bind()");
	}

	retval = listen(serverSocket, 5);
	if (retval == -1) {
		err_quit("listen()");
	}

	hMainRecv = NULL;
	while ( hMainRecv == NULL) 
		hMainRecv =(HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);

	retval = WaitForSingleObject(hMainRecv, INFINITE);

	// run accept thread
	//_beginthreadex(NULL, 0, AcceptReceiver, NULL, 0, NULL);

	// delete critical section
	DeleteCriticalSection(&cs);
	// WINSOCK 
	WSACleanup();
	return 0;
}

