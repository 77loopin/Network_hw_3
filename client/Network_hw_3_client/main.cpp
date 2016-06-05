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

HANDLE hMainThread;
HANDLE hMainRecv;
HANDLE hSender;
HANDLE hSendEvent;
HANDLE hSendOverEvent;

CRITICAL_SECTION cs;

WSADATA wsa_data;
SOCKET serverSocket;

UserList userList;
UserList* userListHeader = NULL;

HWND hEdit1;
HWND hEdit2;
//HWND hEdit3;
HWND hList;

HWND hMenu;

HWND hIp;
HWND hPort;
HWND hNick;

setupInfo setup;
senderArgument sendArgument;


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

	memset(&userList,0, sizeof(userList));
	userListHeader = &userList;
	
	// Create Event
	hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hSendEvent == NULL) err_quit("CreateEvent()");

	hSendOverEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hSendOverEvent == NULL) err_quit("CreateEvent()");
	
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



/*
UINT WINAPI MainThread(LPVOID arg) {

	SOCKET tempSocket;
	int retval;
	while (1) {
		//MessageBox(NULL, "서버와 연결중입니다.", "접속중", MB_ICONINFORMATION);
		WaitForSingleObject(hSetupEvent, INFINITE);

		tempSocket = getConnection(setup.serverIP, setup.serverPort);

		if (tempSocket == -1) {
			MessageBox(NULL, "서버와의 연결에 실패했습니다.\n\n프로그램을 종료합니다.\n\n", "서버 연결 실패", MB_ICONERROR);
			EndDialog(hMainDlg, 0);
			return 0;
		}

		setup.connectFlag = 1;
		serverSocket = tempSocket;

		hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
		if (hMainRecv == NULL) {
			MessageBox(NULL, "클라이언트 시작을 실패했습니다.\n프로그램을 종료합니다.", "스레드 생성 실패", MB_ICONERROR);
			exit(0);
		}

		MessageBox(NULL, "서버와 접속되었습니다.\n\n", "", MB_OK);
		//retval = WaitForSingleObject(hMainRecv, INFINITE);

		if (retval == 0) {
			TerminateThread(hMainRecv, 1);
		}
		CloseHandle(hMainRecv);

		if (setup.connectFlag == 2) {
			MessageBox(NULL, "서버가 접속을 끊었습니다.\n프로그램을 종료합니다.\n", "서버 오류", MB_ICONERROR);
			closesocket(serverSocket);
		}
	}
	return 0;
}*/