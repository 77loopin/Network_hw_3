#ifndef _DATATYPE_
#define _DATATYPE_

// �������� ũ�⸦ �����ϴ� ��ũ�� ���
#define MAXNICK 100 // �ִ� �г��� ����
#define MAXMSG 500 // �ִ� �޽��� ����
#define MAXUSER 10 // �ִ� ���ӻ� �� ( ��� ���� )


// Ŭ���̾�Ʈ�� ���� ���¸� �����ϴ� ��ũ�� ���
#define CONNECT_INIT 0	// �� ���� ����
#define CONNECT_SVR 1	// ������ connect�� ���� ���� (ä�ù� ������)
#define CONNECT_CHAT 2	// �г����� �����ϰ� ä�ù濡 ������ ����


// Ŭ���̾�Ʈ�� ������ ������ �޽����� ������ �����ϴ� ��ũ�� ���
#define CLT_MSG_NEWUSER 1	// ���ο� ������
#define CLT_MSG_CHGNICK 2	// �г��� ����
#define CLT_MSG_ALLMSG 4	// ��ü ä�� �޽���
#define CLT_MSG_WHISPER 5	// �ӼӸ�


// ������ Ŭ���̾�Ʈ�� ������ �޽����� ������ �����ϴ� ��ũ�� ���
#define SVR_MSG_ADDUSER 1	// ����� ���� �˸�
#define SVR_MSG_DELUSER 2	// ����� ���� �˸�
#define SVR_MSG_CHGNICK 3	// ����� �г��� ���� �˸�
#define SVR_MSG_ACCEPT 4	// �г��� ���� �㰡 �޽���
#define SVR_MSG_DENY 5		// �г��� ���� �ź� �޽���
#define SVR_MSG_ALLMSG 6	// ��ü ä�� �޽���
#define SVR_MSG_WHISPER 7	// �ӼӸ� ä�� �޽���
#define SVR_MSG_ADVUSER 8	// ���� ��� ������ �˸�
//(���� ���ӽ� ������ ��� ������ �� ���)


// ������ Message�� ������ ũ�⸦ �����ϴ� Message Header ����ü
// flag�� ���� ������ ���� �κ��� �������ּ���
typedef struct MESSAGEHEADER {
	int flag; // ������ �޽����� ����
	int size; // ������ �޽����� ũ��
} msgHeader;

// ������ �޽��� ����ü
typedef struct MESSAGE {
	char nick[MAXNICK + 1]; // ������ ���(��ü �޽���) Ȥ�� ���� ���(�ӼӸ�)�� �г���
	char msg[MAXMSG + 1]; // ������ �޽���(��ü �޽���) Ȥ�� ������ �г��� (�г��� ���� ��)
} msgData;

// ���� �޽����� ���� argument ������ ��� ����ü
typedef struct SENDERARG {
	SOCKET sock; // ���� ����� ���� ����
	int flag; // ���� �޽����� ����
	int size; // ���� �޽����� ũ��
	char Data[MAXNICK + 1 + MAXMSG + 1]; // ���� �޽��� (�ִ� MESSAGE ����ü�� ũ��)
} senderArgument;

// ���� ������ ����� ����Ʈ
typedef struct USERLIST {
	struct USERLIST *next; // ����Ʈ ADT next
	struct USERLIST *prev; // ����Ʈ ADT prev
	SOCKET clientSocket; // ����ڿ� ���� ���� ����
	int connectFlag; // ������� ���� ����
	char Nick[MAXNICK + 1]; // ������� �г���
} UserList;

// ���� ���� ������ �����ϴ� ����ü ����
typedef struct SETUPINFO {
	char myNick[MAXNICK + 1]; // ������ �г���
	char serverIP[40]; // ���� IP
	int serverPort; // ���ӵ� ���� PORT
	int connectFlag; // ���� ���� ���¸� �����ϴ� �������
}setupInfo;
#endif