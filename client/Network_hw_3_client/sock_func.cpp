//#include "sock_func.h"
#include "sock_lib.h"
#include "std_error.h"
#include "data_type.h"


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
		display_MB("�������� ���ῡ �����߽��ϴ�.\n\n�ٽ� �������ֽñ� �ٶ��ϴ�.\n\n");
		//err_MB("connect()");
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
	int sendSize = 0;
	msgHeader header;
	header.flag = flag;
	header.size = size;
	
	sendSize = send(sock, (char*)&header, sizeof(header), 0);
	sendSize = sendSize + send(sock, Data, size, 0);
	
	return sendSize;
}


