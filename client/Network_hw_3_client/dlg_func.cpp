#include "sock_lib.h"

BOOL CALLBACK main_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG :
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL: // ��� ��ư�� ���� ���
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� ����, �� ���α׷� ����
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}