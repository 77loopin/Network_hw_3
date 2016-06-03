#include "dlg_func.h"

HWND hEdit1;
HWND hEdit2;
HWND hEdit3;

HWND hMenu;

HWND hIp;
HWND hPort;
HWND hNick;

HANDLE hMainThread;

setupInfo setup;

HWND hMainDlg;
HWND hSetupDlg;
HWND hNickDlg;

	
BOOL CALLBACK main_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	static char buf[MAXMSG + 1];
	msgData msg;
	struct tm* timeinfo;
	BOOL retval;
	

	switch (uMsg) {
	case WM_INITDIALOG :
		
		setup.connectFlag = 0; // �� ���� ����
		
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)setup_DlgProc);

		hMainDlg = hDlg; // MainDialog �ڵ� ���������� ����
		
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1); // �޽��� ���â
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2); // �޽��� �Է�â
		hEdit3 = GetDlgItem(hDlg, IDC_EDIT3); // ����� ��Ȳ ( ���� ���� )
		hMenu = GetDlgItem(hDlg, IDR_MENU1);

		/*
		if (setup.connectFlag == 1) {
			display_MB("�г����� �����ؾ��մϴ�.\n\n�޴����� �г����� �������ּ���.\n\n");
			EnableWindow(hEdit3, FALSE);
		}
		*/
		SendMessage(hEdit3, EM_SETLIMITTEXT, MAXMSG, 0);
		
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

			if (setup.connectFlag == 0) {
				MessageBox(hDlg, "������ ������ �ϼ���.\n\n[����] -> [���� ����]\n\n", "���", MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				SetFocus(hEdit2); // �Է� ������ ��Ʈ�ѿ� �ٽ� Focus��Ų��.
				return FALSE;
			}

			else if ( setup.connectFlag == 1 ) {
				MessageBox(hDlg,"�г����� �����ؼ� ä�ù濡 �����ϼ���.\n\n[����] -> [�г��� ����]\n\n","���",MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				SetFocus(hEdit2); // �Է� ������ ��Ʈ�ѿ� �ٽ� Focus��Ų��.
				return FALSE;
			}

			else if ( setup.connectFlag == 2 ) {

				strncpy(msg.msg, buf, strlen(buf) + 1);
				strncpy(msg.nick, setup.myNick, strlen(setup.myNick) +1);
				sendToServer(serverSocket, 4, sizeof(msgData), (char*)&msg); // ��ü �޽���
				DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, buf);
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
		//SendMessage(hNick, EM_SETLIMITTEXT, MAXNICK, 0); // ��ȭ�� �Է��� �ִ� ũ�⸦ �����Ѵ�.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // Ȯ�� ��ư�� ���� ��� ȣ��

			EnableWindow(hDlg, FALSE);
			
			iplen = GetWindowTextLength(hIp); // IP�� ���̸� ����
			GetDlgItemText(hDlg, IDC_IPADDRESS1, ip, iplen + 1); // IP�� �Է� ����
			GetDlgItemText(hDlg, IDC_EDIT1, port, 6); // Port�� �Է� ����
			tempPort = atoi(port);
			//GetDlgItemText(hDlg, IDC_EDIT2, Nick, MAXNICK + 1); // ��ȭ���� �Է� ����

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

			// �ӽ÷� 127.0.0.1�� ���� �����ϵ��� ���� ( ���� �����ϸ� �ٸ� ���� IP�ε� ���� ������ )
			if (strncmp(ip, "127.0.0.1", iplen)) {
				MessageBox(hDlg, "��ȿ�� ���� IP�� �ƴմϴ�.\n\n���� IP�� Ȯ�����ֽñ� �ٶ��ϴ�.\n\n[�ӽ� ���� IP : 127.0.0.1]\n\n","���� IP ����",MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				return FALSE;
			}

			if (setup.connectFlag == 2) {
				MessageBox(NULL, "���ο� ������ ������ �õ��մϴ�.\n\n���ο� ���� ������ ���� ���� ������ �����մϴ�.\n\n", "���� ����", MB_ICONINFORMATION);
				//setup.connectFlag = 0;
				closesocket(serverSocket);
			}

			strncpy(setup.serverIP, ip, strlen(ip) + 1);
			setup.serverPort = tempPort;
			setup.connectFlag = 0;
			
			hMainThread = (HANDLE)_beginthreadex(NULL, 0, MainThread, NULL, 0, NULL);
			if (hMainThread == NULL) {
				MessageBox(hDlg, "Ŭ���̾�Ʈ ������ �����߽��ϴ�.\n���α׷��� �����մϴ�.", "������ ���� ����", MB_ICONERROR);
				EndDialog(hMainDlg, IDOK);
				return FALSE;
			}
			
			CloseHandle(hMainThread);
			
			EndDialog(hDlg, IDOK); // configuration dialog box finish

			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), NULL, (DLGPROC)nickSetup_DlgProc);

			return TRUE;
		case IDCANCEL:
			if (setup.connectFlag == 0) { // ���� ���� �� setting���� ������ ���α׷� ���� ����
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
	struct tm* timeinfo;

	switch (uMsg) {
	case WM_INITDIALOG: // ���α׷��� ó�� ���� ���� �� �ʱ�ȭ
		hEdit = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1�� �ڵ��� �����´�.
		SendMessage(hEdit, EM_SETLIMITTEXT, MAXNICK, 0); // ��ȭ���� �ִ� �Է� ũ�⸦ �����Ѵ�.
		//MessageBox(hDlg, "NickName�� �Է��ϼ���.\n", "Information", MB_ICONINFORMATION);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // Ȯ�� ��ư�� ���� ��� ȣ��
			EnableWindow(hDlg, FALSE);
			GetDlgItemText(hDlg, IDC_EDIT1, newNick, MAXNICK + 1); // IDC_EDIT1�� ���ڿ��� newNick�� ����

			if (strlen(newNick) == 0 || !check_nick(newNick)) { // ���� �Էµ� ��ȭ���� ��ȿ���� Ȯ��
				MessageBox(hDlg, "NickName�� �Է��ϼ���.\n", "���", MB_ICONERROR);
				SetFocus(hEdit);
				EnableWindow(hDlg, TRUE);
				return FALSE;
			}

			sendToServer(serverSocket, 1, strlen(newNick)+1, newNick); // nickname ����

			// ��ȭ���� ���� �ߴٴ� ���� ȭ�鿡 ���
			//MessageBox(hDlg, "�г����� �����߽��ϴ�.\n", "Ȯ��", MB_OK);
			
			EnableWindow(hDlg, TRUE);
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� ����
			return TRUE;
		case IDCANCEL: // ��� ��ư�� ���� ��� ȣ��
			if (setup.connectFlag == 1 || setup.connectFlag == 0) {
				MessageBox(hDlg, "NickName�� �Է����� ������\n\nä�ù濡 ������ �� �����ϴ�.\n\n", "���", MB_ICONERROR);
			}
			EnableWindow(hDlg, TRUE);
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� ����
			return TRUE;
		}
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
