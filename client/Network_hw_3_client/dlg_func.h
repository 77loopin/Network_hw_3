#pragma once
#include "main.h"

BOOL CALLBACK main_DlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK setup_DlgProc(HWND, UINT, WPARAM, LPARAM);

void DisplayText(char *fmt, ...);
BOOL check_ip(char*,int);
BOOL check_port(char*);
BOOL check_nick(char*);

extern HWND hEdit1;
extern HWND hEdit2;
extern HWND hEdit3;

extern HWND hMenu;