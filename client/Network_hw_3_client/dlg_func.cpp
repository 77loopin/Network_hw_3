#include "dlg_func.h"

HWND hEdit1;
HWND hEdit2;
HWND hEdit3;

HWND hMenu;

HWND hIp;
HWND hPort;
HWND hNick;

setupInfo setup;


BOOL CALLBACK main_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	static char buf[MAXMSG + 1];
	BOOL retval;
	

	switch (uMsg) {
	case WM_INITDIALOG :
		
		setup.connectFlag = 0; // 미 접속 상태
		
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, setup_DlgProc);
		
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), NULL, nickSetup_DlgProc);
		
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1); // 메시지 출력창
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2); // 메시지 입력창
		hEdit3 = GetDlgItem(hDlg, IDC_EDIT3); // 사용자 현황 ( 수정 예정 )
		hMenu = GetDlgItem(hDlg, IDR_MENU1);

		if (setup.connectFlag == 1) {
			display_MB("닉네임이 설정해야합니다.\n\n메뉴에서 닉네임을 설정해주세요.\n\n");
			EnableWindow(hEdit3, FALSE);

		}
		
		SendMessage(hEdit3, EM_SETLIMITTEXT, MAXMSG, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK : // 확인 버튼을 누른 경우
			EnableWindow(hDlg, FALSE);
			GetDlgItemText(hDlg, IDC_EDIT2, buf, MAXMSG + 1); // 메시지 입력창에 입력된 데이터를 buf에 저장한다.
			// hDlg : 대화상자 핸들, IDC_EDIT2 : 컨트롤ID, buf : 버퍼의 시작 주소, 버퍼 크기
			if (strlen(buf) == 0) { // 입력 내용이 없으면 Focus를 수정한 후 바로 종료
				SetFocus(hEdit2);
				return TRUE;
			}
			// 메시지를 보내면 됨
			// DisplayText("[%s] : %s\r\n", "test", buf);
			
			// 메시지를 보낸 후에 입력 에디터 컨트롤을 비운다.
			SendMessage(hEdit2, EM_SETSEL, 0, -1); // 입력창에 있는 문자열을 모두 선택 상태로 만든다.
			SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)"");
			SetFocus(hEdit2); // 입력 에디터 컨트롤에 다시 Focus시킨다.

			/*
			if ( setup.connectFlag == 2 ) {
			// send Message
			}
			else if ( setup.connectFlag == 1 ) {
			// MessageBox(hDlg,"닉네임을 입력해서 채팅방에 접속하세요.\n\n[설정] -> [닉네임설정]\n\n","경고",MB_ICONERROR);
			}
			*/

			return TRUE;
		case IDCANCEL: // 취소 버튼을 누른 경우
			EndDialog(hDlg, IDCANCEL); // 대화상자 종료, 즉 프로그램 종료
			return TRUE;
		case ID_40004 : //
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, setup_DlgProc);
			return TRUE;
		case ID_40005 :
			
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}


BOOL CALLBACK setup_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	char ip[40]; // 입력받은 ip를 저장하는 임시 변수
	char port[6]; // 입력받은 port를 저장하는 임시 변수
	int tempPort;
	int iplen; // ip 문자열의 길이를 저장하는 변수

	switch (uMsg) {
	case WM_INITDIALOG: // DialogBox를 처음 실행 됐을 때 초기화
		hIp = GetDlgItem(hDlg, IDC_IPADDRESS1); // IDC_IPADDRESS1의 핸들을 가져온다.
		hPort = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1의 핸들을 가져온다.
		SendMessage(hPort, EM_SETLIMITTEXT, 5, 0); // port의 최대 입력 크기를 5로 설정한다. (최대 65535이므로)
		//SendMessage(hNick, EM_SETLIMITTEXT, MAXNICK, 0); // 대화명 입력의 최대 크기를 설정한다.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // 확인 버튼을 누른 경우 호출

			EnableWindow(hDlg, FALSE);
			
			iplen = GetWindowTextLength(hIp); // IP의 길이를 리턴
			GetDlgItemText(hDlg, IDC_IPADDRESS1, ip, iplen + 1); // IP를 입력 받음
			GetDlgItemText(hDlg, IDC_EDIT1, port, 6); // Port를 입력 받음
			//GetDlgItemText(hDlg, IDC_EDIT2, Nick, MAXNICK + 1); // 대화명을 입력 받음

			if (!check_ip(ip, iplen)) {
				display_MB("IP가 Multicast IP가 아닙니다.\nIP를 올바르게 입력하세요.");
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

			tempPort = atoi(port);
			
			display_MB("서버와 연결시도 중입니다.\n\n잠시만 기다려주세요.\n\n");
			
			// 임시로 127.0.0.1만 접속 가능하도록 했음 ( 추후 삭제하면 다른 서버 IP로도 접속 가능함 )
			if (strncmp(ip, "127.0.0.1", iplen)) {
				MessageBox(hDlg, "유효한 서버 IP가 아닙니다.\n\n서버 IP를 확인해주시기 바랍니다.\n\n[임시 서버 IP : 127.0.0.1]\n\n","경고",MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				return FALSE;
			}
			
			// 서버 접속 시도 ( Connection을 맺는다 )
			serverSocket = getConnection(ip, tempPort);
			// Receiver 생성
			
			if (serverSocket == -1) {
				MessageBox(hDlg, "서버와의 연결에 실패했습니다.\n\n서버 설정을 다시 확인해주시기 바랍니다.\n\n", "Error", MB_ICONERROR);
				EnableWindow(hDlg, TRUE);
				return FALSE;
			}

			// 입력된 채팅방 설정값을 setup 구조체에 저장 ( critical section )
			EnterCriticalSection(&cs);
			strncpy(setup.serverIP, ip, 40);
			setup.serverPort = tempPort;
			setup.connectFlag = 1; // 서버와 접속된 상태 ( 채팅방 입장 전 )
			LeaveCriticalSection(&cs);
			// leave critical section

			MessageBox(hDlg, "서버와 접속되었습니다.\n\n","",MB_OK);

			EndDialog(hDlg, IDOK); // configuration dialog box finish
			return TRUE;
		case IDCANCEL:
			if (setup.connectFlag == 0) { // 최초 접속 시 setting값이 없으면 프로그램 강제 종료
				display_MB("접속한 채팅 방이 없습니다.\n프로그램을 종료합니다.");
				exit(0);
			}
			EndDialog(hDlg, IDCANCEL); // 설정을 변경하지 않으면 대화상자 종료
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL CALLBACK nickSetup_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char newNick[MAXNICK + 1]; // 새로운 대화명을 저장하는 임시 변수
	char oldNick[MAXNICK + 1]; // 기존 대화명을 저장하는 임시 변수
	HWND hEdit;
	HANDLE hAdSender;
	struct tm* timeinfo;

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
				SetFocus(hEdit);
				EnableWindow(hDlg, TRUE);
				return FALSE;
			}

			// check nickname 
			// 리턴값으로 확인
			// 리턴값이 에러면 return FALSE;
			// 리턴값이 성공이면 닉네임 설정하고 flag 설정

			// 대화명을 변경 했다는 것을 화면에 출력
			MessageBox(hDlg, "닉네임을 변경했습니다.\n", "확인", MB_OK);
			
			EndDialog(hDlg, IDCANCEL); // 대화상자 종료
			return TRUE;
		case IDCANCEL: // 취소 버튼을 누른 경우 호출
			if (setup.connectFlag == 1) {
				MessageBox(hDlg, "NickName을 입력하지 않으면\n\n채팅방에 접속할 수 없습니다.\n\n", "경고", MB_ICONERROR);
			}
			EndDialog(hDlg, IDCANCEL); // 대화상자 종료
			return TRUE;
		}
	}
	return FALSE;
	/*
	char myNick[MAXNICK + 1];

	switch (uMsg) {
	case WM_INITDIALOG :
		return TRUE;
	case WM_COMMAND :
		switch (LOWORD(wParam)) {
		case IDOK:
			
			return TRUE;
		case IDCANCEL:
			if (setup.connectFlag == 1) {
				MessageBox(hDlg, "닉네임을 설정해야 채팅방 접속이 가능합니다.\n\n프로그램을 종료합니다.\n\n", "경고", MB_ICONERROR);
				return FALSE;
			}
			
			return TRUE;
		}
		return FALSE;
	}
	
	return FALSE;
	*/
}


// 메시지 출력을 위한 에디터 컨트롤 함수
void DisplayText(char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);

	char cbuf[MAXMSG + MAXNICK + 10];
	vsprintf(cbuf, fmt, arg); // 입력된 문자열을 완성해서 cbuf에 저장한다.
	int nLength = GetWindowTextLength(hEdit1); // 해당 문자열의 길이를 반환한다.
	SendMessage(hEdit1, EM_SETSEL, nLength, nLength); // 현재 hEdit2의 길이 뒤로 포인터를 이동 한 후
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

	for (int i = 0; i < 2; i++) {
		strtok(NULL, ".");
	}

	ptr = strtok(NULL, ".");
	t = atoi(ptr);
	if (t == 0 || t == 255) // IP주소의 마지막값이 0 또는 255이면 주소값 오류
		return FALSE;

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
