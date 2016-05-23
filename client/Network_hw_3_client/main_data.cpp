
#include "std_lib.h"
#include "data_type.h"

extern HINSTANCE hinst;
extern CRITICAL_SECTION cs;
extern WSADATA wsa_data;
extern sockInfo server_info;
extern SOCKET server_socket;
extern char userList[MAXUSER][MAXNICK + 1];