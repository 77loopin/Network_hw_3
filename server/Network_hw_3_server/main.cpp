// 2016�� 1�б� ��Ʈ��ũ���α׷� ���� 3��
// Server
// ���� : �����
// �й� : 101969

#pragma comment(lib, "ws2_32")
#include "main.h"


HINSTANCE hInst; // Main Instance

CRITICAL_SECTION cs; // Critical Section ���� ����
FD_SET fdSet; // recv ����ȭ�� ���� FD_SET ����
WSADATA wsa_data; // WINSOCK ����


UserList* userListHeader; // ������ ����Ʈ�� header
UserList userList; // ������ ����Ʈ�� ù��° ��� (header����)

senderArgument sendArgument; // send���� �Ѱ��� arguemtn�� �����ϴ� ����ü ����
SOCKET serverSocket; // ������ socket ������ ��� ���� ����

// HANDLE ���
HANDLE hMainRecv; // Receiver �ڵ�
HANDLE hSender; // Sender �ڵ�
HANDLE hSendEvent; // �޽��� ������ �غ�Ǿ��ٴ� event
HANDLE hSendOverEvent; // �޽��� ������ �����ٴ� event


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	int retval;
	// �ν��Ͻ� ���� ����
	hInst = hInstance;
	
	// init critical section
	InitializeCriticalSection(&cs);
	// WINSOCK init
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		err_quit("WSAStartup()");
	}

	// Send�� ���� 2���� �̺�Ʈ ����
	hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hSendEvent == NULL) err_quit("CreateEvent()");

	hSendOverEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hSendOverEvent == NULL) err_quit("CreateEvent()");

	// UserList ����ü �ʱ�ȭ
	memset(&userList, 0, sizeof(userList));
	userListHeader = &userList;

	// WINDOW ����
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

	// Receiver Thread ����
	hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "������ ���� ����.\n\n���α׷� ����.\n\n", "����", MB_ICONERROR);
	}
	// Sender Thread ����
	hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, NULL, 0, NULL);
	if (hSender == NULL) {
		MessageBox(NULL, "������ ���� ����.\n\n���α׷� ����.\n\n", "����", MB_ICONERROR);
	}

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Critical Section ����
	DeleteCriticalSection(&cs);

	// Thread ����
	CloseHandle(hSender);
	CloseHandle(hMainRecv);

	// Event Handle ����
	CloseHandle(hSendEvent);
	CloseHandle(hSendOverEvent);
	
	// WINSOCK ����
	WSACleanup();
	
	return msg.wParam;
}


// ������ window���� ����� callback �Լ�
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	SOCKADDR_IN serverInfo;
	int retval;
	static HWND hEdit;

	switch (uMsg) {
	case WM_CREATE : // �ʱ�ȭ
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
	case WM_DESTROY: // �����
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}