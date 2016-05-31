#include "sock_func.h"



UINT WINAPI MainReceiver(LPVOID arg) {
	BOOL sock_option;
	SOCKET sock;
	msgData msg;
	msgHeader header;
	char nickname[MAXNICK + 1];
	struct tm *timeinfo;
	int connectFlag;

	while (1) {
		recv(sock, (char*)&header, sizeof(header), 0);
		timeinfo = gettime();
		EnterCriticalSection(&cs);
		connectFlag = setup.connectFlag;
		LeaveCriticalSection(&cs);
		if (connectFlag == 1) { // 채팅방 미 접속 상태
			switch (header.flag) {
			case 1: // chatting room accept
				recv(sock, nickname, header.size, 0); // nickname
				EnterCriticalSection(&cs);
				setup.connectFlag = 2;
				strncpy(setup.myNick,nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				LeaveCriticalSection(&cs);
				MessageBox(NULL,"반갑습니다.\n\n채팅방에 접속했습니다.\n\n","채팅방 접속 성공",MB_ICONINFORMATION);
				break;
			case 2: // chatting room deny
				MessageBox(NULL, "채팅방에 이미 존재하는 닉네임입니다.\n\n닉네임을 변경하세요.\n\n[설정] -> [닉네임 설정]\n\n", "채팅방 접속 실패", MB_ICONERROR);
				break;
			case 3: // server deny
				MessageBox(NULL, "더 이상 채팅방에 접속할 수 없습니다.\n\n다른 서버를 이용해주세요.\n\n[설정] -> [접속 설정]\n\n", "채팅방 접속 실패", MB_ICONERROR);
				// 일단 보류
				break;
			}
		}
		else if (connectFlag == 2) { // 채팅방 접속 상태
			switch (header.flag) {
			case 1:
				recv(sock, nickname, header.size, 0); // nickname
				EnterCriticalSection(&cs);
				strncpy(setup.myNick, nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				LeaveCriticalSection(&cs);
				MessageBox(NULL, "닉네임이 변경되었습니다.", "닉네임 변경", MB_ICONINFORMATION);
				break;
			case 2:
				MessageBox(NULL, "채팅방에 이미 존재하는 닉네임입니다.", "닉네임 변경 실패", MB_ICONERROR);
				break;
			case 4: // chatting room user list add
				recv(sock, nickname, header.size, 0); // nickname
				addNickName(nickname);
				break;
			case 5: // chatting room user list remove
				recv(sock, nickname, header.size, 0); // nickname
				delNickName(nickname);
				break;
			case 6:
				recv(sock, (char*)&msg, header.size, 0);
				//DisplayText("[%02d:%02d | %s -> %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, setup.myNick, msg.msg);
				break;
			case 7:
				recv(sock, (char*)&msg, header.size, 0); // message
				//DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, msg.msg);
				break;
			}
		}
	}
	return 0;
}


UINT WINAPI MsgSender(LPVOID arg) {
	int sendSize = 0;
	senderArgument* ar = (senderArgument*)arg;
	msgHeader header;
	header.flag = ar->flag;
	header.size = ar->size;

	sendSize = send(ar->sock, (char*)&header, sizeof(header), 0);
	if (ar->size != 0)
		sendSize = sendSize + send(ar->sock, ar->Data, ar->size, 0);

	return sendSize;
}


// ip와 port를 인자로 받아 SOCKET을 리턴해주는 함수
// socket() -> connect()
// 연결 실패시 -1을 리턴

UINT WINAPI AcceptReceiver(LPVOID arg) {
	static SOCKET sock;

	return 0;
}

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


void addNickName(char* nick) {
	UserList *temp;

	temp = userList.next;
	while (temp != NULL) {
		if (!strncmp(temp->Nick, nick, strlen(nick))) {
			break;
		}
		temp = temp->next;
	}

	if (temp != NULL) // 중복된 닉네임이 이미 있으면
		return;

	// 없으면
	temp = (UserList*)malloc(sizeof(UserList));
	strncpy(temp->Nick, nick, strlen(nick) + 1);
	temp->next = userList.next;
	if (temp->next != NULL) {
		temp->next->prev = temp;
	}
	
	userList.next = temp;
	temp->prev = &userList;
	

	return;
}


int delNickName(char* nick) {
	UserList* temp;
	temp = userList.next;
	while (temp != NULL) { // 삭제하려는 닉네임이 있으면
		if (!strncmp(temp->Nick, nick, strlen(nick))) {
			temp->prev->next = temp->next;
			if (temp->next != NULL) {
				temp->next->prev = temp->prev;
			}
			break;
		}
		temp = temp->next;
	}

	if (temp == NULL) // 삭제하려고하는 닉네임이 없으면
		return -1;
	free(temp);
	return 0;
}


// 보내려는 소켓(sock)
// 보내는 메시지 종류(flag)
// 보내는 메시지 크기(size)
// 보내는 메시지 데이터(Data)를 입력 받아
// 상대에게 데이터를 보내는 함수
int sendToServer(SOCKET sock, int flag, int size, char* Data) {
	static HANDLE hSender;
	senderArgument arg;
	arg.Data = Data;
	arg.size = size;
	arg.flag = flag;
	arg.sock = sock;
	hSender = NULL;
	while (hSender == NULL) {
		hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, &arg, 0, NULL);
	}
	//_endthreadex((UINT)hSender);
	return size + sizeof(msgHeader);
}


struct tm* gettime() {
	time_t timer;
	timer = time(NULL);
	return localtime(&timer);
}