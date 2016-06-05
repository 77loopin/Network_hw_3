#pragma once
#include "main.h"

// Receiver Thread
UINT WINAPI MainReceiver(LPVOID arg);
// Sender Thread
UINT WINAPI MsgSender(LPVOID arg);

// sock�� �ش��ϴ� Ŭ���̾�Ʈ���� �޽����� ����
int sendToClient(SOCKET sock, int flag, int size, char* Data);
// sock�� �ش��ϴ� Ŭ���̾�Ʈ�� �����ϰ� ��� Ŭ���̾�Ʈ���� �޽����� ����
int sendToAllClient(SOCKET, int, int, char*);
// �ð� ���� �������� �Լ�
struct tm* gettime();

// ����� ���ӽ� ����� ��Ͽ��� ����� ������ �߰��ϴ� �Լ�
BOOL addUser(SOCKET);
// ����� ����� ����� ��Ͽ��� ����� ������ �����ϴ� �Լ�
BOOL delUser(UserList*);
// ����� ����Ʈ���� ����� ������ �˻��ϴ� �Լ�
UserList* searchNick(char*);