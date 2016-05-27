#include "sock_func.h"




DWORD WINAPI chatMsgSender(LPVOID arg) {
	
	return 0;
}


// ip와 port를 인자로 받아 SOCKET을 리턴해주는 함수
// socket() -> connect()
// 연결 실패시 -1을 리턴
SOCKET getConnection(char* ip, int port) {
	int retval;
	SOCKET sock;
	SOCKADDR_IN dest;

	// init SOCKADDR
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(ip);
	dest.sin_port = htons(port);
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock == INVALID_SOCKET) {
		err_MB("socket()");
		return -1;
	}
	
	retval = connect(sock, (SOCKADDR*)&dest, sizeof(dest));

	if (retval == SOCKET_ERROR) {
		//display_MB("서버와의 연결에 실패했습니다.\n\n서버 설정을 다시 확인해주시기 바랍니다.\n\n");
		//err_MB("connect()");
		return -1;
	}

	return sock;
}


// 보내려는 소켓(sock)
// 보내는 메시지 종류(flag)
// 보내는 메시지 크기(size)
// 보내는 메시지 데이터(Data)를 입력 받아
// 상대에게 데이터를 보내는 함수
int sendToServer(SOCKET sock, int flag, int size, char* Data) {
	int sendSize = 0;
	msgHeader header;
	header.flag = flag;
	header.size = size;
	
	EnterCriticalSection(&cs);
	sendSize = send(sock, (char*)&header, sizeof(header), 0);
	sendSize = sendSize + send(sock, Data, size, 0);
	LeaveCriticalSection(&cs);
	
	return sendSize;
}

int checkUserNick(SOCKET sock, char* nickname) {
	
	sendToServer(serverSocket, 2, sizeof(nickname) + 1, nickname);

}

int getUserList() {
}


