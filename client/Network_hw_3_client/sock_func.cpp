#include "sock_func.h"

HANDLE hMainRecv;

UINT WINAPI MainThread(LPVOID arg) {
	SOCKET tempSocket;
	int retval;

	//MessageBox(NULL, "������ �������Դϴ�.", "������", MB_ICONINFORMATION);
	tempSocket = getConnection(setup.serverIP, setup.serverPort);
	if (tempSocket == -1) {
		MessageBox(NULL, "�������� ���ῡ �����߽��ϴ�.\n\n���α׷��� �����մϴ�.\n\n", "���� ���� ����", MB_ICONERROR);
		EndDialog(hMainDlg, 0);
		return 0;
	}
	
	setup.connectFlag = 1;
	serverSocket = tempSocket;
	
	hMainRecv = (HANDLE)_beginthreadex(NULL, 0, MainReceiver, NULL, 0, NULL);
	if (hMainRecv == NULL) {
		MessageBox(NULL, "Ŭ���̾�Ʈ ������ �����߽��ϴ�.\n���α׷��� �����մϴ�.", "������ ���� ����", MB_ICONERROR);
		exit(0);
	}
	
	MessageBox(NULL, "������ ���ӵǾ����ϴ�.\n\n", "", MB_OK);
	retval = WaitForSingleObject( hMainRecv, INFINITE);
	
	if (retval == 0) {
		TerminateThread(hMainRecv, 1);
	}
	CloseHandle(hMainRecv);
	
	if (setup.connectFlag == 2) {
		MessageBox(NULL, "������ ������ �������ϴ�.\n���α׷��� �����մϴ�.\n", "���� ����", MB_ICONERROR);
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
		if (connectFlag == 1) { // ä�ù� �� ���� ����
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
				MessageBox(NULL,"�ݰ����ϴ�.\n\nä�ù濡 �����߽��ϴ�.\n\n","ä�ù� ���� ����",MB_ICONINFORMATION);
				break;
			case 2: // chatting room deny
				MessageBox(NULL, "ä�ù濡 �̹� �����ϴ� �г����Դϴ�.\n\n�г����� �����ϼ���.\n\n[����] -> [�г��� ����]\n\n", "ä�ù� ���� ����", MB_ICONERROR);
				break;
			case 3: // server deny
				MessageBox(NULL, "�� �̻� ä�ù濡 ������ �� �����ϴ�.\n\n�ٸ� ������ �̿����ּ���.\n\n[����] -> [���� ����]\n\n", "ä�ù� ���� ����", MB_ICONERROR);
				// �ϴ� ����
				break;
			}
		}
		else if (connectFlag == 2) { // ä�ù� ���� ����
			switch (header.flag) {
			case 1:
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				EnterCriticalSection(&cs);
				DisplayText("[%02d:%02d | info] : %s���� %s�� �г����� �����߽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick,nickname);
				strncpy(setup.myNick, nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				LeaveCriticalSection(&cs);
				MessageBox(NULL, "�г����� ����Ǿ����ϴ�.", "�г��� ����", MB_ICONINFORMATION);
				break;
			case 2:
				MessageBox(NULL, "ä�ù濡 �̹� �����ϴ� �г����Դϴ�.", "�г��� ���� ����", MB_ICONERROR);
				break;
			case 4: // chatting room user list add
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s���� �����ϼ̽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
				addUser(nickname);
				break;
			case 5: // chatting room user list remove
				retval = recv(serverSocket, nickname, header.size, 0); // nickname
				if (retval == SOCKET_ERROR || retval == 0)
					return 0;
				DisplayText("[%02d:%02d | info] : %s���� �����ϼ̽��ϴ�.\n", timeinfo->tm_hour, timeinfo->tm_min, nickname);
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
		MessageBox(NULL, "�޽��� ���ۿ� �����߽��ϴ�.\n�ٽ� �õ����ּ���.\n", "����", MB_ICONERROR);
		return 0;
	}
	sendSize = retval;
	if (ar->size != 0){
		retval = send(ar->sock, ar->Data, ar->size, 0);
		if (retval == SOCKET_ERROR) {
			MessageBox(NULL, "�޽��� ���ۿ� �����߽��ϴ�.\n�ٽ� �õ����ּ���.\n", "����", MB_ICONERROR);
			return 0;
		}
		sendSize = sendSize + retval;
	}
	return sendSize;
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
		//display_MB("�������� ���ῡ �����߽��ϴ�.\n\n���� ������ �ٽ� Ȯ�����ֽñ� �ٶ��ϴ�.\n\n");
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

	// ������
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
	while (temp != NULL) { // �����Ϸ��� �г����� ������
		if (!strncmp(temp->Nick, nick, strlen(nick))) {
			temp->prev->next = temp->next;
			if (temp->next != NULL) {
				temp->next->prev = temp->prev;
			}
			break;
		}
		temp = temp->next;
	}
	if (temp == NULL) // �����Ϸ����ϴ� �г����� ������
		return -1;
	
	free(temp);
	return 0;
}

int delAllUser() {
	// ä�ù� ������ ��� �����
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


// �������� ����(sock)
// ������ �޽��� ����(flag)
// ������ �޽��� ũ��(size)
// ������ �޽��� ������(Data)�� �Է� �޾�
// ��뿡�� �����͸� ������ �Լ�
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