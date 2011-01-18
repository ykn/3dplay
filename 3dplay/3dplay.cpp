// 3dplay.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "3dplay.h"

#define MAX_LOADSTRING 100

// �O���[�o���ϐ�:
HINSTANCE hInst;								// ���݂̃C���^�[�t�F�C�X
TCHAR szTitle[MAX_LOADSTRING];					// �^�C�g�� �o�[�̃e�L�X�g
TCHAR szWindowClass[MAX_LOADSTRING];			// ���C�� �E�B���h�E �N���X��

void getScreenShot(int iX, int iY, int iWidth, int iHeight);
void ShowPoint(HWND hWnd, POINTS pt);

HBITMAP _bmpShot = NULL, _bmpOld;
HDC _hdcShot = NULL;
HWND hWnd_target;
int _iWidth, _iHeight;
BOOL bCap = FALSE, bCaptured = FALSE;

#define TIMER_ID     (100)      // �쐬����^�C�}�̎���ID
#define TIMER_ELAPSE (33)     // WM_TIMER�̔����Ԋu (30fps)

// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̐錾��]�����܂�:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: �����ɃR�[�h��}�����Ă��������B
	MSG msg;
	HACCEL hAccelTable;

	// �O���[�o������������������Ă��܂��B
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MY3DPLAY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂�:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY3DPLAY));
	
	// ���C�� ���b�Z�[�W ���[�v:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}



//
//  �֐�: MyRegisterClass()
//
//  �ړI: �E�B���h�E �N���X��o�^���܂��B
//
//  �R�����g:
//
//    ���̊֐�����юg�����́A'RegisterClassEx' �֐����ǉ����ꂽ
//    Windows 95 ���O�� Win32 �V�X�e���ƌ݊�������ꍇ�ɂ̂ݕK�v�ł��B
//    �A�v���P�[�V�������A�֘A�t����ꂽ
//    �������`���̏������A�C�R�����擾�ł���悤�ɂ���ɂ́A
//    ���̊֐����Ăяo���Ă��������B
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY3DPLAY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MY3DPLAY);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �֐�: InitInstance(HINSTANCE, int)
//
//   �ړI: �C���X�^���X �n���h����ۑ����āA���C�� �E�B���h�E���쐬���܂��B
//
//   �R�����g:
//
//        ���̊֐��ŁA�O���[�o���ϐ��ŃC���X�^���X �n���h����ۑ����A
//        ���C�� �v���O���� �E�B���h�E���쐬����ѕ\�����܂��B
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

	//hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	//	CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	//hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	//		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);
	//hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX ,
	//		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX ,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);


	if (!hWnd)
	{
		return FALSE;
	}
	
	RECT rc;
	SetRect(&rc, 0, 0, 640, 480);
	AdjustWindowRectEx(&rc,  WS_OVERLAPPEDWINDOW, TRUE, 0);
	SetWindowPos(hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	SetTimer( hWnd, TIMER_ID, TIMER_ELAPSE, NULL );
	return TRUE;
}

//
//  �֐�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  �ړI:  ���C�� �E�B���h�E�̃��b�Z�[�W���������܂��B
//
//  WM_COMMAND	- �A�v���P�[�V���� ���j���[�̏���
//  WM_PAINT	- ���C�� �E�B���h�E�̕`��
//  WM_DESTROY	- ���~���b�Z�[�W��\�����Ė߂�
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	POINTS pts;
	POINT pos;
	pos.x=0;
	pos.y=0;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �I�����ꂽ���j���[�̉��:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_TIMER:
		InvalidateRect( hWnd, NULL, TRUE);
		InvalidateRect( hWnd, NULL, FALSE);
		break;
	case WM_PAINT:
		if (!bCap) {
			
			RECT rc;
			LPRECT lprc = &rc;
			GetClientRect( hWnd_target, lprc );
			getScreenShot(pos.x, pos.y, lprc->right, lprc->bottom);
			hdc = BeginPaint(hWnd, &ps);
			//if (_bmpShot != NULL) {
				BitBlt(hdc, 0, 0, 640, 480, _hdcShot, 0, 0, SRCCOPY);
			//}
			EndPaint(hWnd, &ps);
			ReleaseDC( hWnd, hdc );
		}
		break;

    case WM_LBUTTONDOWN:
        if (!bCaptured) {
			bCap = TRUE;
			SetCapture(hWnd);
			//pts = MAKEPOINTS(lParam);
			//ShowPoint(hWnd, pts);
			GetCursorPos(&pos);
			pts.x=(short)pos.x;
			pts.y=(short)pos.y;
			ShowPoint(hWnd,pts);
		}
		break;
    case WM_MOUSEMOVE:
        if (!bCaptured) {
			if (bCap) {
				SetCursor(LoadCursor(NULL, IDC_CROSS));
				//pts = MAKEPOINTS(lParam);
				//ShowPoint(hWnd, pts);
				GetCursorPos(&pos);
				// �֐�����������ƃE�B���h�E�̃n���h�����Ԃ�
				hWnd_target = WindowFromPoint(pos);
				pts.x=(short)pos.x;
				pts.y=(short)pos.y;
				ShowPoint(hWnd,pts);
			} else
				SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
        break;
    case WM_LBUTTONUP:
        if (!bCaptured) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			ReleaseCapture();
			bCap = FALSE;
		//	bCaptured = TRUE;
		}
        break;
    case WM_CREATE:
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        break;

	case WM_DESTROY:
		/* �r�b�g�}�b�v���쐬����Ă�����֘A���\�[�X���폜 */
		if (_bmpShot != NULL) {
			SelectObject(_hdcShot, _bmpOld);
			DeleteObject(_bmpShot);
			DeleteObject(_hdcShot);
		}
		KillTimer( hWnd, TIMER_ID );
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


// �o�[�W�������{�b�N�X�̃��b�Z�[�W �n���h���[�ł��B
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void ShowPoint(HWND hWnd, POINTS pt)
{
    TCHAR strbuf[256];
	LPTSTR str=strbuf;
    TCHAR spbuf[] = _T("                                                                     ");
    LPTSTR sp = spbuf;
	TCHAR formatstrbuf[] = _T("x = %4d, y = %4d");
	LPCWSTR formatstr=formatstrbuf;
	HDC hdc;

    wsprintf(str, formatstr, pt.x, pt.y);
    hdc = GetDC(hWnd);
    TextOut(hdc, 20, 20, sp, _tcslen(spbuf));
    TextOut(hdc, 20, 20, str, _tcslen(strbuf));
    
	
	TCHAR windowtext_buf[256];
	LPWSTR windowtext=windowtext_buf;
	GetWindowText(hWnd_target, windowtext, GetWindowTextLength(hWnd_target) + 2 );
	TextOut(hdc, 20, 40, sp, _tcslen(spbuf));
	TextOut(hdc, 20, 40, windowtext, _tcslen(windowtext));
	
	ReleaseDC(hWnd, hdc);
    return;
}

void getScreenShot(int iX, int iY, int iWidth, int iHeight) {

	/* �L���v�`���T�C�Y��ۑ� */
	//_iWidth = iWidth;
	//_iHeight = iHeight;

	/* ��ʂ̃f�o�C�X�R���e�L�X�g�擾 */
	//HDC hdcScreen = GetDC(0);
	HDC hdcScreen = GetDC(hWnd_target);

	/* �X�N���[���V���b�g�ۑ��p�r�b�g�}�b�v�쐬 */
	_bmpShot = CreateCompatibleBitmap(hdcScreen, 640, 480);

	/* �r�b�g�}�b�v�`��p�f�o�C�X�R���e�L�X�g�쐬 */
	_hdcShot = CreateCompatibleDC(hdcScreen);

	/* �f�o�C�X�R���e�L�X�g�Ƀr�b�g�}�b�v��ݒ� */
	_bmpOld = (HBITMAP)SelectObject(_hdcShot, _bmpShot);

	/* ��ʏ�̗̈���r�b�g�}�b�v�ɕ`�� */
	//BitBlt(_hdcShot, 0, 0, iWidth, iHeight, hdcScreen, 0, 0, SRCCOPY);
	SetStretchBltMode(_hdcShot, HALFTONE);
	StretchBlt(_hdcShot, 0, 0, 640, 480, hdcScreen, 0, 0, iWidth, iHeight, SRCCOPY);

	/* ��ʂ̃f�o�C�X�R���e�L�X�g��� */
	ReleaseDC(NULL, hdcScreen);

}

