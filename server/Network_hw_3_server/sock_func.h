#pragma once
#include "main.h"

// Receiver Thread
UINT WINAPI MainReceiver(LPVOID arg);
// Sender Thread
UINT WINAPI MsgSender(LPVOID arg);

// sock에 해당하는 클라이언트에게 메시지를 전송
int sendToClient(SOCKET sock, int flag, int size, char* Data);
// sock에 해당하는 클라이언트를 제외하고 모든 클라이언트에게 메시지를 전송
int sendToAllClient(SOCKET, int, int, char*);
// 시간 정보 가져오는 함수
struct tm* gettime();

// 사용자 접속시 사용자 목록에서 사용자 정보를 추가하는 함수
BOOL addUser(SOCKET);
// 사용자 퇴장시 사용자 목록에서 사용자 정보를 삭제하는 함수
BOOL delUser(UserList*);
// 사용자 리스트에서 사용자 정보를 검색하는 함수
UserList* searchNick(char*);