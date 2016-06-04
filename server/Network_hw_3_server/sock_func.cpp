#include "sock_func.h"



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
UINT WINAPI MainReceiver(LPVOID arg) {
	SOCKET clientSocket;
	SOCKADDR_IN clientInfo;
	msgHeader header;
	UserList* temp;
	UserList* ptr;
	FD_SET fdSet;
	senderArgument argument;
	msgData msg;
	char nick[MAXNICK + 1];
	char message[MAXMSG + 1];
	int size = sizeof(clientInfo);
	int retval;
	char ms[500]; // test용

	while (1) {
		FD_ZERO(&fdSet);
		FD_SET(serverSocket, &fdSet); // accept listener FD_SET

		temp = userListHeader->next;

		while (temp != NULL) {
			FD_SET(temp->clientSocket, &fdSet); // client socket listencer
			temp = temp->next;
		}

		retval = select(0, &fdSet, NULL, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			err_MB("select()");
			break;
		}

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

		while (temp != NULL) {
			//sprintf(ms, "check fd = %d", temp->clientSocket); MessageBox(NULL, ms, "test", MB_OK);
			if (FD_ISSET(temp->clientSocket, &fdSet)) {
				retval = recv(temp->clientSocket, (char*)&header, sizeof(header), 0);
				if (retval == SOCKET_ERROR || retval == 0) { // 클라이언트 접속 종료 시
					UserList* t;
					t = temp;
					temp = temp->next;
					strncpy(nick, t->Nick, strlen(t->Nick) + 1); // 퇴장한 사용자 닉네임 확인
					sendToAllClient(t->clientSocket, 2, strlen(nick) + 1, nick); // 퇴장 여부 알림
					delUser(t);
					continue;
				}
				switch (header.flag) {
				case 1: // 채팅방 접속 시 닉네임 설정
					retval = recv(temp->clientSocket, nick, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					if (searchNick(nick) == NULL) { // 닉네임 사용 가능
						strncpy(temp->Nick, nick, strlen(nick) + 1);
						temp->connectFlag = 2;
						//sprintf(ms, "Send to client\nData : %s\n", nick); MessageBox(NULL, ms, "test", MB_OK);
						sendToClient(temp->clientSocket, 4, strlen(nick) + 1, nick); // 닉네임 허가
						sendToAllClient(temp->clientSocket, 1, strlen(nick) + 1, nick); // 다른 사용자에게 새로운 접속자 알림
						ptr = userListHeader->next;
						while (ptr != NULL) {
							if (strcmp(ptr->Nick, temp->Nick)) {
								sendToClient(temp->clientSocket, 8, strlen(ptr->Nick) + 1, ptr->Nick);
							}
							ptr = ptr->next;
						}
					}
					else {  // 접속 거부
						//sprintf(ms, "Send to client\nDeny Nickname"); MessageBox(NULL, ms, "test", MB_OK);
						sendToClient(temp->clientSocket, 5, 0, NULL); // 닉네임 설정 거부
					}
					break;
				case 2: // 채팅방에서 닉네임 변경 시
					retval = recv(temp->clientSocket, nick, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					if (searchNick(nick) == NULL) { // 닉네임 사용 가능
						strncpy(msg.msg, temp->Nick, strlen(temp->Nick) + 1); // 보낼 메시지 데이터 new nickname
						strncpy(msg.nick, nick, strlen(nick) + 1); // old nickname

						strncpy(temp->Nick, nick, strlen(nick) + 1); // nickname change

						temp->connectFlag = 2;
						//sprintf(ms, "Send to client\nData : %s\n", nick); MessageBox(NULL, ms, "test", MB_OK);
						sendToClient(temp->clientSocket, 4, strlen(nick) + 1, nick); // 닉네임 허가
						sendToAllClient(temp->clientSocket, 3, sizeof(msg), (char*)&msg); // 다른 사용자에게 닉네임 변경 알림
					}
					else { // 변경 거부
						//sprintf(ms, "Send to client\nDeny Nickname"); MessageBox(NULL, ms, "test", MB_OK);
						sendToClient(temp->clientSocket, 5, 0, NULL); // 닉네임 변경 거부
					}
					break;
				case 4: // 전체 메시지
					retval = recv(temp->clientSocket, (char*)&msg, header.size, 0);
					//sprintf(ms, "Nick : %s\nData : %s\n", msg.nick, msg.msg); MessageBox(NULL, ms, "test", MB_OK);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					sendToAllClient(temp->clientSocket, 6, sizeof(msgData), (char*)&msg);
					break;
				case 5: // 귓속말
					retval = recv(temp->clientSocket, (char*)&msg, header.size, 0);
					//sprintf(ms, "Nick : %s\nData : %s\n", msg.nick, msg.msg); MessageBox(NULL, ms, "test", MB_OK);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					ptr = userListHeader->next;
					while (ptr != NULL) {
						if (!strcmp(ptr->Nick,msg.nick)) {
							break;
						}
					}
					if (ptr != NULL) { // 없는 사용자에게 귓속말
						sendToClient(ptr->clientSocket, 7, sizeof(msgData), (char*)&msg);
					}
				}
			}
			temp = temp->next;
		}
	}
	return 0;
}


BOOL addUser(SOCKET sock) {
	UserList *temp1,*temp2;

	temp1 = userListHeader;
	
	while (temp1->next != NULL) {

		if (temp1->next->clientSocket == sock) {
			return FALSE;
		}
		temp1 = temp1->next;
	}

	temp2 = (UserList*)malloc(sizeof(UserList));
	
	temp2->clientSocket = sock;
	temp2->connectFlag = 1;
	
	temp2->next = temp1->next;
	temp1->next = temp2;
	temp2->prev = temp1;

	return TRUE;
}

BOOL delUser(UserList* user) {
	
	user->prev->next = user->next;
	if (user->next != NULL) {
		user->next->prev = user->prev;
	}
	closesocket(user->clientSocket);
	free(user);
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
int sendToClient(SOCKET sock, int flag, int size, char* Data) {
	

	WaitForSingleObject(hSendOverEvent, INFINITE);
	
	memcpy(&sendArgument.Data, Data, size);
	sendArgument.sock = sock;
	sendArgument.flag = flag;
	sendArgument.size = size;
	
	SetEvent(hSendEvent);
	

	return size + sizeof(msgHeader);
}

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


struct tm* gettime() {
	time_t timer;
	timer = time(NULL);
	return localtime(&timer);
}