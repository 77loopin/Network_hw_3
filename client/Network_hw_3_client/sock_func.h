#pragma once
#include "main.h"

// Main Thread
UINT WINAPI MainThread(LPVOID arg);
// Receiver Thread
UINT WINAPI MainReceiver(LPVOID arg);
// Sender Thread
UINT WINAPI MsgSender(LPVOID arg);

// ������ �����ϴ� �Լ�
SOCKET getConnection(char* ip, int port);
// ������ �޽����� �����ϴ� �Լ�
int sendToServer(SOCKET sock, int flag, int size, char* Data);
// ���� �ð������� Ȯ���ϴ� �Լ�
struct tm* gettime();

// ����� ����Ʈ���� ����ڸ� �߰��ϴ� �Լ�
BOOL addUser(char*);
// ����� ����Ʈ���� ����ڸ� �����ϴ� �Լ�
int delUser(char*);
// ��� ����� ����Ʈ�� �����ϴ� �Լ�
int delAllUser();
// �г��ӿ� �ش��ϴ� ����� ������ �˻��ϴ� �Լ�
UserList* searchNick(char*);