#pragma once
#include "main.h"

// Main DialogBox
BOOL CALLBACK main_DlgProc(HWND, UINT, WPARAM, LPARAM);
// Setup DialogBox
BOOL CALLBACK setup_DlgProc(HWND, UINT, WPARAM, LPARAM);
// Nickname Setup DialogBox
BOOL CALLBACK nickSetup_DlgProc(HWND, UINT, WPARAM, LPARAM);

// �޽��� ���â�� �޽����� ����ϴ� �Լ�
void DisplayText(char *fmt, ...);
// IP�� ��ȿ���� Ȯ���ϴ� �Լ�
BOOL check_ip(char*,int);
// PORT�� ��ȿ���� Ȯ���ϴ� �Լ�
BOOL check_port(char*);
// Nickname�� ��ȿ���� Ȯ���ϴ� �Լ�
BOOL check_nick(char*);
// UserList�� �����ϴ� �Լ�
void updateUserList();
// Nickname�� �����ϴ� �Լ�
int changeNick(char*, char*);
