#include "sock_func.h"



UINT WINAPI MainReceiver(LPVOID arg) {
	BOOL sock_option;
	SOCKET sock;
	msgData msg;
	msgHeader header;
	char nickname[MAXNICK + 1];
	struct tm *timeinfo;
	int connectFlag;

	while (1) {
		recv(sock, (char*)&header, sizeof(header), 0);
		timeinfo = gettime();
		EnterCriticalSection(&cs);
		connectFlag = setup.connectFlag;
		LeaveCriticalSection(&cs);
		if (connectFlag == 1) { // ä�ù� �� ���� ����
			switch (header.flag) {
			case 1: // chatting room accept
				recv(sock, nickname, header.size, 0); // nickname
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
				recv(sock, nickname, header.size, 0); // nickname
				EnterCriticalSection(&cs);
				strncpy(setup.myNick, nickname, header.size);
				strncpy(userList.Nick, setup.myNick, header.size);
				LeaveCriticalSection(&cs);
				MessageBox(NULL, "�г����� ����Ǿ����ϴ�.", "�г��� ����", MB_ICONINFORMATION);
				break;
			case 2:
				MessageBox(NULL, "ä�ù濡 �̹� �����ϴ� �г����Դϴ�.", "�г��� ���� ����", MB_ICONERROR);
				break;
			case 4: // chatting room user list add
				recv(sock, nickname, header.size, 0); // nickname
				addNickName(nickname);
				break;
			case 5: // chatting room user list remove
				recv(sock, nickname, header.size, 0); // nickname
				delNickName(nickname);
				break;
			case 6:
				recv(sock, (char*)&msg, header.size, 0);
				//DisplayText("[%02d:%02d | %s -> %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, setup.myNick, msg.msg);
				break;
			case 7:
				recv(sock, (char*)&msg, header.size, 0); // message
				//DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, msg.nick, msg.msg);
				break;
			}
		}
	}
	return 0;
}


UINT WINAPI MsgSender(LPVOID arg) {
	int sendSize = 0;
	senderArgument* ar = (senderArgument*)arg;
	msgHeader header;
	header.flag = ar->flag;
	header.size = ar->size;

	sendSize = send(ar->sock, (char*)&header, sizeof(header), 0);
	if (ar->size != 0)
		sendSize = sendSize + send(ar->sock, ar->Data, ar->size, 0);

	return sendSize;
}


// ip�� port�� ���ڷ� �޾� SOCKET�� �������ִ� �Լ�
// socket() -> connect()
// ���� ���н� -1�� ����

UINT WINAPI AcceptReceiver(LPVOID arg) {
	static SOCKET sock;

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
		//display_MB("�������� ���ῡ �����߽��ϴ�.\n\n���� ������ �ٽ� Ȯ�����ֽñ� �ٶ��ϴ�.\n\n");
		//err_MB("connect()");
		return -1;
	}

	return sock;
}


void addNickName(char* nick) {
	UserList *temp;

	temp = userList.next;
	while (temp != NULL) {
		if (!strncmp(temp->Nick, nick, strlen(nick))) {
			break;
		}
		temp = temp->next;
	}

	if (temp != NULL) // �ߺ��� �г����� �̹� ������
		return;

	// ������
	temp = (UserList*)malloc(sizeof(UserList));
	strncpy(temp->Nick, nick, strlen(nick) + 1);
	temp->next = userList.next;
	if (temp->next != NULL) {
		temp->next->prev = temp;
	}
	
	userList.next = temp;
	temp->prev = &userList;
	

	return;
}


int delNickName(char* nick) {
	UserList* temp;
	temp = userList.next;
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


// �������� ����(sock)
// ������ �޽��� ����(flag)
// ������ �޽��� ũ��(size)
// ������ �޽��� ������(Data)�� �Է� �޾�
// ��뿡�� �����͸� ������ �Լ�
int sendToServer(SOCKET sock, int flag, int size, char* Data) {
	static HANDLE hSender;
	senderArgument arg;
	arg.Data = Data;
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