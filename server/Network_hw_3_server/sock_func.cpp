#include "sock_func.h"



UINT WINAPI MsgSender(LPVOID arg) {
	int sendSize = 0;
	senderArgument* ar = (senderArgument*)arg;
	msgHeader header;
	char ms[1000];
	int retval;
	header.flag = ar->flag;
	header.size = ar->size;
	retval = send(ar->sock, (char*)&header, sizeof(header), 0);
	if (retval == SOCKET_ERROR) {
		MessageBox(NULL, "Send Error", "Error", MB_ICONERROR);
		return retval;
	}
	sendSize = retval;
	if (ar->size != 0) {
		//sprintf(ms, "flag : %d\nsize : %d\nNick : %s\nData : %s\n", ar->flag, ar->size, ((msgData*)ar->Data)->nick, ((msgData*)ar->Data)->msg); MessageBox(NULL, ms, "test", MB_OK);
		retval = send(ar->sock, ar->Data, ar->size, 0);
		if (retval == SOCKET_ERROR) {
			MessageBox(NULL, "Send Error", "Error", MB_ICONERROR);
			return retval;
		}
	}

	return sendSize + retval;
}


// ip와 port를 인자로 받아 SOCKET을 리턴해주는 함수
// socket() -> connect()
// 연결 실패시 -1을 리턴
UINT WINAPI MainReceiver(LPVOID arg) {
	SOCKET clientSocket;
	SOCKADDR_IN clientInfo;
	msgHeader header;
	UserList* temp;
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
				if (retval == SOCKET_ERROR || retval == 0) {
					UserList* t;
					t = temp;
					temp = temp->next;
					strncpy(nick, t->Nick, strlen(t->Nick) + 1);
					sendToAllClient(t->clientSocket, 5, strlen(nick) + 1, nick);
					delUser(t);
					continue;
				}
				switch (header.flag) {
				case 1: // 채팅방 접속 시 닉네임 설정
				case 2: // 채팅방에서 닉네임 변경 시
					retval = recv(temp->clientSocket, nick, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					if (checkNick(nick)) {
						strncpy(temp->Nick, nick, strlen(nick) + 1);
						temp->connectFlag = 2;
						//sprintf(ms, "Send to client\nData : %s\n", nick); MessageBox(NULL, ms, "test", MB_OK);
						sendToClient(temp->clientSocket, 1, strlen(nick) + 1, nick);
						sendToAllClient(temp->clientSocket, 4, strlen(nick) + 1, nick);
						// sent to all client user list update
					}
					else {
						//sprintf(ms, "Send to client\nDeny Nickname"); MessageBox(NULL, ms, "test", MB_OK);
						sendToClient(temp->clientSocket, 2, 0, NULL);
						// send to client deny message flag 2
					}
					break;
				case 4:
					retval = recv(temp->clientSocket, (char*)&msg, header.size, 0);
					//sprintf(ms, "Nick : %s\nData : %s\n", msg.nick, msg.msg); MessageBox(NULL, ms, "test", MB_OK);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					sendToAllClient(temp->clientSocket, 7, sizeof(msgData), (char*)&msg);
					break;
				case 5:
					// send to client whisper message
					break;
				}
			}
			temp = temp->next;
		}
	}
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


BOOL checkNick(char* nick) {
	UserList *temp1, *temp2;

	temp1 = userListHeader->next;

	while (temp1 != NULL) {
		if (!strncmp(temp1->Nick, nick, strlen(nick))) {
			return FALSE;
		}
		temp1 = temp1->next;
	}

	if (temp1 != NULL) {
		return FALSE;
	}
	return TRUE;
}


BOOL delUser(UserList* user) {
	
	/*
	temp = userListHeader

	while (temp->next != NULL) { // 삭제하려는 닉네임이 있으면
		if (temp->next->clientSocket == sock) {
			temp->prev->next = temp->next;
			if (temp->next != NULL) {
				temp->next->prev = temp->prev;
			}
			break;
		}
		temp = temp->next;
	}
	*/

	user->prev->next = user->next;
	if (user->next != NULL) {
		user->next->prev = user->prev;
	}
	closesocket(user->clientSocket);
	free(user);
	return 0;
}


// 보내려는 소켓(sock)
// 보내는 메시지 종류(flag)
// 보내는 메시지 크기(size)
// 보내는 메시지 데이터(Data)를 입력 받아
// 상대에게 데이터를 보내는 함수
int sendToClient(SOCKET sock, int flag, int size, char* Data) {
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

int sendToAllClient(SOCKET sock, int flag, int size, char*  Data) {
	UserList* temp;
	static HANDLE hSender;
	static senderArgument arg;
	memcpy(arg.Data, Data, size);
	arg.size = size;
	arg.flag = flag;
	
	temp = userListHeader->next;
	while (temp != NULL) {
		if (temp->clientSocket != sock) {
			arg.sock = temp->clientSocket;
			hSender = NULL;
			while (hSender == NULL) {
				hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, &arg, 0, NULL);
			}
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