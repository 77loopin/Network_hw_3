// 2016년 1학기 네트워크프로그램 숙제 3번
// Client
// 성명 : 백승재
// 학번 : 101969

#pragma comment(lib, "ws2_32")
#include "main.h"


HINSTANCE hInst; // Main instance
CRITICAL_SECTION cs; // Critical Section 접근 변수
WSADATA wsa_data; // WINSOCK 변수
SOCKET serverSocket; // 서버와 연결된 소켓 변수

UserList* userListHeader; // 접속자 리스트의 header
UserList userList; // 접속자 리스트의 첫번째 노드 (header역할)
setupInfo setup; // 현재 접속 상태를 저장하는 setupInfo 구조체 변수
senderArgument sendArgument; // send에게 넘겨줄 arguemtn를 저장하는 구조체 변수

HANDLE hMainRecv; // Receiver Thread 핸들
HANDLE hSender; // Sender Thread 핸들
HANDLE hSendEvent; // 메시지 전송이 준비되었다는 event
HANDLE hSendOverEvent; // 메시지 전송이 끝났다는 event
HANDLE hMainThread; // Main Thread 핸들


HWND hEdit1; // 채팅 메시지를 출력하는 Edit Control 핸들
HWND hEdit2; // 채팅 메시지를 입력하는 Edit Control 핸들
HWND hList; // 접속한 사용자를 표시하는 List Box 핸들

HWND hMenu; // 메뉴 핸들

HWND hIp; // IP를 입력하는 핸들
HWND hPort; // PORT를 입력하는 핸들
HWND hNick; // 닉네임을 입력하는 Edit control 핸들


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

	// 사용자 리스트 초기화
	memset(&userList,0, sizeof(userList));
	userListHeader = &userList;
	
	// Create Event
	hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hSendEvent == NULL) err_quit("CreateEvent()");

	hSendOverEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hSendOverEvent == NULL) err_quit("CreateEvent()");
	
	// Main dialog Box 생성
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
