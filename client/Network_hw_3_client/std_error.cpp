#include "std_lib.h"

void display_MB(char* msg) {
	MessageBox(NULL, msg, "Error", MB_ICONERROR);
}

void err_MB(char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

// ���Ͽ��� �߻��� ������ �޽��� �ڽ��� ����� �Ŀ�
// ���α׷��� �����Ű�� ����ó�� �Լ�
void err_quit(char *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���Ͽ��� �߻��� ������ �ܼ�â�� ����ϴ� ����ó�� �Լ�
void err_display(char *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	//DisplayText("[%s] : %s\r\n", msg, (char*)lpMsgBuf);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

