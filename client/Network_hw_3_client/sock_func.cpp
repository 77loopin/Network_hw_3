#include "sock_func.h"


UINT WINAPI MainThread(LPVOID arg) {

	SOCKET tempSocket;
	int retval;

	

	tempSocket = getConnection(setup.serverIP, setup.serverPort);

	if (tempSocket == -1) {
		MessageBox(NULL, "서버와의 연결에 실패했습니다.\n\n프로그램을 종료합니다.\n\n", "서버 연결 실패", MB_ICONERROR);
		exit(-1);
		return 0;
	}

	setup.connectFlag = CONNECT_SVR;
	serverSocket = tempSocket;
	ResetEvent(hSendEvent);
	SetEvent(hSendOverEvent);

	hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "클라이언트 시작을 실패했습니다.\n프로그램을 종료합니다.", "스레드 생성 실패", MB_ICONERROR);
		exit(-1);
	}

	hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, NULL, 0, NULL);
	if (hSender == NULL) {
		MessageBox(NULL, "클라이언트 시작을 실패했습니다.\n프로그램을 종료합니다.", "스레드 생성 실패", MB_ICONERROR);
		exit(-1);
	}

	HANDLE hThread[2];
	hThread[0] = hMainRecv;
	hThread[1] = hSender;
	retval = WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	retval -= WAIT_OBJECT_0;
	if (retval == 0) {
		TerminateThread(hSender, 1);
	}
	else if (retval == 1) {
		TerminateThread(hMainRecv, 1);
	}
	else if (retval == WAIT_FAILED){
		TerminateThread(hSender, 1);
		TerminateThread(hMainRecv, 1);
	}

	CloseHandle(hMainRecv);
	CloseHandle(hSender);
	if (setup.connectFlag == CONNECT_CHAT) {
		MessageBox(NULL, "서버와 접속이 끊어졌습니다", "알림", MB_ICONINFORMATION);
		setup.connectFlag = CONNECT_INIT;
	}
	closesocket(serverSocket);
	
	return 0;
}


UINT WINAPI MainReceiver(LPVOID arg) {
	BOOL sock_option;
	SOCKET sock;
	msgData msg;
	msgHeader header;
	UserList* temp;
	char nickname[MAXNICK + 1];
	struct tm *timeinfo;
	int retval;

	while (1) {
		retval = recv(serverSocket, (char*)&header, sizeof(header), 0);
		if (retval == SOCKET_ERROR || retval == 0) {
			return 0;
		}
		timeinfo = gettime();
		if (setup.connectFlag == CONNECT_SVR) { // 채팅방 미 접속 상태
			switch (header.flag) {
			case SVR_MSG_ACCEPT: // 채팅방 접속 허가
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0 )
					return 0;
				setup.connectFlag = CONNECT_CHAT;
				strncpy(setup.myNick,nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				delAllUser(); // 접속자 목록 초기화
				updateUserList();
				MessageBox(NULL, "반갑습니다.\n\n채팅방에 접속하셨습니다.\n\n", "알림", MB_ICONINFORMATION);
				break;
			case SVR_MSG_DENY: // chatting room deny
				MessageBox(NULL, "채팅방에 이미 존재하는 닉네임입니다.\n\n닉네임을 변경하세요.\n\n[설정] -> [닉네임 설정]\n\n", "채팅방 접속 실패", MB_ICONERROR);
				break;
			}
		}
		else if (setup.connectFlag == CONNECT_CHAT) { // 채팅방 접속 상태
			switch (header.flag) {
			case SVR_MSG_ADDUSER: // chatting room user list add
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s님이 접속하셨습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				addUser(nickname);
				break;
			case SVR_MSG_DELUSER: // chatting room user list remove
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s님이 퇴장하셨습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				delUser(nickname);
				break;

			case SVR_MSG_CHGNICK: // 닉네임 변경 내용
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s님이 %s로 닉네임을 변경했습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, msg.msg,msg.nick);
				//updateUserList();
				changeNick(msg.msg, msg.nick);
				break;
			case SVR_MSG_ACCEPT: // 닉네임 변경 허가
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				setup.connectFlag = CONNECT_CHAT;
				DisplayText("[%02d:%02d | info] : %s님이 %s로 닉네임을 변경했습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, nickname);
				//updateUserList();
				changeNick(setup.myNick, nickname);

				strncpy(setup.myNick, nickname, header.size);

				break;
			case SVR_MSG_DENY:
				MessageBox(NULL, "채팅방에 이미 존재하는 닉네임입니다.\n\n닉네임을 변경하세요.\n\n[설정] -> [닉네임 설정]\n\n", "채팅방 접속 실패", MB_ICONERROR);
				break;
			case SVR_MSG_ALLMSG:
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // message
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, msg.msg);
				break;
			case SVR_MSG_WHISPER:
				retval = recv(serverSocket, (char*)&msg, header.size, 0);
				if (retval == SOCKET_ERROR || retval == 0)	
					return 0;
				DisplayText("[%02d:%02d | 귓속말 %s -> %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, setup.myNick, msg.msg);
				break;
			case 8: // 최초 접속 시 현재 접속자 정보 가져오기
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				addUser(nickname);
				break;
			}
		}
	}
	return 0;
}

UINT WINAPI MsgSender(LPVOID arg) {
	msgHeader header;
	int retval;

	while (1) {
		WaitForSingleObject(hSendEvent, INFINITE);

		header.flag = sendArgument.flag;
		header.size = sendArgument.size;

		retval = send(sendArgument.sock, (char*)&header, sizeof(header), 0);

		if (retval == SOCKET_ERROR) {
			break;
		}

		if (header.size != 0) {
			retval = send(sendArgument.sock, (char*)sendArgument.Data, header.size, 0);
			if (retval == SOCKET_ERROR) {
				break;
			}
		}

		SetEvent(hSendOverEvent);
	}
	MessageBox(NULL, "Sender Exit", "eee", 0);
	return retval;
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

	WaitForSingleObject(hSendOverEvent, INFINITE);

	memcpy(&sendArgument.Data, Data, size);
	sendArgument.sock = sock;
	sendArgument.flag = flag;
	sendArgument.size = size;

	SetEvent(hSendEvent);


	return size + sizeof(msgHeader);
}


struct tm* gettime() {
	time_t timer;
	timer = time(NULL);
	return localtime(&timer);
}



BOOL addUser(char* nick) {
	UserList* temp;
	UserList* ptr = userListHeader;

	temp = (UserList*)malloc(sizeof(UserList));

	while (ptr->next != NULL) {
		ptr = ptr->next;
	}

	ptr->next = temp;
	temp->prev = ptr;
	temp->next = NULL;
	strncpy(temp->Nick, nick, strlen(nick) + 1);
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(temp->Nick));

	return TRUE;
}


int delUser(char* nick) {
	UserList *temp;
	int index;
	temp = userListHeader->next;
	while (temp != NULL) { // 삭제하려는 닉네임이 있으면
		if (!strcmp(temp->Nick, nick)) {
			temp->prev->next = temp->next;
			if (temp->next != NULL) {
				temp->next->prev = temp->prev;
			}
			break;
		}
		temp = temp->next;
	}

	index = SendMessage(hList, LB_FINDSTRING, 0, (LPARAM)(temp->Nick));
	if (index == LB_ERR) {
		// search error
		return -1;
	}
	SendMessage(hList, LB_DELETESTRING, index, 0);
	free(temp);
	return 0;
}


int delAllUser() {
	// 채팅방 모든 접속자 목록 지우기 ( 초기화 )
	UserList *temp;
	while (userListHeader->next != NULL) {
		temp = userListHeader->next;
		userListHeader->next = temp->next;
		free(temp);
	}

	return 0;
}


UserList* searchNick(char* nick) {
	UserList *temp;

	temp = userListHeader;

	while (temp != NULL) {
		if (!strcmp(temp->Nick, nick)) {
			return temp;
		}
		temp = temp->next;
	}

	return temp; // 노드가 없으면
}