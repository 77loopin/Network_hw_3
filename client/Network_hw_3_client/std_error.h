#pragma once

// 여러가지 에러 표시 함수
#ifndef _ERROR_
#define _ERROR_
extern void display_MB(char *msg);
extern void err_MB(char* msg);
extern void err_quit(char *msg);
extern void err_display(char *msg);
#endif