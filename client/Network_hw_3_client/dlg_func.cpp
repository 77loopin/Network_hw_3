#include "dlg_func.h"

// Main DialogBox 함수
BOOL CALLBACK main_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	static char buf[MAXMSG + 1];	// 메시지를 저장하는 임시 버퍼
	static char nick[MAXNICK + 1];	// 닉네임을 저장하는 임시 버퍼
	UserList* ptr;					// 사용자 리스트 포인터
	msgData msg;					// 메시지를 저장하는 임시 버퍼 구조체
	struct tm* timeinfo;			// 시간 정보를 저장하는 tm 구조체
	int index;						// List Box의 사용자 index를 저장하는 임시 변수

	switch (uMsg) {
	case WM_INITDIALOG : // DialogBox 초기화시

		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)setup_DlgProc); // setup dialog box 호출
		
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1); // 메시지 출력창
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2); // 메시지 입력창
		hList = GetDlgItem(hDlg, IDC_LIST2); // 접속자 리스트

		hMenu = GetDlgItem(hDlg, IDR_MENU1); // 메뉴 핸들

		SendMessage(hEdit2, EM_SETLIMITTEXT, MAXMSG, 0); // MSG의 최대 길이 설정

		// ListBox에 전체 사용자와 해당 사용자의 닉네임을 출력한다.
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"전체 사용자");
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)userList.Nick);
		
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK : // 확인 버튼을 누른 경우
			EnableWindow(hDlg, FALSE);
			GetDlgItemText(hDlg, IDC_EDIT2, buf, MAXMSG + 1); // 메시지 입력창에 입력된 데이터를 buf에 저장한다.
			// hDlg : 대화상자 핸들, IDC_EDIT2 : 컨트롤ID, buf : 버퍼의 시작 주소, 버퍼 크기
			if (strlen(buf) == 0) { // 입력 내용이 없으면 Focus를 수정한 후 바로 종료
				SetFocus(hEdit2);
				EnableWindow(hDlg, TRUE);
				return TRUE;
			}

			// 현재 시간 정보를 가져온다.
			timeinfo = gettime();
			
			// 메시지를 보낸 후에 입력 에디터 컨트롤을 비운다.
			SendMessage(hEdit2, EM_SETSEL, 0, -1); // 입력창에 있는 문자열을 모두 선택 상태로 만든다.
			SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)""); // 입력창을 비운다.

			if (setup.connectFlag == CONNECT_INIT) { // 현재 미 접속 상태면 서버 설정 경고를 띄운다.
				MessageBox(hDlg, "서버를 설정을 하세요.\n\n[설정] -> [접속 설정]\n\n", "경고", MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				SetFocus(hEdit2); // 입력 에디터 컨트롤에 다시 Focus시킨다.
				return FALSE;
			}

			else if ( setup.connectFlag == CONNECT_SVR ) { // 현재 서버만 접속된 상태라면 닉네임 설정으로 채팅방 접속 경고를 띄운다.
				MessageBox(hDlg,"닉네임을 설정해서 채팅방에 접속하세요.\n\n[설정] -> [닉네임 설정]\n\n","경고",MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				SetFocus(hEdit2); // 입력 에디터 컨트롤에 다시 Focus시킨다.
				return FALSE;
			}

			else if ( setup.connectFlag == CONNECT_CHAT ) { // 채팅 상태라면

				strncpy(msg.msg, buf, strlen(buf) + 1);
				strncpy(msg.nick, setup.myNick, strlen(setup.myNick) +1);
				index = SendMessage(hList, LB_GETCURSEL, 0, 0); // 현재 List Box의 커서 위치를 가져온다.
				if (index == 0 || index == 1 || index == LB_ERR) { // 전체 메시지인 경우 ( index가 0,1 이거나 선택된 것이 없으면 )
					sendToServer(serverSocket, CLT_MSG_ALLMSG, sizeof(msgData), (char*)&msg); // 서버로 전체 메시지 전송
					DisplayText("[%02d:%02d | %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, buf); // 보낸 메시지를 화면에 출력
				}
				else { // 귓속말인 경우
					SendMessage(hList, LB_GETTEXT, index, (LPARAM)nick); // 해당 index의 사용자 닉네임을 가져온다.
					ptr = searchNick(nick); // 닉네임에 해당하는 사용자 정보를 가져온다.
					if (ptr != NULL) {
						strncpy(msg.nick, nick, strlen(nick) + 1);
						sendToServer(serverSocket, CLT_MSG_WHISPER, sizeof(msgData), (char*)&msg); // 귓속말 메시지를 서버로 전송
						DisplayText("[%02d:%02d | 귓속말 %s -> %s] : %s\n", timeinfo->tm_hour, timeinfo->tm_min, setup.myNick, nick , msg.msg); // 화면에 출력
					}
				}
			}
			
			EnableWindow(hDlg, TRUE);
			SetFocus(hEdit2); // 입력 에디터 컨트롤에 다시 Focus시킨다.
			return TRUE;
		case IDCANCEL: // 취소 버튼을 누른 경우
			// 프로그램 종료 ( 소켓 종료 후 종료 )
			EndDialog(hDlg, IDCANCEL); // 대화상자 종료, 즉 프로그램 종료
			return TRUE;
		case ID_40004 : // 서버 설정을 실행하면 setup Dialog Box 호출
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)setup_DlgProc);
			return TRUE;
		case ID_40005 : // 닉네임 설정을 실행하면 Nickname setup Dialog Box 호출
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), NULL, (DLGPROC)nickSetup_DlgProc);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}


// Setup Dialog Box 함수
BOOL CALLBACK setup_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	char ip[40]; // 입력받은 ip를 저장하는 임시 변수
	char port[6]; // 입력받은 port를 저장하는 임시 변수
	char Nick[MAXNICK + 1]; // 닉네임을 저장하는 임시 변수
	int tempPort; // port값을 저장하는 임시 변수
	int iplen; // ip 문자열의 길이를 저장하는 변수
	msgHeader header; // 메시지의 헤더 구조체

	switch (uMsg) {
	case WM_INITDIALOG: // DialogBox를 처음 실행 됐을 때 초기화
		hIp = GetDlgItem(hDlg, IDC_IPADDRESS1); // IDC_IPADDRESS1의 핸들을 가져온다.
		hPort = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1의 핸들을 가져온다.
		SendMessage(hPort, EM_SETLIMITTEXT, 5, 0); // port의 최대 입력 크기를 5로 설정한다. (최대 65535이므로)
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // 확인 버튼을 누른 경우 호출

			EnableWindow(hDlg, FALSE);
			
			iplen = GetWindowTextLength(hIp); // IP의 길이를 리턴
			GetDlgItemText(hDlg, IDC_IPADDRESS1, ip, iplen + 1); // IP를 입력 받음
			GetDlgItemText(hDlg, IDC_EDIT1, port, 6); // Port를 입력 받음

			if (!check_ip(ip, iplen)) {
				MessageBox(hDlg, "유효한 IP가 아닙니다.\n서버 IP를 다시 확인해주세요.\n", "경고", MB_ICONERROR);
				SetFocus(hIp);
				EnableWindow(hDlg, TRUE);
				return FALSE;
			}

			if (!check_port(port)) { // Port가 유효한지 확인
									 // 유효한 Port가 아닌 경우 경고 메시지 호출 후 다시 입력 받도록 함
				MessageBox(hDlg, "PORT를 알 수 없습니다.\nPORT를 올바르게 입력하세요.\n\n[PORT는 1024 ~ 65535까지 가능]", "경고", MB_ICONERROR);
				SetFocus(hPort);
				EnableWindow(hDlg, TRUE);
				return FALSE;	
			}
			
			tempPort = atoi(port); // 입력받은 포트를 숫자로 변환

			// setup 구조체에 서버 설정을 저장
			strncpy(setup.serverIP, ip, strlen(ip) + 1);
			setup.serverPort = tempPort;
			// 현재 접속중인 상태에서 서버 설정을 변경하는 경우
			if (setup.connectFlag == CONNECT_CHAT) {
				setup.connectFlag = CONNECT_INIT; // 서버 설정을 미 접속 상태로 변경
				TerminateThread(hMainRecv, 1); // Receiver Thread 강제 종료
				TerminateThread(hSender, 1); // Sender Thread 강제 종료
				TerminateThread(hMainThread, 1); // Main Thread 강제 종료
				closesocket(serverSocket); // 소켓 종료

				MessageBox(hDlg, "새로운 서버와 접속합니다.\n기존 서버는 종료됩니다.\n", "알림", MB_ICONINFORMATION);
			}

			// Main Thread를 실행한다.
			hMainThread = (HANDLE)_beginthreadex(NULL, 0, MainThread, NULL, 0, NULL); // Create Main Thread 
			if (hMainThread == NULL) {
				MessageBox(NULL, "클라이언트 시작을 실패했습니다.\n프로그램을 종료합니다.", "스레드 생성 실패", MB_ICONERROR);
				exit(-1);
			}
			
			EndDialog(hDlg, IDCANCEL); // configuration dialog box finish
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), NULL, (DLGPROC)nickSetup_DlgProc);
			return TRUE;
		case IDCANCEL:
			if (setup.connectFlag == CONNECT_INIT) { // 최초 접속 시 setting값이 없으면 프로그램 강제 종료
				MessageBox(hDlg, "접속한 채팅 방이 없습니다.\n프로그램을 종료합니다.", "실패", MB_ICONERROR);
				exit(0);
			}
			EndDialog(hDlg, IDCANCEL); // 설정을 변경하지 않으면 대화상자 종료
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}


// Nickname Setup Dialog Box 함수
BOOL CALLBACK nickSetup_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char newNick[MAXNICK + 1];	// 새로운 대화명을 저장하는 임시 변수
	HWND hEdit;					// 닉네임 입력을 받는 edit control 핸들
	msgData msg;				// 메시지 데이터를 저장하는 msgData 구조체

	switch (uMsg) {
	case WM_INITDIALOG: // 프로그램을 처음 실행 했을 때 초기화
		hEdit = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1의 핸들을 가져온다.
		SendMessage(hEdit, EM_SETLIMITTEXT, MAXNICK, 0); // 대화명의 최대 입력 크기를 설정한다.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // 확인 버튼을 누른 경우 호출
			EnableWindow(hDlg, FALSE);
			GetDlgItemText(hDlg, IDC_EDIT1, newNick, MAXNICK + 1); // IDC_EDIT1의 문자열을 newNick에 저장

			if (!check_nick(newNick)) { // 새로 입력된 대화명이 유효한지 확인
				MessageBox(hDlg, "NickName을 입력하세요.\n", "경고", MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				return TRUE;
			}

			if (setup.connectFlag == CONNECT_INIT) { // 현재 미접속 상태면 경고 메시지
				MessageBox(hDlg, "서버를 설정을 하세요.\n\n[설정] -> [접속 설정]\n\n", "경고", MB_ICONERROR);
				EndDialog(hDlg, IDOK); // 대화상자 종료
				return TRUE;
			}

			else if (setup.connectFlag == CONNECT_SVR) { // 서버만 접속된 상태라면 최초 접속 (CLT_MSG_NEWUSER)
				sendToServer(serverSocket, CLT_MSG_NEWUSER, strlen(newNick) + 1, newNick); // nickname 설정
			}

			else if (setup.connectFlag == CONNECT_CHAT) { // 채팅중에 닉네임만 변경하면 닉네임 변경 메시지 (CLT_MSG_CHGNICK)
				sendToServer(serverSocket, CLT_MSG_CHGNICK, strlen(newNick) + 1, newNick); // nickname 변경
			}

			EnableWindow(hDlg, TRUE);
			EndDialog(hDlg, IDOK); // 대화상자 종료
			return TRUE;
		case IDCANCEL: // 취소 버튼을 누른 경우 호출
			// 채팅방에 접속하지 않은 상태에서 취소를 누르면 경고 메시지 출력
			if (setup.connectFlag == CONNECT_SVR || setup.connectFlag == CONNECT_INIT) {
				MessageBox(hDlg, "NickName을 입력하지 않으면\n\n채팅방에 접속할 수 없습니다.\n\n", "경고", MB_ICONERROR);
			}
			EnableWindow(hDlg, TRUE);
			EndDialog(hDlg, IDCANCEL); // 대화상자 종료
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}


// 메시지 출력을 위한 에디터 컨트롤 함수
void DisplayText(char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);

	char cbuf[MAXMSG + MAXNICK + 10];
	vsprintf(cbuf, fmt, arg); // 입력된 문자열을 완성해서 cbuf에 저장한다.
	int nLength = GetWindowTextLength(hEdit1); // 해당 문자열의 길이를 반환한다.
	SendMessage(hEdit1, EM_SETSEL, nLength, nLength); // 현재 hEdit1의 길이 뒤로 포인터를 이동 한 후
	SendMessage(hEdit1, EM_REPLACESEL, FALSE, (LPARAM)cbuf); // 현재 위치에 cbuf 내용을 출력한다.

	va_end(arg);
}



// ip가 유효한 서버 ip인지 확인하는 함수
BOOL check_ip(char* ip_addr, int len) {
	char tempString[40];
	char *ptr;
	int t = 0;
	strncpy(tempString, ip_addr, len + 1);

	ptr = strtok(tempString, ".");

	t = atoi(ptr);
	if (t >= 0 && t <= 127) { // A Class check
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
	else if (t >= 128 && t <= 191) { // B Class check
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
	else if (t >= 192 && t <= 223) { // C Class check
		strtok(NULL, ".");
		strtok(NULL, ".");
		ptr = strtok(NULL, ".");
		t = atoi(ptr);
		if (t == 0 || t == 255) // IP주소의 마지막값이 0 또는 255이면 주소값 오류
			return FALSE;
	}
	else {
		return FALSE;
	}
	// 모든 check를 확인하면 유효한 IP로 TRUE 리턴
	return TRUE;
}

// 입력된 port 번호가 유효한지 확인하는 함수
BOOL check_port(char* port) {
	int t;
	for (int i = 0; i < strlen(port); i++) {
		if (port[i] > 0x39 || port[i] < 0x30) // 포트번호가 숫자값이 아니면 오류
			return FALSE;
	}

	t = atoi(port);

	if (t > 65535 || t < 1024) // 포트번호가 포트 숫자 범위 밖이거나 1024보다 작으면 오류
		return FALSE;
	return TRUE;
}

// 입력된 대화명이 유효한지 확인하는 함수
BOOL check_nick(char* nickname) {
	if (strlen(nickname) == 0) // 대화명 길이가 0이면 오류
		return FALSE;
	if (nickname[0] == '\n' || nickname[0] == '\r') // 대화명 첫 글자가 개행문자면 오류
		return FALSE;
	return TRUE;
}


// ListBox에 사용자 목록을 갱신하는 함수
void updateUserList() {
	UserList* temp;
	int count = 0;
	
	// ListBox의 모든 목록을 삭제
	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	temp = userListHeader;

	// 먼저 "전체 사용자"를 설정
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"전체 사용자");
	
	// 사용자 리스트에 모든 사용자를 출력
	while (temp != NULL) {
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(temp->Nick));
		temp = temp->next;
	}
	
}

// 닉네임 변경 함수
int changeNick(char* oldNick, char* newNick) {
	UserList* ptr;
	int index;

	// oldNick을 ListBox에서 검색, 사용자 리스트에서 검색
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

	// 사용자 리스트의 닉네임 변경
	strncpy(ptr->Nick, newNick, strlen(newNick) + 1); // UserList update

	// ListBox의 해당 index에 있는 닉네임을 삭제하고 새로운 닉네임으로 다시 설정
	SendMessage(hList, LB_DELETESTRING, index, 0); // List Box update
	SendMessage(hList, LB_INSERTSTRING, index, (LPARAM)newNick);

	return 0;
}
