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
UserList* userListHeader;
UserList userList;
setupInfo setup;
SOCKET serverSocket;
FD_SET fdSet;


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

	SOCKADDR_IN clientInfo;
	int clientInfoSize = sizeof(clientInfo);
	SOCKET client = accept(serverSocket, (SOCKADDR*)&clientInfo, &clientInfoSize);
	while (1) {
		printf("running\n");
		Sleep(3);
	}
	
	// run accept thread
	//_beginthreadex(NULL, 0, AcceptReceiver, NULL, 0, NULL);

	// delete critical section
	DeleteCriticalSection(&cs);
	// WINSOCK 
	WSACleanup();
	return 0;
}

