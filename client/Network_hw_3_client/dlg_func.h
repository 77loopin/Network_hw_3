#pragma once
#include "main.h"

// Main DialogBox
BOOL CALLBACK main_DlgProc(HWND, UINT, WPARAM, LPARAM);
// Setup DialogBox
BOOL CALLBACK setup_DlgProc(HWND, UINT, WPARAM, LPARAM);
// Nickname Setup DialogBox
BOOL CALLBACK nickSetup_DlgProc(HWND, UINT, WPARAM, LPARAM);

// 메시지 출력창에 메시지를 출력하는 함수
void DisplayText(char *fmt, ...);
// IP가 유효한지 확인하는 함수
BOOL check_ip(char*,int);
// PORT가 유효한지 확인하는 함수
BOOL check_port(char*);
// Nickname이 유효한지 확인하는 함수
BOOL check_nick(char*);
// UserList를 갱신하는 함수
void updateUserList();
// Nickname을 변경하는 함수
int changeNick(char*, char*);
