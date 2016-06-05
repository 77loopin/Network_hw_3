#ifndef _DATATYPE_
#define _DATATYPE_

// 데이터의 크기를 정의하는 매크로 상수
#define MAXNICK 100 // 최대 닉네임 길이
#define MAXMSG 500 // 최대 메시지 길이
#define MAXUSER 10 // 최대 접속사 수 ( 사용 안함 )


// 클라이언트의 접속 상태를 정의하는 매크로 상수
#define CONNECT_INIT 0	// 미 접속 상태
#define CONNECT_SVR 1	// 서버와 connect만 맺은 상태 (채팅방 미접속)
#define CONNECT_CHAT 2	// 닉네임을 설정하고 채팅방에 접속한 상태


// 클라이언트가 서버로 보내는 메시지의 종류를 정의하는 매크로 상수
#define CLT_MSG_NEWUSER 1	// 새로운 접속자
#define CLT_MSG_CHGNICK 2	// 닉네임 변경
#define CLT_MSG_ALLMSG 4	// 전체 채팅 메시지
#define CLT_MSG_WHISPER 5	// 귓속말


// 서버가 클라이언트로 보내는 메시지의 종류를 정의하는 매크로 상수
#define SVR_MSG_ADDUSER 1	// 사용자 접속 알림
#define SVR_MSG_DELUSER 2	// 사용자 퇴장 알림
#define SVR_MSG_CHGNICK 3	// 사용자 닉네임 변경 알림
#define SVR_MSG_ACCEPT 4	// 닉네임 설정 허가 메시지
#define SVR_MSG_DENY 5		// 닉네임 설정 거부 메시지
#define SVR_MSG_ALLMSG 6	// 전체 채팅 메시지
#define SVR_MSG_WHISPER 7	// 귓속말 채팅 메시지
#define SVR_MSG_ADVUSER 8	// 현재 모든 접속자 알림
//(최초 접속시 접속자 목록 가져올 때 사용)


// 전송할 Message의 종류와 크기를 저장하는 Message Header 구조체
// flag에 대한 설명은 정의 부분을 참고해주세요
typedef struct MESSAGEHEADER {
	int flag; // 전송할 메시지의 종류
	int size; // 전송할 메시지의 크기
} msgHeader;

// 전송할 메시지 구조체
typedef struct MESSAGE {
	char nick[MAXNICK + 1]; // 보내는 사람(전체 메시지) 혹은 받을 사람(귓속말)의 닉네임
	char msg[MAXMSG + 1]; // 보내는 메시지(전체 메시지) 혹은 변경할 닉네임 (닉네임 변경 시)
} msgData;

// 보낼 메시지에 대한 argument 정보를 담는 구조체
typedef struct SENDERARG {
	SOCKET sock; // 보낼 상대의 소켓 변수
	int flag; // 보낼 메시지의 종류
	int size; // 보낼 메시지의 크기
	char Data[MAXNICK + 1 + MAXMSG + 1]; // 보낼 메시지 (최대 MESSAGE 구조체의 크기)
} senderArgument;

// 현재 접속한 사용자 리스트
typedef struct USERLIST {
	struct USERLIST *next; // 리스트 ADT next
	struct USERLIST *prev; // 리스트 ADT prev
	SOCKET clientSocket; // 사용자에 대한 소켓 변수
	int connectFlag; // 사용자의 접속 상태
	char Nick[MAXNICK + 1]; // 사용자의 닉네임
} UserList;

// 현재 접속 정보를 저장하는 구조체 변수
typedef struct SETUPINFO {
	char myNick[MAXNICK + 1]; // 설정된 닉네임
	char serverIP[40]; // 서버 IP
	int serverPort; // 접속된 서버 PORT
	int connectFlag; // 현재 접속 상태를 저장하는 멤버변수
}setupInfo;
#endif