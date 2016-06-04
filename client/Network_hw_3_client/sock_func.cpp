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

	setup.connectFlag = 1;
	serverSocket = tempSocket;

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

	MessageBox(NULL, "서버와 접속되었습니다.\n\n", "", MB_OK);
	HANDLE hThread[2];
	hThread[0] = hMainRecv;
	hThread[1] = hSender;
	retval = WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	retval -= WAIT_OBJECT_0;
	if (retval == 0)
		TerminateThread(hSender, 1);
	else
		TerminateThread(hMainRecv, 1);

	CloseHandle(hMainRecv);
	CloseHandle(hSender);
	if (setup.connectFlag == 2) {
		MessageBox(NULL, "서버가 접속을 끊었습니다", "알림", MB_ICONINFORMATION);
		closesocket(serverSocket);
	}
	
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
	int connectFlag;
	int retval;
	char ms[1000];

	while (1) {
		retval = recv(serverSocket, (char*)&header, sizeof(header), 0);
		//sprintf(ms, "%d process\n\nflag : %d\nsize : %d\n",getpid(),header.flag,header.size); MessageBox(NULL, ms, "test", MB_OK);
		if (retval == SOCKET_ERROR || retval == 0) {
			return 0;
		}
		timeinfo = gettime();
		if (setup.connectFlag == 1) { // 채팅방 미 접속 상태
			switch (header.flag) {
			case 4: // 채팅방 접속 허가
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0 )
					return 0;
				setup.connectFlag = 2;
				strncpy(setup.myNick,nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				delAllUser(); // 접속자 목록 초기화
				MessageBox(NULL,"반갑습니다.\n\n채팅방에 접속했습니다.\n\n","채팅방 접속 성공",MB_ICONINFORMATION);
				break;
			case 5: // chatting room deny
				MessageBox(NULL, "채팅방에 이미 존재하는 닉네임입니다.\n\n닉네임을 변경하세요.\n\n[설정] -> [닉네임 설정]\n\n", "채팅방 접속 실패", MB_ICONERROR);
				break;
			case 0: // server deny
				MessageBox(NULL, "더 이상 채팅방에 접속할 수 없습니다.\n\n다른 서버를 이용해주세요.\n\n[설정] -> [접속 설정]\n\n", "채팅방 접속 실패", MB_ICONERROR);
				// 일단 보류
				break;
			}
		}
		else if (setup.connectFlag == 2) { // 채팅방 접속 상태
			switch (header.flag) {
			case 1: // chatting room user list add
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s님이 접속하셨습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				addUser(nickname);
				break;
			case 2: // chatting room user list remove
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s님이 퇴장하셨습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				delUser(nickname);
				break;

			case 3: // 닉네임 변경 내용
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s님이 %s로 닉네임을 변경했습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, msg.msg,msg.nick);
				temp = searchNick(msg.nick);
				if (temp != NULL) {
					strncpy(temp->Nick, msg.msg, strlen(msg.msg) + 1);
				}
				break;
			case 4: // 닉네임 변경 허가
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				setup.connectFlag = 2;
				DisplayText("[%02d:%02d | info] : %s님이 %s로 닉네임을 변경했습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, nickname);
				strncpy(setup.myNick, nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				break;
			case 5:
				MessageBox(NULL, "채팅방에 이미 존재하는 닉네임입니다.\n\n닉네임을 변경하세요.\n\n[설정] -> [닉네임 설정]\n\n", "채팅방 접속 실패", MB_ICONERROR);
				break;
			case 6:
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // message
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, msg.msg);
				break;
			case 7:
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
		//sprintf(ms, "flag : %d\nsize : %d\n",header.flag,header.size); MessageBox(NULL, ms, "test", MB_OK);

		retval = send(sendArgument.sock, (char*)&header, sizeof(header), 0);

		if (retval == SOCKET_ERROR) {
			//MessageBox(NULL, "Send Error", "Error", MB_ICONERROR);
			break;
		}

		if (header.size != 0) {
			retval = send(sendArgument.sock, (char*)sendArgument.Data, header.size, 0);
			if (retval == SOCKET_ERROR) {
				//	MessageBox(NULL, "Send Error", "Error", MB_ICONERROR);
				break;
			}
		}

		SetEvent(hSendOverEvent);
	}

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
		//display_MB("서버와의 연결에 실패했습니다.\n\n서버 설정을 다시 확인해주시기 바랍니다.\n\n");
		//err_MB("connect()");
		return -1;
	}

	return sock;
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

	return TRUE;
}


int delUser(char* nick) {
	UserList *temp;
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

	temp = userListHeader->next;

	while (temp != NULL) {
		if (!strcmp(temp->Nick, nick)) {
			return temp;
		}
		temp = temp->next;
	}

	return temp; // 노드가 없으면
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