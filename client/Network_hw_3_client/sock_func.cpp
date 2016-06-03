#include "sock_func.h"

HANDLE hMainRecv;

UINT WINAPI MainThread(LPVOID arg) {
	SOCKET tempSocket;
	int retval;

	//MessageBox(NULL, "서버와 연결중입니다.", "접속중", MB_ICONINFORMATION);
	tempSocket = getConnection(setup.serverIP, setup.serverPort);
	if (tempSocket == -1) {
		MessageBox(NULL, "서버와의 연결에 실패했습니다.\n\n프로그램을 종료합니다.\n\n", "서버 연결 실패", MB_ICONERROR);
		EndDialog(hMainDlg, 0);
		return 0;
	}
	
	setup.connectFlag = 1;
	serverSocket = tempSocket;
	
	hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "클라이언트 시작을 실패했습니다.\n프로그램을 종료합니다.", "스레드 생성 실패", MB_ICONERROR);
		exit(0);
	}
	
	MessageBox(NULL, "서버와 접속되었습니다.\n\n", "", MB_OK);
	retval = WaitForSingleObject( hMainRecv, INFINITE);
	
	if (retval == 0) {
		TerminateThread(hMainRecv, 1);
	}
	CloseHandle(hMainRecv);
	
	if (setup.connectFlag == 2) {
		MessageBox(NULL, "서버가 접속을 끊었습니다.\n프로그램을 종료합니다.\n", "서버 오류", MB_ICONERROR);
		closesocket(serverSocket);
	}

	return 0;
}

UINT WINAPI MainReceiver(LPVOID arg) {
	BOOL sock_option;
	SOCKET sock;
	msgData msg;
	msgHeader header;
	char nickname[MAXNICK + 1];
	struct tm *timeinfo;
	int connectFlag;
	int retval;

	while (1) {
		retval = recv(serverSocket, (char*)&header, sizeof(header), 0);
		if (retval == SOCKET_ERROR || retval == 0) {
			return 0;
		}
		timeinfo = gettime();
		EnterCriticalSection(&cs);
		connectFlag = setup.connectFlag;
		LeaveCriticalSection(&cs);
		if (connectFlag == 1) { // 채팅방 미 접속 상태
			switch (header.flag) {
			case 1: // chatting room accept
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0 )
					return 0;
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
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				EnterCriticalSection(&cs);
				DisplayText("[%02d:%02d | info] : %s님이 %s로 닉네임을 변경했습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick,nickname);
				strncpy(setup.myNick, nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				LeaveCriticalSection(&cs);
				MessageBox(NULL, "닉네임이 변경되었습니다.", "닉네임 변경", MB_ICONINFORMATION);
				break;
			case 2:
				MessageBox(NULL, "채팅방에 이미 존재하는 닉네임입니다.", "닉네임 변경 실패", MB_ICONERROR);
				break;
			case 4: // chatting room user list add
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s님이 접속하셨습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				addUser(nickname);
				break;
			case 5: // chatting room user list remove
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s님이 퇴장하셨습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				delUser(nickname);
				break;
			case 6:
				retval = recv(serverSocket, (char*)&msg, header.size, 0);
				if (retval == SOCKET_ERROR || retval == 0)	
					return 0;
				DisplayText("[%02d:%02d | %s -> %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, setup.myNick, msg.msg);
				break;
			case 7:
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // message
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, msg.msg);
				break;
			}
		}
	}
	return 0;
}


UINT WINAPI MsgSender(LPVOID arg) {
	int sendSize = 0;
	int retval;
	senderArgument* ar = (senderArgument*)arg;
	msgHeader header;
	header.flag = ar->flag;
	header.size = ar->size;
	retval = send(ar->sock, (char*)&header, sizeof(header), 0);
	if (retval == SOCKET_ERROR) {
		MessageBox(NULL, "메시지 전송에 실패했습니다.\n다시 시도해주세요.\n", "오류", MB_ICONERROR);
		return 0;
	}
	sendSize = retval;
	if (ar->size != 0){
		retval = send(ar->sock, ar->Data, ar->size, 0);
		if (retval == SOCKET_ERROR) {
			MessageBox(NULL, "메시지 전송에 실패했습니다.\n다시 시도해주세요.\n", "오류", MB_ICONERROR);
			return 0;
		}
		sendSize = sendSize + retval;
	}
	return sendSize;
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


void addUser(char* nick) {
	UserList *temp1, *temp2;

	temp1 = userListHeader;
	while (temp1->next != NULL) {
		temp1 = temp1->next;
	}

	// 없으면
	temp2 = (UserList*)malloc(sizeof(UserList));
	strncpy(temp2->Nick, nick, strlen(nick) + 1);

	temp2->next = temp1->next;
	temp1->next = temp2;
	temp2->prev = temp1;
	return;
}


int delUser(char* nick) {
	UserList *temp;
	temp = userListHeader->next;
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

int delAllUser() {
	// 채팅방 접속자 목록 지우기
	UserList *temp1, *temp2;
	temp1 = userList.next;
	while (temp1 != NULL) {
		temp2 = temp1;
		temp1 = temp1->next;
		free(temp2);
	}
	userList.next = NULL;
	return 0;
}


// 보내려는 소켓(sock)
// 보내는 메시지 종류(flag)
// 보내는 메시지 크기(size)
// 보내는 메시지 데이터(Data)를 입력 받아
// 상대에게 데이터를 보내는 함수
int sendToServer(SOCKET sock, int flag, int size, char* Data) {
	static HANDLE hSender;
	static senderArgument arg;
	memcpy(arg.Data,Data,size);
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