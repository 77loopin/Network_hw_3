#ifndef _DATATYPE_
#define _DATATYPE_
#define MAXNICK 100
#define MAXMSG 500
#define MAXUSER 10


typedef struct MESSAGEHEADER {
	int flag;
	int size;
} msgHeader;

typedef struct MESSAGE {
	char nick[MAXNICK+1];
	char msg[MAXMSG+1];
} msgData;

typedef struct PACKETDATA {
	msgHeader header;
	msgData data;
} packetData;

typedef struct SETUPINFO {
	char myNick[MAXNICK + 1];
	char serverIP[40];
	int serverPort;
	int connectFlag;
}setupInfo;
#endif