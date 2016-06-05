#ifndef _DATATYPE_
#define _DATATYPE_
#define MAXNICK 100
#define MAXMSG 500
#define MAXUSER 10


#define CONNECT_INIT 0
#define CONNECT_SVR 1
#define CONNECT_CHAT 2


#define CLT_MSG_NEWUSER 1
#define CLT_MSG_CHGNICK 2
#define CLT_MSG_ALLMSG 4
#define CLT_MSG_WHISPER 5


#define SVR_MSG_ADDUSER 1
#define SVR_MSG_DELUSER 2
#define SVR_MSG_CHGNICK 3
#define SVR_MSG_ACCEPT 4
#define SVR_MSG_DENY 5
#define SVR_MSG_ALLMSG 6
#define SVR_MSG_WHISPER 7
#define SVR_MSG_ADVUSER 8


typedef struct MESSAGEHEADER {
	int flag;
	int size;
} msgHeader;

typedef struct MESSAGE {
	char nick[MAXNICK+1];
	char msg[MAXMSG+1];
} msgData;

typedef struct SENDERARG {
	SOCKET sock;
	int flag;
	int size;
	char Data[MAXMSG+1+MAXNICK+1];
} senderArgument;

typedef struct USERLIST {
	struct USERLIST *next;
	struct USERLIST *prev;
	char Nick[MAXNICK + 1];
} UserList;


typedef struct SETUPINFO {
	char myNick[MAXNICK + 1];
	char serverIP[40];
	int serverPort;
	int connectFlag;
}setupInfo;
#endif