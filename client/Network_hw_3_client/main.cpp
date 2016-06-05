// 2016�� 1�б� ��Ʈ��ũ���α׷� ���� 3��
// Client
// ���� : �����
// �й� : 101969

#pragma comment(lib, "ws2_32")
#include "main.h"


HINSTANCE hInst; // Main instance
CRITICAL_SECTION cs; // Critical Section ���� ����
WSADATA wsa_data; // WINSOCK ����
SOCKET serverSocket; // ������ ����� ���� ����

UserList* userListHeader; // ������ ����Ʈ�� header
UserList userList; // ������ ����Ʈ�� ù��° ��� (header����)
setupInfo setup; // ���� ���� ���¸� �����ϴ� setupInfo ����ü ����
senderArgument sendArgument; // send���� �Ѱ��� arguemtn�� �����ϴ� ����ü ����

HANDLE hMainRecv; // Receiver Thread �ڵ�
HANDLE hSender; // Sender Thread �ڵ�
HANDLE hSendEvent; // �޽��� ������ �غ�Ǿ��ٴ� event
HANDLE hSendOverEvent; // �޽��� ������ �����ٴ� event
HANDLE hMainThread; // Main Thread �ڵ�


HWND hEdit1; // ä�� �޽����� ����ϴ� Edit Control �ڵ�
HWND hEdit2; // ä�� �޽����� �Է��ϴ� Edit Control �ڵ�
HWND hList; // ������ ����ڸ� ǥ���ϴ� List Box �ڵ�

HWND hMenu; // �޴� �ڵ�

HWND hIp; // IP�� �Է��ϴ� �ڵ�
HWND hPort; // PORT�� �Է��ϴ� �ڵ�
HWND hNick; // �г����� �Է��ϴ� Edit control �ڵ�


// WinMain
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

	// ����� ����Ʈ �ʱ�ȭ
	memset(&userList,0, sizeof(userList));
	userListHeader = &userList;
	
	// Create Event
	hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hSendEvent == NULL) err_quit("CreateEvent()");

	hSendOverEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hSendOverEvent == NULL) err_quit("CreateEvent()");
	
	// Main dialog Box ����
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)main_DlgProc);

	// delete critical section
	DeleteCriticalSection(&cs);
	// Remove Event
	CloseHandle(hSendEvent);
	CloseHandle(hSendOverEvent);
	// WINSOCK 
	WSACleanup();
	return 0;
}
