// 2016�� 1�б� ��Ʈ��ũ���α׷� ���� 3��
// ���� : �����
// �й� : 101969

#pragma comment(lib, "ws2_32")
#include "main.h"
#include "resource.h"

#include "std_lib.h"
#include "std_error.h"
#include "dlg_func.h"
#include "data_type.h"

CRITICAL_SECTION cs; // critical section�� ���� �ϱ� ���� CS ���� ����
HINSTANCE hInst;
WSADATA wsa_data;
int a;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// Store instance
	hInst = hInstance;

	// init critical section
	InitializeCriticalSection(&cs);
	// WINSOCK init
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		err_quit("WSAStartup()");
	}

	// run DialogBox
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, main_DlgProc);

	// delete critical section
	DeleteCriticalSection(&cs);
	// WINSOCK 
	WSACleanup();
	return 0;
}

