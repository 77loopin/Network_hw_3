#include "sock_func.h"


// Main Thread 함수
UINT WINAPI MainThread(LPVOID arg) {

	SOCKET tempSocket;
	int retval;

	// 서버와 접속을 시도
	tempSocket = getConnection(setup.serverIP, setup.serverPort);

	// 접속 실채
	if (tempSocket == -1) {
		MessageBox(NULL, "서버와의 연결에 실패했습니다.\n\n프로그램을 종료합니다.\n\n", "서버 연결 실패", MB_ICONERROR);
		exit(-1);
		return 0;
	}

	// 접속되면 서버 접속 상태로 connectFlag를 설정하고 소켓 정보 저장
	setup.connectFlag = CONNECT_SVR;
	serverSocket = tempSocket;
	
	// Sender의 이번트 초기화
	ResetEvent(hSendEvent);
	SetEvent(hSendOverEvent);

	// Receiver Thread 실행
	hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "클라이언트 시작을 실패했습니다.\n프로그램을 종료합니다.", "스레드 생성 실패", MB_ICONERROR);
		exit(-1);
	}

	// Sender Thread 실행
	hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, NULL, 0, NULL);
	if (hSender == NULL) {
		MessageBox(NULL, "클라이언트 시작을 실패했습니다.\n프로그램을 종료합니다.", "스레드 생성 실패", MB_ICONERROR);
		exit(-1);
	}

	// 핸들 정보 저장
	HANDLE hThread[2];
	hThread[0] = hMainRecv;
	hThread[1] = hSender;
	// 두 스레드 중에서 하나라도 종료 될 때까지 대기
	retval = WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	// 두 스레드 중에서 하나라도 종료되면 나머지 스레드도 종료
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

	// 스레드 핸들 닫기
	CloseHandle(hMainRecv);
	CloseHandle(hSender);
	
	// 현재 채팅방 접속 중에 종료되면 서버와의 연결이 끊어졌음을 알림
	if (setup.connectFlag == CONNECT_CHAT) {
		MessageBox(NULL, "서버와 접속이 끊어졌습니다", "알림", MB_ICONINFORMATION);
		setup.connectFlag = CONNECT_INIT;
	}

	// 소켓 닫기
	closesocket(serverSocket);
	
	return 0;
}


// Receiver Thread 함수
UINT WINAPI MainReceiver(LPVOID arg) {
	msgData msg;				// 메시지 데이터를 저장하는 구조체 변수
	msgHeader header;			// 메시지 헤더를 저장하는 구조체 변수
	UserList* temp;				// 사용자 리스트 포인터
	char nickname[MAXNICK + 1];	// 닉네임을 저장하는 임시 변수
	struct tm *timeinfo;		// 시간 정보를 저장하는 변수
	int retval;					// 리턴값을 저장하는 임시 변수

	while (1) {
		// Header를 입력 받는다.
		retval = recv(serverSocket, (char*)&header, sizeof(header), 0);
		if (retval == SOCKET_ERROR || retval == 0) {
			return 0;
		}
		timeinfo = gettime(); // 입력 받은 시간 값을 가져온다.
		if (setup.connectFlag == CONNECT_SVR) { // 서버와 접속된 상태 (채팅방 미접속)
			switch (header.flag) { // header의 flag 확인
			case SVR_MSG_ACCEPT: // 채팅방 접속 허가 메시지
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0 )
					return 0;
				setup.connectFlag = CONNECT_CHAT; // 채팅방 접속 상태로 설정
				strncpy(setup.myNick,nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				delAllUser(); // 접속자 목록 초기화 ( 새로운 접속 )
				updateUserList(); // 사용자 목록을 화면에 갱신한다.
				MessageBox(NULL, "반갑습니다.\n\n채팅방에 접속하셨습니다.\n\n", "알림", MB_ICONINFORMATION);
				break;
			case SVR_MSG_DENY: // 채팅방 접속 거부 메시지
				MessageBox(NULL, "채팅방에 이미 존재하는 닉네임입니다.\n\n닉네임을 변경하세요.\n\n[설정] -> [닉네임 설정]\n\n", "채팅방 접속 실패", MB_ICONERROR);
				break;
			}
		}
		else if (setup.connectFlag == CONNECT_CHAT) { // 채팅방 접속 상태
			switch (header.flag) { // header의 flag 확인
			case SVR_MSG_ADDUSER: // 사용자 접속 메시지
				// 접속한 사용자 닉네임을 입력 받는다.
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				// 접속 알림
				DisplayText("[%02d:%02d | info] : %s님이 접속하셨습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				addUser(nickname); // 사용자 리스트에 추가
				break;
			case SVR_MSG_DELUSER: // 사용자 퇴장 메시지
				// 퇴장한 사용자 닉네임을 입력 받는다.
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				// 퇴장 알림
				DisplayText("[%02d:%02d | info] : %s님이 퇴장하셨습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
 				delUser(nickname); // 사용자 리스트에서 삭제
				break;

			case SVR_MSG_CHGNICK: // 닉네임 변경 내용
				// 변경한 사용자와 새로운 닉네임 정보를 받는다.
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				// 변경 알림
				DisplayText("[%02d:%02d | info] : %s님이 %s로 닉네임을 변경했습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, msg.msg,msg.nick);
				changeNick(msg.msg, msg.nick); // 닉네임 변경
				break;
			case SVR_MSG_ACCEPT: // 닉네임 변경 허가
				// 허가 메시지 수신
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				setup.connectFlag = CONNECT_CHAT; // 채팅방 접속 상태로 설정
				// 변경 알림
				DisplayText("[%02d:%02d | info] : %s님이 %s로 닉네임을 변경했습니다.\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, nickname);
				changeNick(setup.myNick, nickname); // 내 닉네임 변경
				strncpy(setup.myNick, nickname, header.size);

				break;
			case SVR_MSG_DENY: // 닉네임 변경 거부 메시지
				MessageBox(NULL, "채팅방에 이미 존재하는 닉네임입니다.\n\n닉네임을 변경하세요.\n\n[설정] -> [닉네임 설정]\n\n", "채팅방 접속 실패", MB_ICONERROR);
				break;
			case SVR_MSG_ALLMSG: // 전체 채팅 메시지
				// 메시지 수신
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // message
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				// 수신한 메시지 화면에 출력
				DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, msg.msg);
				break;
			case SVR_MSG_WHISPER: // 귓속말 메시지
				// 귓속말 메시지 수신
				retval = recv(serverSocket, (char*)&msg, header.size, 0);
				if (retval == SOCKET_ERROR || retval == 0)	
					return 0;
				// 귓속말 메시지 화면에 출력
				DisplayText("[%02d:%02d | 귓속말 %s -> %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, setup.myNick, msg.msg);
				break;
			case 8: // 최초 접속 시 현재 접속자 정보 가져오기
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				addUser(nickname); // 입력 받는 사용자 정보를 사용자 리스트에 추가
				break;
			}
		}
	}
	return 0;
}


// Msg Sender Thread Function
UINT WINAPI MsgSender(LPVOID arg) {
	msgHeader header;
	int retval;

	while (1) {
		WaitForSingleObject(hSendEvent, INFINITE); // hSendEvent가 set 될 때까지 대기

												   // sendArgument에서 flag와 size를 가져와서 저장한다.
		header.flag = sendArgument.flag;
		header.size = sendArgument.size;

		// Header를 먼저 보낸다.
		retval = send(sendArgument.sock, (char*)&header, sizeof(header), 0);

		if (retval == SOCKET_ERROR) { // 전송 실패시 종료
			break;
		}

		if (header.size != 0) { // 보낼 데이터가 있으면
								// sendArgument에서 보낼 데이터를 구성한 후에 데이터 전송
			retval = send(sendArgument.sock, (char*)sendArgument.Data, header.size, 0);
			if (retval == SOCKET_ERROR) {
				break;
			}
		}
		// hSendOverEvent를 set 한다.
		SetEvent(hSendOverEvent);
	}

	return retval;
}


// ip와 port를 인자로 받아 SOCKET을 리턴해주는 함수
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
	
	// 소켓 생성
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock == INVALID_SOCKET) {
		err_MB("socket()");
		return -1;
	}
	
	// 서버에 접속
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

	// hSendOverEvent가 set될 때까지 기다린다. (Sender Thread가 입력 받을 수 있는 상태)
	WaitForSingleObject(hSendOverEvent, INFINITE);

	// 보낼 값을 sendArgument에 설정한다.
	memcpy(&sendArgument.Data, Data, size);
	sendArgument.sock = sock;
	sendArgument.flag = flag;
	sendArgument.size = size;

	// Sender Thread가 메시지를 보낼 수 있도록 hSendEvent를 set 한다.
	SetEvent(hSendEvent);


	return size + sizeof(msgHeader);
}

// 현재 시간 정보를 가져오는 함수
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