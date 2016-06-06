// 2016년 1학기 네트워크프로그램 숙제 3번
// Server
// 성명 : 백승재
// 학번 : 101969

#pragma comment(lib, "ws2_32")
#include "main.h"


HINSTANCE hInst; // Main Instance

CRITICAL_SECTION cs; // Critical Section 접근 변수
FD_SET fdSet; // recv 동기화를 위한 FD_SET 변수
WSADATA wsa_data; // WINSOCK 변수


UserList* userListHeader; // 접속자 리스트의 header
UserList userList; // 접속자 리스트의 첫번째 노드 (header역할)

senderArgument sendArgument; // send에게 넘겨줄 arguemtn를 저장하는 구조체 변수
SOCKET serverSocket; // 서버의 socket 정보를 담는 전역 변수

// HANDLE 목록
HANDLE hMainRecv; // Receiver 핸들
HANDLE hSender; // Sender 핸들
HANDLE hSendEvent; // 메시지 전송이 준비되었다는 event
HANDLE hSendOverEvent; // 메시지 전송이 끝났다는 event


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	int retval;
	// 인스턴스 정보 저장
	hInst = hInstance;
	
	// init critical section
	InitializeCriticalSection(&cs);
	// WINSOCK init
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		err_quit("WSAStartup()");
	}

	// Send를 위한 2개의 이벤트 생성
	hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hSendEvent == NULL) err_quit("CreateEvent()");

	hSendOverEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hSendOverEvent == NULL) err_quit("CreateEvent()");

	// UserList 구조체 초기화
	memset(&userList, 0, sizeof(userList));
	userListHeader = &userList;

	// WINDOW 생성
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

	// Receiver Thread 생성
	hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "스레드 생성 실패.\n\n프로그램 종료.\n\n", "오류", MB_ICONERROR);
	}
	// Sender Thread 생성
	hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, NULL, 0, NULL);
	if (hSender == NULL) {
		MessageBox(NULL, "스레드 생성 실패.\n\n프로그램 종료.\n\n", "오류", MB_ICONERROR);
	}

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Critical Section 제거
	DeleteCriticalSection(&cs);

	// Thread 제거
	CloseHandle(hSender);
	CloseHandle(hMainRecv);

	// Event Handle 제거
	CloseHandle(hSendEvent);
	CloseHandle(hSendOverEvent);
	
	// WINSOCK 제거
	WSACleanup();
	
	return msg.wParam;
}


// 생성된 window에서 실행될 callback 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	SOCKADDR_IN serverInfo;
	int retval;
	static HWND hEdit;

	switch (uMsg) {
	case WM_CREATE : // 초기화
		hEdit = CreateWindow("edit", NULL,
			WS_CHILD | WS_VISIBLE | WS_HSCROLL |
			WS_VSCROLL | ES_AUTOHSCROLL |
			ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
			0, 0, 0, 0, hWnd, (HMENU)100, hInst, NULL);
		return 0;
	case WM_SIZE :
		MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;
	case WM_SETFOCUS:
		SetFocus(hEdit);
		return 0;
	case WM_DESTROY: // 종료시
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}