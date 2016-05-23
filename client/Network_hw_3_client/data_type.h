#pragma once
#define MAXNICK 100
#define MAXMSG 500
#define MAXUSER 10

typedef struct SOCKETINFO {
	char ip[40];
	int port;
} sockInfo;

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