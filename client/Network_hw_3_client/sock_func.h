#pragma once
#include "main.h"

// Main Thread
UINT WINAPI MainThread(LPVOID arg);
// Receiver Thread
UINT WINAPI MainReceiver(LPVOID arg);
// Sender Thread
UINT WINAPI MsgSender(LPVOID arg);

// 서버와 연결하는 함수
SOCKET getConnection(char* ip, int port);
// 서버에 메시지를 전송하는 함수
int sendToServer(SOCKET sock, int flag, int size, char* Data);
// 현재 시간정보를 확인하는 함수
struct tm* gettime();

// 사용자 리스트에서 사용자를 추가하는 함수
BOOL addUser(char*);
// 사용자 리스트에서 사용자를 삭제하는 함수
int delUser(char*);
// 모든 사용자 리스트를 삭제하는 함수
int delAllUser();
// 닉네임에 해당하는 사용자 정보를 검색하는 함수
UserList* searchNick(char*);