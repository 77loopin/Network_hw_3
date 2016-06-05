#include "sock_func.h"


UINT WINAPI MainThread(LPVOID arg) {

	SOCKET tempSocket;
	int retval;

	

	tempSocket = getConnection(setup.serverIP, setup.serverPort);

	if (tempSocket == -1) {
		MessageBox(NULL, "�������� ���ῡ �����߽��ϴ�.\n\n���α׷��� �����մϴ�.\n\n", "���� ���� ����", MB_ICONERROR);
		exit(-1);
		return 0;
	}

	setup.connectFlag = CONNECT_SVR;
	serverSocket = tempSocket;
	ResetEvent(hSendEvent);
	SetEvent(hSendOverEvent);

	hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "Ŭ���̾�Ʈ ������ �����߽��ϴ�.\n���α׷��� �����մϴ�.", "������ ���� ����", MB_ICONERROR);
		exit(-1);
	}

	hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, NULL, 0, NULL);
	if (hSender == NULL) {
		MessageBox(NULL, "Ŭ���̾�Ʈ ������ �����߽��ϴ�.\n���α׷��� �����մϴ�.", "������ ���� ����", MB_ICONERROR);
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
		MessageBox(NULL, "������ ������ ���������ϴ�", "�˸�", MB_ICONINFORMATION);
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
		if (setup.connectFlag == CONNECT_SVR) { // ä�ù� �� ���� ����
			switch (header.flag) {
			case SVR_MSG_ACCEPT: // ä�ù� ���� �㰡
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0 )
					return 0;
				setup.connectFlag = CONNECT_CHAT;
				strncpy(setup.myNick,nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				delAllUser(); // ������ ��� �ʱ�ȭ
				updateUserList();
				MessageBox(NULL, "�ݰ����ϴ�.\n\nä�ù濡 �����ϼ̽��ϴ�.\n\n", "�˸�", MB_ICONINFORMATION);
				break;
			case SVR_MSG_DENY: // chatting room deny
				MessageBox(NULL, "ä�ù濡 �̹� �����ϴ� �г����Դϴ�.\n\n�г����� �����ϼ���.\n\n[����] -> [�г��� ����]\n\n", "ä�ù� ���� ����", MB_ICONERROR);
				break;
			}
		}
		else if (setup.connectFlag == CONNECT_CHAT) { // ä�ù� ���� ����
			switch (header.flag) {
			case SVR_MSG_ADDUSER: // chatting room user list add
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s���� �����ϼ̽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				addUser(nickname);
				break;
			case SVR_MSG_DELUSER: // chatting room user list remove
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s���� �����ϼ̽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				delUser(nickname);
				break;

			case SVR_MSG_CHGNICK: // �г��� ���� ����
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s���� %s�� �г����� �����߽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, msg.msg,msg.nick);
				//updateUserList();
				changeNick(msg.msg, msg.nick);
				break;
			case SVR_MSG_ACCEPT: // �г��� ���� �㰡
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				setup.connectFlag = CONNECT_CHAT;
				DisplayText("[%02d:%02d | info] : %s���� %s�� �г����� �����߽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, nickname);
				//updateUserList();
				changeNick(setup.myNick, nickname);

				strncpy(setup.myNick, nickname, header.size);

				break;
			case SVR_MSG_DENY:
				MessageBox(NULL, "ä�ù濡 �̹� �����ϴ� �г����Դϴ�.\n\n�г����� �����ϼ���.\n\n[����] -> [�г��� ����]\n\n", "ä�ù� ���� ����", MB_ICONERROR);
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
				DisplayText("[%02d:%02d | �ӼӸ� %s -> %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, setup.myNick, msg.msg);
				break;
			case 8: // ���� ���� �� ���� ������ ���� ��������
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


// ip�� port�� ���ڷ� �޾� SOCKET�� �������ִ� �Լ�
// socket() -> connect()
// ���� ���н� -1�� ����
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


// �������� ����(sock)
// ������ �޽��� ����(flag)
// ������ �޽��� ũ��(size)
// ������ �޽��� ������(Data)�� �Է� �޾�
// ��뿡�� �����͸� ������ �Լ�
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
	while (temp != NULL) { // �����Ϸ��� �г����� ������
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
	// ä�ù� ��� ������ ��� ����� ( �ʱ�ȭ )
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

	return temp; // ��尡 ������
}