#pragma once
#include "std_lib.h"
#include "std_error.h"
#include "data_type.h"
#include "dlg_func.h"
#include "sock_func.h"

// window 생성
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


extern HINSTANCE hInst; // Main Instance

extern CRITICAL_SECTION cs; // Critical Section 접근 변수
extern FD_SET fdSet; // recv 동기화를 위한 FD_SET 변수
extern WSADATA wsa_data; // WINSOCK 변수


extern UserList* userListHeader; // 접속자 리스트의 header
extern UserList userList; // 접속자 리스트의 첫번째 노드 (header역할)

extern senderArgument sendArgument; // send에게 넘겨줄 arguemtn를 저장하는 구조체 변수
extern SOCKET serverSocket; // 서버의 socket 정보를 담는 전역 변수

// HANDLE 목록
extern HANDLE hMainRecv; // Receiver Thread 핸들
extern HANDLE hSender; // Sender Thread 핸들
extern HANDLE hSendEvent; // 메시지 전송이 준비되었다는 event
extern HANDLE hSendOverEvent; // 메시지 전송이 끝났다는 event

