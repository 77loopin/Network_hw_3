#include "sock_func.h"


// Main Thread �Լ�
UINT WINAPI MainThread(LPVOID arg) {

	SOCKET tempSocket;
	int retval;

	// ������ ������ �õ�
	tempSocket = getConnection(setup.serverIP, setup.serverPort);

	// ���� ��ä
	if (tempSocket == -1) {
		MessageBox(NULL, "�������� ���ῡ �����߽��ϴ�.\n\n���α׷��� �����մϴ�.\n\n", "���� ���� ����", MB_ICONERROR);
		exit(-1);
		return 0;
	}

	// ���ӵǸ� ���� ���� ���·� connectFlag�� �����ϰ� ���� ���� ����
	setup.connectFlag = CONNECT_SVR;
	serverSocket = tempSocket;
	
	// Sender�� �̹�Ʈ �ʱ�ȭ
	ResetEvent(hSendEvent);
	SetEvent(hSendOverEvent);

	// Receiver Thread ����
	hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "Ŭ���̾�Ʈ ������ �����߽��ϴ�.\n���α׷��� �����մϴ�.", "������ ���� ����", MB_ICONERROR);
		exit(-1);
	}

	// Sender Thread ����
	hSender = (HANDLE)_beginthreadex(NULL, 0, MsgSender, NULL, 0, NULL);
	if (hSender == NULL) {
		MessageBox(NULL, "Ŭ���̾�Ʈ ������ �����߽��ϴ�.\n���α׷��� �����մϴ�.", "������ ���� ����", MB_ICONERROR);
		exit(-1);
	}

	// �ڵ� ���� ����
	HANDLE hThread[2];
	hThread[0] = hMainRecv;
	hThread[1] = hSender;
	// �� ������ �߿��� �ϳ��� ���� �� ������ ���
	retval = WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	// �� ������ �߿��� �ϳ��� ����Ǹ� ������ �����嵵 ����
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

	// ������ �ڵ� �ݱ�
	CloseHandle(hMainRecv);
	CloseHandle(hSender);
	
	// ���� ä�ù� ���� �߿� ����Ǹ� �������� ������ ���������� �˸�
	if (setup.connectFlag == CONNECT_CHAT) {
		MessageBox(NULL, "������ ������ ���������ϴ�", "�˸�", MB_ICONINFORMATION);
		setup.connectFlag = CONNECT_INIT;
	}

	// ���� �ݱ�
	closesocket(serverSocket);
	
	return 0;
}


// Receiver Thread �Լ�
UINT WINAPI MainReceiver(LPVOID arg) {
	msgData msg;				// �޽��� �����͸� �����ϴ� ����ü ����
	msgHeader header;			// �޽��� ����� �����ϴ� ����ü ����
	UserList* temp;				// ����� ����Ʈ ������
	char nickname[MAXNICK + 1];	// �г����� �����ϴ� �ӽ� ����
	struct tm *timeinfo;		// �ð� ������ �����ϴ� ����
	int retval;					// ���ϰ��� �����ϴ� �ӽ� ����

	while (1) {
		// Header�� �Է� �޴´�.
		retval = recv(serverSocket, (char*)&header, sizeof(header), 0);
		if (retval == SOCKET_ERROR || retval == 0) {
			return 0;
		}
		timeinfo = gettime(); // �Է� ���� �ð� ���� �����´�.
		if (setup.connectFlag == CONNECT_SVR) { // ������ ���ӵ� ���� (ä�ù� ������)
			switch (header.flag) { // header�� flag Ȯ��
			case SVR_MSG_ACCEPT: // ä�ù� ���� �㰡 �޽���
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0 )
					return 0;
				setup.connectFlag = CONNECT_CHAT; // ä�ù� ���� ���·� ����
				strncpy(setup.myNick,nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				delAllUser(); // ������ ��� �ʱ�ȭ ( ���ο� ���� )
				updateUserList(); // ����� ����� ȭ�鿡 �����Ѵ�.
				MessageBox(NULL, "�ݰ����ϴ�.\n\nä�ù濡 �����ϼ̽��ϴ�.\n\n", "�˸�", MB_ICONINFORMATION);
				break;
			case SVR_MSG_DENY: // ä�ù� ���� �ź� �޽���
				MessageBox(NULL, "ä�ù濡 �̹� �����ϴ� �г����Դϴ�.\n\n�г����� �����ϼ���.\n\n[����] -> [�г��� ����]\n\n", "ä�ù� ���� ����", MB_ICONERROR);
				break;
			}
		}
		else if (setup.connectFlag == CONNECT_CHAT) { // ä�ù� ���� ����
			switch (header.flag) { // header�� flag Ȯ��
			case SVR_MSG_ADDUSER: // ����� ���� �޽���
				// ������ ����� �г����� �Է� �޴´�.
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				// ���� �˸�
				DisplayText("[%02d:%02d | info] : %s���� �����ϼ̽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				addUser(nickname); // ����� ����Ʈ�� �߰�
				break;
			case SVR_MSG_DELUSER: // ����� ���� �޽���
				// ������ ����� �г����� �Է� �޴´�.
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				// ���� �˸�
				DisplayText("[%02d:%02d | info] : %s���� �����ϼ̽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
 				delUser(nickname); // ����� ����Ʈ���� ����
				break;

			case SVR_MSG_CHGNICK: // �г��� ���� ����
				// ������ ����ڿ� ���ο� �г��� ������ �޴´�.
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				// ���� �˸�
				DisplayText("[%02d:%02d | info] : %s���� %s�� �г����� �����߽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, msg.msg,msg.nick);
				changeNick(msg.msg, msg.nick); // �г��� ����
				break;
			case SVR_MSG_ACCEPT: // �г��� ���� �㰡
				// �㰡 �޽��� ����
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				setup.connectFlag = CONNECT_CHAT; // ä�ù� ���� ���·� ����
				// ���� �˸�
				DisplayText("[%02d:%02d | info] : %s���� %s�� �г����� �����߽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, nickname);
				changeNick(setup.myNick, nickname); // �� �г��� ����
				strncpy(setup.myNick, nickname, header.size);

				break;
			case SVR_MSG_DENY: // �г��� ���� �ź� �޽���
				MessageBox(NULL, "ä�ù濡 �̹� �����ϴ� �г����Դϴ�.\n\n�г����� �����ϼ���.\n\n[����] -> [�г��� ����]\n\n", "ä�ù� ���� ����", MB_ICONERROR);
				break;
			case SVR_MSG_ALLMSG: // ��ü ä�� �޽���
				// �޽��� ����
				retval = recv(serverSocket, (char*)&msg, header.size, 0); // message
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				// ������ �޽��� ȭ�鿡 ���
				DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, msg.msg);
				break;
			case SVR_MSG_WHISPER: // �ӼӸ� �޽���
				// �ӼӸ� �޽��� ����
				retval = recv(serverSocket, (char*)&msg, header.size, 0);
				if (retval == SOCKET_ERROR || retval == 0)	
					return 0;
				// �ӼӸ� �޽��� ȭ�鿡 ���
				DisplayText("[%02d:%02d | �ӼӸ� %s -> %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, setup.myNick, msg.msg);
				break;
			case 8: // ���� ���� �� ���� ������ ���� ��������
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				addUser(nickname); // �Է� �޴� ����� ������ ����� ����Ʈ�� �߰�
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


// ip�� port�� ���ڷ� �޾� SOCKET�� �������ִ� �Լ�
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
	
	// ���� ����
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock == INVALID_SOCKET) {
		err_MB("socket()");
		return -1;
	}
	
	// ������ ����
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

// ���� �ð� ������ �������� �Լ�
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