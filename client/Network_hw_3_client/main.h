#pragma once
#include "resource.h"
#include "std_lib.h"
#include "std_error.h"
#include "data_type.h"
#include "dlg_func.h"
#include "sock_func.h"


extern HINSTANCE hInst; // Main instance
extern CRITICAL_SECTION cs; // Critical Section 접근 변수
extern WSADATA wsa_data; // WINSOCK 변수
extern SOCKET serverSocket; // 서버와 연결된 소켓 변수

extern UserList* userListHeader; // 접속자 리스트의 header
extern UserList userList; // 접속자 리스트의 첫번째 노드 (header역할)
extern setupInfo setup; // 현재 접속 상태를 저장하는 setupInfo 구조체 변수
extern senderArgument sendArgument; // send에게 넘겨줄 arguemtn를 저장하는 구조체 변수

extern HANDLE hMainRecv; // Receiver Thread 핸들
extern HANDLE hSender; // Sender Thread 핸들
extern HANDLE hSendEvent; // 메시지 전송이 준비되었다는 event
extern HANDLE hSendOverEvent; // 메시지 전송이 끝났다는 event
extern HANDLE hMainThread; // Main Thread 핸들


extern HWND hEdit1; // 채팅 메시지를 출력하는 Edit Control 핸들
extern HWND hEdit2; // 채팅 메시지를 입력하는 Edit Control 핸들
extern HWND hList; // 접속한 사용자를 표시하는 List Box 핸들

extern HWND hMenu; // 메뉴 핸들

extern HWND hIp; // IP를 입력하는 핸들
extern HWND hPort; // PORT를 입력하는 핸들
extern HWND hNick; // 닉네임을 입력하는 Edit control 핸들