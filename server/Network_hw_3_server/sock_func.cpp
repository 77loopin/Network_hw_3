#include "sock_func.h"



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


// Main Receiver Thread Function
UINT WINAPI MainReceiver(LPVOID arg) {
	SOCKET clientSocket;			// 클라이언트의 소켓
	SOCKADDR_IN clientInfo;			// 클라이언트의 네트워크 정보
	SOCKADDR_IN serverInfo;			// 서버의 네트워크 정보
	msgHeader header;				// 메시지 헤더 구조체
	UserList* temp;					// user list pointer
	UserList* ptr;					// user list pointer
	FD_SET fdSet;					// recv를 위한 FD_SET
	msgData msg;					// 받은 메시지 데이터를 저장하는 msgData 구조체
	char nick[MAXNICK + 1];			// 닉네임
	int size = sizeof(clientInfo);	// clientInfo의 크기
	int retval;						// return 값을 저장하는 임시 변수

	// 서버 소켓 구성
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_port = htons(9999);
	serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);

	// socket()
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		err_quit("socket()");
	}

	// bind()
	retval = bind(serverSocket, (SOCKADDR*)&serverInfo, sizeof(serverInfo));
	if (retval == SOCKET_ERROR) {
		err_quit("bind()");
	}

	// listen()
	retval = listen(serverSocket, 5);
	if (retval == -1) {
		err_quit("listen()");
	}

	while (1) {
		FD_ZERO(&fdSet); // FD_SET을 초기화
		FD_SET(serverSocket, &fdSet); // FD_SET에 서버 소켓을 설정

		temp = userListHeader->next;

		// 현재 접속한 모든 사용자들의 소켓을 FD_SET에 설정
		while (temp != NULL) {
			FD_SET(temp->clientSocket, &fdSet); // client socket listencer
			temp = temp->next;
		}

		// select() 함수로 recv 입력이 들어 올 때 까지 대기
		retval = select(0, &fdSet, NULL, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			err_MB("select()");
			break;
		}

		// 접속한 클라이언트가 있으면 accept()로 접속을 하고 addUser로 사용자 추가
		if (FD_ISSET(serverSocket, &fdSet)) {
			size = sizeof(clientInfo);
			clientSocket = accept(serverSocket, (SOCKADDR*)&clientInfo, &size);
			if (clientSocket == INVALID_SOCKET) {
				err_MB("accept()");
			}
			else {
				addUser(clientSocket);
			}
		}

		temp = userListHeader->next;

		// 현재 접속한 모든 사용자의 소켓을 확인하면서
		while (temp != NULL) {
			if (FD_ISSET(temp->clientSocket, &fdSet)) { // 사용자의 소켓이 FD_SET에 남아 있으면(입력이 있으면)
				// header를 받는다.
				retval = recv(temp->clientSocket, (char*)&header, sizeof(header), 0);
				if (retval == SOCKET_ERROR || retval == 0) { // 클라이언트 접속 종료 시 종료한 사용자를 사용자 리스트에서 제거
					UserList* t;
					t = temp;
					temp = temp->next;
					strncpy(nick, t->Nick, strlen(t->Nick) + 1); // 퇴장한 사용자 닉네임 확인
					sendToAllClient(t->clientSocket, SVR_MSG_DELUSER, strlen(nick) + 1, nick); // 다른 사용자들에게 퇴장 여부 알림
					delUser(t); // 퇴장한 사용자 삭제
					continue;
				}

				switch (header.flag) { // header의 flag값을 확인
				case CLT_MSG_NEWUSER: // 채팅방 최초 접속 시 닉네임 설정
					// 접속을 요청하는 사용자의 nickname을 확인
					retval = recv(temp->clientSocket, nick, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					if (searchNick(nick) == NULL) { // 닉네임 사용 가능
						strncpy(temp->Nick, nick, strlen(nick) + 1);
						temp->connectFlag = CONNECT_CHAT; // connectFlag를 채팅방 접속으로 설정하고
						sendToClient(temp->clientSocket, SVR_MSG_ACCEPT, strlen(nick) + 1, nick); // 닉네임 허가 알림
						sendToAllClient(temp->clientSocket, SVR_MSG_ADDUSER, strlen(nick) + 1, nick); // 다른 사용자에게 새로운 접속자 알림
						ptr = userListHeader->next;
						while (ptr != NULL) { // 해당 클라이언트에 현재 채팅방에 접속한 사용자 목록을 보내준다.
							if (strcmp(ptr->Nick, temp->Nick)) {
								sendToClient(temp->clientSocket, SVR_MSG_ADVUSER, strlen(ptr->Nick) + 1, ptr->Nick);
							}
							ptr = ptr->next;
						}
					}
					else {  // 닉네임 중복 접속 거부
						sendToClient(temp->clientSocket, SVR_MSG_DENY, 0, NULL); // 닉네임 설정 거부
					}
					break;
				case CLT_MSG_CHGNICK: // 채팅방에서 닉네임 변경 시
					// 닉네임 변경을 요청하는 사용자의 nickname을 확인
					retval = recv(temp->clientSocket, nick, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					if (searchNick(nick) == NULL) { // 닉네임 사용 가능
						strncpy(msg.msg, temp->Nick, strlen(temp->Nick) + 1); // 보내는 메시지에 새로운 닉네임 설정
						strncpy(msg.nick, nick, strlen(nick) + 1); // 보내는 메시지의 닉네임을 old 닉네음으로 설정

						strncpy(temp->Nick, nick, strlen(nick) + 1); // nickname change

						temp->connectFlag = CONNECT_CHAT; // 채팅방 접속 상태로 설정
						sendToClient(temp->clientSocket, SVR_MSG_ACCEPT, strlen(nick) + 1, nick); // 닉네임 허가 알림
						sendToAllClient(temp->clientSocket, SVR_MSG_CHGNICK, sizeof(msg), (char*)&msg); // 다른 사용자에게 닉네임 변경 알림
					}
					else { // 닉네임 변경 거부
						sendToClient(temp->clientSocket, SVR_MSG_DENY, 0, NULL); // 닉네임 변경 거부
					}
					break;
				case CLT_MSG_ALLMSG: // 전체 메시지
					// msg를 입력 받는다.
					retval = recv(temp->clientSocket, (char*)&msg, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					// 메시지를 보낸 사용자를 제외한 모든 사용자들에게 메시지 전송
					sendToAllClient(temp->clientSocket, SVR_MSG_ALLMSG, sizeof(msgData), (char*)&msg);
					break;
				case CLT_MSG_WHISPER: // 귓속말
					// 보낸 귓속말을 입력 받는다.
					retval = recv(temp->clientSocket, (char*)&msg, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					// 보내는 상대방 닉네임으로 상대방의 사용자 정보를 검색한다.
					ptr = searchNick(msg.nick);
					if (ptr != NULL) {
						strncpy(msg.nick, temp->Nick, strlen(temp->Nick) + 1); // nick 멤버변수에 귓속말을 보낸 사용자의 닉네임으로 설정
						sendToClient(ptr->clientSocket, SVR_MSG_WHISPER, sizeof(msgData), (char*)&msg); // 메시지를 받는 사용자에게 전송
					}

				}
			}
			temp = temp->next;
		}
	}
	return 0;
}

// 사용자 리스트에 사용자를 추가하는 함순
BOOL addUser(SOCKET sock) {
	UserList *temp1,*temp2; // 임시변수

	temp1 = userListHeader;
	
	while (temp1->next != NULL) { // 사용자들 중에 같은 socket값을 갖는 사용자가 없으면

		if (temp1->next->clientSocket == sock) {
			return FALSE;
		}
		temp1 = temp1->next;
	}

	// 사용자 정보를 생성
	temp2 = (UserList*)malloc(sizeof(UserList));
	
	// connectFlag는 서버 접속 상태로 설정하고 저장
	temp2->clientSocket = sock;
	temp2->connectFlag = CONNECT_SVR;
	
	temp2->next = temp1->next;
	temp1->next = temp2;
	temp2->prev = temp1;

	return TRUE;
}

// 사용자 리스트에서 사용자를 삭제하는 함수
BOOL delUser(UserList* user) {
	
	// 사용자를 리스트에서 제거
	user->prev->next = user->next;
	if (user->next != NULL) {
		user->next->prev = user->prev;
	}
	// 사용자의 소켓 종료
	closesocket(user->clientSocket);
	// 사용자 정보를 메모리에서 제거
	free(user);
	return 0;
}

// 사용자 리스트에서 닉네임으로 검색해서 해당 사용자 정보를 리턴하는 함수
UserList* searchNick(char* nick) {
	UserList *temp;

	temp = userListHeader->next;

	while (temp != NULL) {
		if (!strcmp(temp->Nick, nick)) {
			return temp; // 닉네임이 같으면 리턴
		}
		temp = temp->next;
	}

	return temp; // 노드가 없으면 NULL값 리턴
}


// 보내려는 소켓(sock)
// 보내는 메시지 종류(flag)
// 보내는 메시지 크기(size)
// 보내는 메시지 데이터(Data)를 입력 받아
// Sender에서 sendArgument를 넘겨주는 함수
int sendToClient(SOCKET sock, int flag, int size, char* Data) {
	
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

// 입력받은 sock을 제외하고 모든 사용자들에게 메시지를 전송하는 함수
// 모든 사용에게 sendToClient함수를 이용하여 전송
int sendToAllClient(SOCKET sock, int flag, int size, char*  Data) {

	UserList* temp;
	
	temp = userListHeader->next;
	while (temp != NULL) {
		if (temp->clientSocket != sock) {
			sendToClient(temp->clientSocket, flag, size, Data);
		}
		temp = temp->next;
	}
	return size + sizeof(msgHeader);
}

// 현재 시간 정보를 가져오는 함수
struct tm* gettime() {
	time_t timer;
	timer = time(NULL);
	return localtime(&timer);
}