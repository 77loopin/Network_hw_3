#include "sock_func.h"



// Msg Sender Thread Function
UINT WINAPI MsgSender(LPVOID arg) {
	msgHeader header;
	int retval;

	while (1) {
		WaitForSingleObject(hSendEvent, INFINITE); // hSendEvent�� set �� ������ ���
		
		// sendArgument���� flag�� size�� �����ͼ� �����Ѵ�.
		header.flag = sendArgument.flag;
		header.size = sendArgument.size;

		// Header�� ���� ������.
		retval = send(sendArgument.sock, (char*)&header, sizeof(header), 0);

		if (retval == SOCKET_ERROR) { // ���� ���н� ����
			break;
		}

		if (header.size != 0) { // ���� �����Ͱ� ������
			// sendArgument���� ���� �����͸� ������ �Ŀ� ������ ����
			retval = send(sendArgument.sock, (char*)sendArgument.Data, header.size, 0);
			if (retval == SOCKET_ERROR) {
				break;
			}
		}
		// hSendOverEvent�� set �Ѵ�.
		SetEvent(hSendOverEvent);
	}

	return retval;
}


// Main Receiver Thread Function
UINT WINAPI MainReceiver(LPVOID arg) {
	SOCKET clientSocket;			// Ŭ���̾�Ʈ�� ����
	SOCKADDR_IN clientInfo;			// Ŭ���̾�Ʈ�� ��Ʈ��ũ ����
	SOCKADDR_IN serverInfo;			// ������ ��Ʈ��ũ ����
	msgHeader header;				// �޽��� ��� ����ü
	UserList* temp;					// user list pointer
	UserList* ptr;					// user list pointer
	FD_SET fdSet;					// recv�� ���� FD_SET
	msgData msg;					// ���� �޽��� �����͸� �����ϴ� msgData ����ü
	char nick[MAXNICK + 1];			// �г���
	int size = sizeof(clientInfo);	// clientInfo�� ũ��
	int retval;						// return ���� �����ϴ� �ӽ� ����

	// ���� ���� ����
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
		FD_ZERO(&fdSet); // FD_SET�� �ʱ�ȭ
		FD_SET(serverSocket, &fdSet); // FD_SET�� ���� ������ ����

		temp = userListHeader->next;

		// ���� ������ ��� ����ڵ��� ������ FD_SET�� ����
		while (temp != NULL) {
			FD_SET(temp->clientSocket, &fdSet); // client socket listencer
			temp = temp->next;
		}

		// select() �Լ��� recv �Է��� ��� �� �� ���� ���
		retval = select(0, &fdSet, NULL, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			err_MB("select()");
			break;
		}

		// ������ Ŭ���̾�Ʈ�� ������ accept()�� ������ �ϰ� addUser�� ����� �߰�
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

		// ���� ������ ��� ������� ������ Ȯ���ϸ鼭
		while (temp != NULL) {
			if (FD_ISSET(temp->clientSocket, &fdSet)) { // ������� ������ FD_SET�� ���� ������(�Է��� ������)
				// header�� �޴´�.
				retval = recv(temp->clientSocket, (char*)&header, sizeof(header), 0);
				if (retval == SOCKET_ERROR || retval == 0) { // Ŭ���̾�Ʈ ���� ���� �� ������ ����ڸ� ����� ����Ʈ���� ����
					UserList* t;
					t = temp;
					temp = temp->next;
					strncpy(nick, t->Nick, strlen(t->Nick) + 1); // ������ ����� �г��� Ȯ��
					sendToAllClient(t->clientSocket, SVR_MSG_DELUSER, strlen(nick) + 1, nick); // �ٸ� ����ڵ鿡�� ���� ���� �˸�
					delUser(t); // ������ ����� ����
					continue;
				}

				switch (header.flag) { // header�� flag���� Ȯ��
				case CLT_MSG_NEWUSER: // ä�ù� ���� ���� �� �г��� ����
					// ������ ��û�ϴ� ������� nickname�� Ȯ��
					retval = recv(temp->clientSocket, nick, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					if (searchNick(nick) == NULL) { // �г��� ��� ����
						strncpy(temp->Nick, nick, strlen(nick) + 1);
						temp->connectFlag = CONNECT_CHAT; // connectFlag�� ä�ù� �������� �����ϰ�
						sendToClient(temp->clientSocket, SVR_MSG_ACCEPT, strlen(nick) + 1, nick); // �г��� �㰡 �˸�
						sendToAllClient(temp->clientSocket, SVR_MSG_ADDUSER, strlen(nick) + 1, nick); // �ٸ� ����ڿ��� ���ο� ������ �˸�
						ptr = userListHeader->next;
						while (ptr != NULL) { // �ش� Ŭ���̾�Ʈ�� ���� ä�ù濡 ������ ����� ����� �����ش�.
							if (strcmp(ptr->Nick, temp->Nick)) {
								sendToClient(temp->clientSocket, SVR_MSG_ADVUSER, strlen(ptr->Nick) + 1, ptr->Nick);
							}
							ptr = ptr->next;
						}
					}
					else {  // �г��� �ߺ� ���� �ź�
						sendToClient(temp->clientSocket, SVR_MSG_DENY, 0, NULL); // �г��� ���� �ź�
					}
					break;
				case CLT_MSG_CHGNICK: // ä�ù濡�� �г��� ���� ��
					// �г��� ������ ��û�ϴ� ������� nickname�� Ȯ��
					retval = recv(temp->clientSocket, nick, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					if (searchNick(nick) == NULL) { // �г��� ��� ����
						strncpy(msg.msg, temp->Nick, strlen(temp->Nick) + 1); // ������ �޽����� ���ο� �г��� ����
						strncpy(msg.nick, nick, strlen(nick) + 1); // ������ �޽����� �г����� old �г������� ����

						strncpy(temp->Nick, nick, strlen(nick) + 1); // nickname change

						temp->connectFlag = CONNECT_CHAT; // ä�ù� ���� ���·� ����
						sendToClient(temp->clientSocket, SVR_MSG_ACCEPT, strlen(nick) + 1, nick); // �г��� �㰡 �˸�
						sendToAllClient(temp->clientSocket, SVR_MSG_CHGNICK, sizeof(msg), (char*)&msg); // �ٸ� ����ڿ��� �г��� ���� �˸�
					}
					else { // �г��� ���� �ź�
						sendToClient(temp->clientSocket, SVR_MSG_DENY, 0, NULL); // �г��� ���� �ź�
					}
					break;
				case CLT_MSG_ALLMSG: // ��ü �޽���
					// msg�� �Է� �޴´�.
					retval = recv(temp->clientSocket, (char*)&msg, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					// �޽����� ���� ����ڸ� ������ ��� ����ڵ鿡�� �޽��� ����
					sendToAllClient(temp->clientSocket, SVR_MSG_ALLMSG, sizeof(msgData), (char*)&msg);
					break;
				case CLT_MSG_WHISPER: // �ӼӸ�
					// ���� �ӼӸ��� �Է� �޴´�.
					retval = recv(temp->clientSocket, (char*)&msg, header.size, 0);
					if (retval == SOCKET_ERROR || retval == 0) {
						return 0;
					}
					// ������ ���� �г������� ������ ����� ������ �˻��Ѵ�.
					ptr = searchNick(msg.nick);
					if (ptr != NULL) {
						strncpy(msg.nick, temp->Nick, strlen(temp->Nick) + 1); // nick ��������� �ӼӸ��� ���� ������� �г������� ����
						sendToClient(ptr->clientSocket, SVR_MSG_WHISPER, sizeof(msgData), (char*)&msg); // �޽����� �޴� ����ڿ��� ����
					}

				}
			}
			temp = temp->next;
		}
	}
	return 0;
}

// ����� ����Ʈ�� ����ڸ� �߰��ϴ� �Լ�
BOOL addUser(SOCKET sock) {
	UserList *temp1,*temp2; // �ӽú���

	temp1 = userListHeader;
	
	while (temp1->next != NULL) { // ����ڵ� �߿� ���� socket���� ���� ����ڰ� ������

		if (temp1->next->clientSocket == sock) {
			return FALSE;
		}
		temp1 = temp1->next;
	}

	// ����� ������ ����
	temp2 = (UserList*)malloc(sizeof(UserList));
	
	// connectFlag�� ���� ���� ���·� �����ϰ� ����
	temp2->clientSocket = sock;
	temp2->connectFlag = CONNECT_SVR;
	
	temp2->next = temp1->next;
	temp1->next = temp2;
	temp2->prev = temp1;

	return TRUE;
}

// ����� ����Ʈ���� ����ڸ� �����ϴ� �Լ�
BOOL delUser(UserList* user) {
	
	// ����ڸ� ����Ʈ���� ����
	user->prev->next = user->next;
	if (user->next != NULL) {
		user->next->prev = user->prev;
	}
	// ������� ���� ����
	closesocket(user->clientSocket);
	// ����� ������ �޸𸮿��� ����
	free(user);
	return 0;
}

// ����� ����Ʈ���� �г������� �˻��ؼ� �ش� ����� ������ �����ϴ� �Լ�
UserList* searchNick(char* nick) {
	UserList *temp;

	temp = userListHeader->next;

	while (temp != NULL) {
		if (!strcmp(temp->Nick, nick)) {
			return temp; // �г����� ������ ����
		}
		temp = temp->next;
	}

	return temp; // ��尡 ������ NULL�� ����
}


// �������� ����(sock)
// ������ �޽��� ����(flag)
// ������ �޽��� ũ��(size)
// ������ �޽��� ������(Data)�� �Է� �޾�
// Sender���� sendArgument�� �Ѱ��ִ� �Լ�
int sendToClient(SOCKET sock, int flag, int size, char* Data) {
	
	// hSendOverEvent�� set�� ������ ��ٸ���. (Sender Thread�� �Է� ���� �� �ִ� ����)
	WaitForSingleObject(hSendOverEvent, INFINITE);
	
	// ���� ���� sendArgument�� �����Ѵ�.
	memcpy(&sendArgument.Data, Data, size);
	sendArgument.sock = sock;
	sendArgument.flag = flag;
	sendArgument.size = size;
	
	// Sender Thread�� �޽����� ���� �� �ֵ��� hSendEvent�� set �Ѵ�.
	SetEvent(hSendEvent);
	

	return size + sizeof(msgHeader);
}

// �Է¹��� sock�� �����ϰ� ��� ����ڵ鿡�� �޽����� �����ϴ� �Լ�
// ��� ��뿡�� sendToClient�Լ��� �̿��Ͽ� ����
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

// ���� �ð� ������ �������� �Լ�
struct tm* gettime() {
	time_t timer;
	timer = time(NULL);
	return localtime(&timer);
}