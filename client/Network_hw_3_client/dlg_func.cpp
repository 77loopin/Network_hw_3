#include "dlg_func.h"

// Main DialogBox
BOOL CALLBACK main_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	static char buf[MAXMSG + 1];
	static char nick[MAXNICK + 1];
	UserList* ptr;
	msgData msg;
	struct tm* timeinfo;
	BOOL retval;
	int index;

	switch (uMsg) {
	case WM_INITDIALOG :

		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)setup_DlgProc); // setup dialog box ȣ��
		
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1); // �޽��� ���â
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2); // �޽��� �Է�â
		hList = GetDlgItem(hDlg, IDC_LIST2); // ������ ����Ʈ

		hMenu = GetDlgItem(hDlg, IDR_MENU1); // �޴� �ڵ�

		SendMessage(hEdit2, EM_SETLIMITTEXT, MAXMSG, 0);

		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"��ü �����");
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)userList.Nick);
		
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK : // Ȯ�� ��ư�� ���� ���
			EnableWindow(hDlg, FALSE);
			GetDlgItemText(hDlg, IDC_EDIT2, buf, MAXMSG + 1); // �޽��� �Է�â�� �Էµ� �����͸� buf�� �����Ѵ�.
			// hDlg : ��ȭ���� �ڵ�, IDC_EDIT2 : ��Ʈ��ID, buf : ������ ���� �ּ�, ���� ũ��
			if (strlen(buf) == 0) { // �Է� ������ ������ Focus�� ������ �� �ٷ� ����
				SetFocus(hEdit2);
				EnableWindow(hDlg, TRUE);
				return TRUE;
			}

			timeinfo = gettime();
			
			// �޽����� ���� �Ŀ� �Է� ������ ��Ʈ���� ����.
			SendMessage(hEdit2, EM_SETSEL, 0, -1); // �Է�â�� �ִ� ���ڿ��� ��� ���� ���·� �����.
			SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)"");

			if (setup.connectFlag == CONNECT_INIT) {
				MessageBox(hDlg, "������ ������ �ϼ���.\n\n[����] -> [���� ����]\n\n", "���", MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				SetFocus(hEdit2); // �Է� ������ ��Ʈ�ѿ� �ٽ� Focus��Ų��.
				return FALSE;
			}

			else if ( setup.connectFlag == CONNECT_SVR ) {
				MessageBox(hDlg,"�г����� �����ؼ� ä�ù濡 �����ϼ���.\n\n[����] -> [�г��� ����]\n\n","���",MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				SetFocus(hEdit2); // �Է� ������ ��Ʈ�ѿ� �ٽ� Focus��Ų��.
				return FALSE;
			}

			else if ( setup.connectFlag == CONNECT_CHAT ) {

				strncpy(msg.msg, buf, strlen(buf) + 1);
				strncpy(msg.nick, setup.myNick, strlen(setup.myNick) +1);
				index = SendMessage(hList, LB_GETCURSEL, 0, 0);
				if (index == 0 || index == 1 || index == LB_ERR) { // ��ü �޽����� ���
					sendToServer(serverSocket, CLT_MSG_ALLMSG, sizeof(msgData), (char*)&msg); // ��ü �޽���
					DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, buf);
				}
				else { // �ӼӸ��� ���
					SendMessage(hList, LB_GETTEXT, index, (LPARAM)nick);
					ptr = searchNick(nick);
					if (ptr != NULL) {
						strncpy(msg.nick, nick, strlen(nick) + 1);
						sendToServer(serverSocket, CLT_MSG_WHISPER, sizeof(msgData), (char*)&msg); // �ӼӸ�
						DisplayText("[%02d:%02d | �ӼӸ� %s -> %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, nick , msg.msg);
					}
				}
			}
			
			EnableWindow(hDlg, TRUE);
			SetFocus(hEdit2); // �Է� ������ ��Ʈ�ѿ� �ٽ� Focus��Ų��.
			return TRUE;
		case IDCANCEL: // ��� ��ư�� ���� ���
			// ���α׷� ���� ( ���� ���� �� ���� )
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� ����, �� ���α׷� ����
			return TRUE;
		case ID_40004 :
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)setup_DlgProc);
			return TRUE;
		case ID_40005 :
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), NULL, (DLGPROC)nickSetup_DlgProc);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}


BOOL CALLBACK setup_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	char ip[40]; // �Է¹��� ip�� �����ϴ� �ӽ� ����
	char port[6]; // �Է¹��� port�� �����ϴ� �ӽ� ����
	char Nick[MAXNICK + 1];
	int tempPort;
	int iplen; // ip ���ڿ��� ���̸� �����ϴ� ����
	msgHeader header;
	SOCKET tempSocket;

	switch (uMsg) {
	case WM_INITDIALOG: // DialogBox�� ó�� ���� ���� �� �ʱ�ȭ
		hIp = GetDlgItem(hDlg, IDC_IPADDRESS1); // IDC_IPADDRESS1�� �ڵ��� �����´�.
		hPort = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1�� �ڵ��� �����´�.
		SendMessage(hPort, EM_SETLIMITTEXT, 5, 0); // port�� �ִ� �Է� ũ�⸦ 5�� �����Ѵ�. (�ִ� 65535�̹Ƿ�)
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // Ȯ�� ��ư�� ���� ��� ȣ��

			EnableWindow(hDlg, FALSE);
			
			iplen = GetWindowTextLength(hIp); // IP�� ���̸� ����
			GetDlgItemText(hDlg, IDC_IPADDRESS1, ip, iplen + 1); // IP�� �Է� ����
			GetDlgItemText(hDlg, IDC_EDIT1, port, 6); // Port�� �Է� ����
			tempPort = atoi(port);

			if (!check_ip(ip, iplen)) {
				MessageBox(hDlg, "��ȿ�� IP�� �ƴմϴ�.\n���� IP�� �ٽ� Ȯ�����ּ���.\n", "���", MB_ICONERROR);
				SetFocus(hIp);
				EnableWindow(hDlg, TRUE);
				return FALSE;
			}

			if (!check_port(port)) { // Port�� ��ȿ���� Ȯ��
									 // ��ȿ�� Port�� �ƴ� ��� ��� �޽��� ȣ�� �� �ٽ� �Է� �޵��� ��
				MessageBox(hDlg, "PORT�� �� �� �����ϴ�.\nPORT�� �ùٸ��� �Է��ϼ���.\n\n[PORT�� 1024 ~ 65535���� ����]", "���", MB_ICONERROR);
				SetFocus(hPort);
				EnableWindow(hDlg, TRUE);
				return FALSE;	
			}

			strncpy(setup.serverIP, ip, strlen(ip) + 1);
			setup.serverPort = tempPort;

			if (setup.connectFlag == CONNECT_CHAT) {
				setup.connectFlag = CONNECT_INIT;
				TerminateThread(hMainRecv, 1);
				TerminateThread(hSender, 1);
				TerminateThread(hMainThread, 1);
				closesocket(serverSocket);

				MessageBox(hDlg, "���ο� ������ �����մϴ�.\n���� ������ ����˴ϴ�.\n", "�˸�", MB_ICONINFORMATION);
			}

			hMainThread = (HANDLE)_beginthreadex(NULL, 0, MainThread, NULL, 0, NULL); // Create Main Thread 
			if (hMainThread == NULL) {
				MessageBox(NULL, "Ŭ���̾�Ʈ ������ �����߽��ϴ�.\n���α׷��� �����մϴ�.", "������ ���� ����", MB_ICONERROR);
				exit(-1);
			}
			
			EndDialog(hDlg, IDCANCEL); // configuration dialog box finish
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), NULL, (DLGPROC)nickSetup_DlgProc);
			return TRUE;
		case IDCANCEL:
			if (setup.connectFlag == CONNECT_INIT) { // ���� ���� �� setting���� ������ ���α׷� ���� ����
				MessageBox(hDlg, "������ ä�� ���� �����ϴ�.\n���α׷��� �����մϴ�.", "����", MB_ICONERROR);
				exit(0);
			}
			EndDialog(hDlg, IDCANCEL); // ������ �������� ������ ��ȭ���� ����
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL CALLBACK nickSetup_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char newNick[MAXNICK + 1]; // ���ο� ��ȭ���� �����ϴ� �ӽ� ����
	HWND hEdit;
	msgData msg;
	struct tm* timeinfo;

	switch (uMsg) {
	case WM_INITDIALOG: // ���α׷��� ó�� ���� ���� �� �ʱ�ȭ
		hEdit = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1�� �ڵ��� �����´�.
		SendMessage(hEdit, EM_SETLIMITTEXT, MAXNICK, 0); // ��ȭ���� �ִ� �Է� ũ�⸦ �����Ѵ�.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // Ȯ�� ��ư�� ���� ��� ȣ��
			EnableWindow(hDlg, FALSE);
			GetDlgItemText(hDlg, IDC_EDIT1, newNick, MAXNICK + 1); // IDC_EDIT1�� ���ڿ��� newNick�� ����

			if (!check_nick(newNick)) { // ���� �Էµ� ��ȭ���� ��ȿ���� Ȯ��
				MessageBox(hDlg, "NickName�� �Է��ϼ���.\n", "���", MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				return TRUE;
			}

			if (setup.connectFlag == CONNECT_INIT) {
				MessageBox(hDlg, "������ ������ �ϼ���.\n\n[����] -> [���� ����]\n\n", "���", MB_ICONERROR);
				EndDialog(hDlg, IDOK); // ��ȭ���� ����
				return TRUE;
			}

			else if (setup.connectFlag == CONNECT_SVR) {
				sendToServer(serverSocket, CLT_MSG_NEWUSER, strlen(newNick) + 1, newNick); // nickname ����
			}

			else if (setup.connectFlag == CONNECT_CHAT) {
				sendToServer(serverSocket, CLT_MSG_CHGNICK, strlen(newNick) + 1, newNick); // nickname ����
			}

			EnableWindow(hDlg, TRUE);
			EndDialog(hDlg, IDOK); // ��ȭ���� ����
			return TRUE;
		case IDCANCEL: // ��� ��ư�� ���� ��� ȣ��
			if (setup.connectFlag == CONNECT_SVR || setup.connectFlag == CONNECT_INIT) {
				MessageBox(hDlg, "NickName�� �Է����� ������\n\nä�ù濡 ������ �� �����ϴ�.\n\n", "���", MB_ICONERROR);
			}
			EnableWindow(hDlg, TRUE);
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� ����
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}


// �޽��� ����� ���� ������ ��Ʈ�� �Լ�
void DisplayText(char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);

	char cbuf[MAXMSG + MAXNICK + 10];
	vsprintf(cbuf, fmt, arg); // �Էµ� ���ڿ��� �ϼ��ؼ� cbuf�� �����Ѵ�.
	int nLength = GetWindowTextLength(hEdit1); // �ش� ���ڿ��� ���̸� ��ȯ�Ѵ�.
	SendMessage(hEdit1, EM_SETSEL, nLength, nLength); // ���� hEdit1�� ���� �ڷ� �����͸� �̵� �� ��
	SendMessage(hEdit1, EM_REPLACESEL, FALSE, (LPARAM)cbuf); // ���� ��ġ�� cbuf ������ ����Ѵ�.

	va_end(arg);
}



// ip�� ��ȿ�� ���� ip���� Ȯ���ϴ� �Լ�
BOOL check_ip(char* ip_addr, int len) {
	char tempString[40];
	char *ptr;
	int t = 0;
	strncpy(tempString, ip_addr, len + 1);

	ptr = strtok(tempString, ".");

	t = atoi(ptr);
	if (t >= 0 && t <= 127) { // A Class
		ptr = strtok(NULL, ".");
		t = atoi(ptr);
		if (t == 0) {
			ptr = strtok(NULL, ".");
			t = atoi(ptr);
			if (t == 0) {
				ptr = strtok(NULL, ".");
				t = atoi(ptr);
				if (t == 0) {
					return FALSE;
				}
			}
		}
		else if (t == 255) {
			ptr = strtok(NULL, ".");
			t = atoi(ptr);
			if (t == 255) {
				ptr = strtok(NULL, ".");
				t = atoi(ptr);
				if (t == 255) {
					return FALSE;
				}
			}
		}
	}
	else if (t >= 128 && t <= 191) { // B Class
		strtok(NULL, ".");
		ptr = strtok(NULL, ".");
		t = atoi(ptr);
		if (t == 0) {
			ptr = strtok(NULL, ".");
			t = atoi(ptr);
			if (t == 0) {
				return FALSE;
			}
		}
		else if (t == 255) {
			ptr = strtok(NULL, ".");
			t = atoi(ptr);
			if (t == 255) {
				return FALSE;
			}
		}
	}
	else if (t >= 192 && t <= 223) { // C Class
		strtok(NULL, ".");
		strtok(NULL, ".");
		ptr = strtok(NULL, ".");
		t = atoi(ptr);
		if (t == 0 || t == 255) // IP�ּ��� ���������� 0 �Ǵ� 255�̸� �ּҰ� ����
			return FALSE;
	}
	else {
		return FALSE;
	}

	return TRUE;
}

// �Էµ� port ��ȣ�� ��ȿ���� Ȯ���ϴ� �Լ�
BOOL check_port(char* port) {
	int t;
	for (int i = 0; i < strlen(port); i++) {
		if (port[i] > 0x39 || port[i] < 0x30) // ��Ʈ��ȣ�� ���ڰ��� �ƴϸ� ����
			return FALSE;
	}

	t = atoi(port);

	if (t > 65535 || t < 1024) // ��Ʈ��ȣ�� ��Ʈ ���� ���� ���̰ų� 1024���� ������ ����
		return FALSE;
	return TRUE;
}

// �Էµ� ��ȭ���� ��ȿ���� Ȯ���ϴ� �Լ�
BOOL check_nick(char* nickname) {
	if (strlen(nickname) == 0) // ��ȭ�� ���̰� 0�̸� ����
		return FALSE;
	if (nickname[0] == '\n' || nickname[0] == '\r') // ��ȭ�� ù ���ڰ� ���๮�ڸ� ����
		return FALSE;
	return TRUE;
}



void updateUserList() {
	UserList* temp;
	int count = 0;
	
	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	temp = userListHeader;
	
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"��ü �����");
	
	while (temp != NULL) {
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(temp->Nick));
		temp = temp->next;
	}
	
}

int changeNick(char* oldNick, char* newNick) {
	UserList* ptr;
	int index;

	index = SendMessage(hList, LB_FINDSTRINGEXACT, 0, (LPARAM)(oldNick));
	ptr = searchNick(oldNick);

	if (ptr == NULL) {
		MessageBox(NULL, "User Search Error", "Error", MB_ICONERROR);
		return -1;
	}

	if (index == LB_ERR) {
		MessageBox(NULL, "Index Search Error", "Error", MB_ICONERROR);
		return -1;
	}

	strncpy(ptr->Nick, newNick, strlen(newNick) + 1); // UserList update

	SendMessage(hList, LB_DELETESTRING, index, 0); // List Box update
	SendMessage(hList, LB_INSERTSTRING, index, (LPARAM)newNick);

	return 0;
}
