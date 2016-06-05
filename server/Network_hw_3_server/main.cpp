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
HANDLE hSender;
HANDLE hSendEvent;
HANDLE hSendOverEvent;
senderArgument sendArgument;

HWND hEdit;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int retval;

	hInst = hInstance;
	
	// init critical section
	InitializeCriticalSection(&cs);
	// WINSOCK init
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		err_quit("WSAStartup()");
	}

	hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hSendEvent == NULL) err_quit("CreateEvent()");

	hSendOverEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hSendOverEvent == NULL) err_quit("CreateEvent()");

	memset(&userList, 0, sizeof(userList));
	userListHeader = &userList;


	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "MyWndClass";
	if (!RegisterClass(&wndclass)) return 1;

	HWND hWnd = CreateWindow("MyWndClass", "Server", WS_OVERLAPPEDWINDOW, 0, 0, 600, 200, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return 1;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "스레드 생성 실패.\n\n프로그램 종료.\n\n", "오류", MB_ICONERROR);
	}
	hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, NULL, 0, NULL);
	if (hSender == NULL) {
		MessageBox(NULL, "스레드 생성 실패.\n\n프로그램 종료.\n\n", "오류", MB_ICONERROR);
	}

	//retval = WaitForSingleObject(hMainRecv, INFINITE);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeleteCriticalSection(&cs);
	CloseHandle(hSender);
	CloseHandle(hMainRecv);
	WSACleanup();
	
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	SOCKADDR_IN serverInfo;
	int retval;

	switch (uMsg) {
	case WM_CREATE :
		hEdit = CreateWindow("edit", NULL,
			WS_CHILD | WS_VISIBLE | WS_HSCROLL |
			WS_VSCROLL | ES_AUTOHSCROLL |
			ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
			0, 0, 0, 0, hWnd, (HMENU)100, hInst, NULL);
		/*
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
		*/

		//retval = WaitForSingleObject(hMainRecv, INFINITE);
		return 0;
	case WM_SIZE :
		MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;
	case WM_SETFOCUS:
		SetFocus(hEdit);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/*
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

	hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hSendEvent == NULL) err_quit("CreateEvent()");

	hSendOverEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hSendOverEvent == NULL) err_quit("CreateEvent()");

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

	hMainRecv =(HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "스레드 생성 실패.\n\n프로그램 종료.\n\n", "오류", MB_ICONERROR);
	}
	hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, NULL, 0, NULL);
	if (hSender == NULL) {
		MessageBox(NULL, "스레드 생성 실패.\n\n프로그램 종료.\n\n", "오류", MB_ICONERROR);
	}

	retval = WaitForSingleObject(hMainRecv, INFINITE);

	// run accept thread
	//_beginthreadex(NULL, 0, AcceptReceiver, NULL, 0, NULL);

	// delete critical section
	DeleteCriticalSection(&cs);
	// WINSOCK 
	WSACleanup();
	return 0;
}

*/