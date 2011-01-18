// 3dplay.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "3dplay.h"

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;								// 現在のインターフェイス
TCHAR szTitle[MAX_LOADSTRING];					// タイトル バーのテキスト
TCHAR szWindowClass[MAX_LOADSTRING];			// メイン ウィンドウ クラス名

void getScreenShot(int iX, int iY, int iWidth, int iHeight);
void ShowPoint(HWND hWnd, POINTS pt);

HBITMAP _bmpShot = NULL, _bmpOld;
HDC _hdcShot = NULL;
HWND hWnd_target;
int _iWidth, _iHeight;
BOOL bCap = FALSE, bCaptured = FALSE;

#define TIMER_ID     (100)      // 作成するタイマの識別ID
#define TIMER_ELAPSE (33)     // WM_TIMERの発生間隔 (30fps)

// このコード モジュールに含まれる関数の宣言を転送します:
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

 	// TODO: ここにコードを挿入してください。
	MSG msg;
	HACCEL hAccelTable;

	// グローバル文字列を初期化しています。
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MY3DPLAY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY3DPLAY));
	
	// メイン メッセージ ループ:
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
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
//  コメント:
//
//    この関数および使い方は、'RegisterClassEx' 関数が追加された
//    Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
//    アプリケーションが、関連付けられた
//    正しい形式の小さいアイコンを取得できるようにするには、
//    この関数を呼び出してください。
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
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

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
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
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
		// 選択されたメニューの解析:
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
				// 関数が成功するとウィンドウのハンドルが返る
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
		/* ビットマップが作成されていたら関連リソースを削除 */
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


// バージョン情報ボックスのメッセージ ハンドラーです。
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

	/* キャプチャサイズを保存 */
	//_iWidth = iWidth;
	//_iHeight = iHeight;

	/* 画面のデバイスコンテキスト取得 */
	//HDC hdcScreen = GetDC(0);
	HDC hdcScreen = GetDC(hWnd_target);

	/* スクリーンショット保存用ビットマップ作成 */
	_bmpShot = CreateCompatibleBitmap(hdcScreen, 640, 480);

	/* ビットマップ描画用デバイスコンテキスト作成 */
	_hdcShot = CreateCompatibleDC(hdcScreen);

	/* デバイスコンテキストにビットマップを設定 */
	_bmpOld = (HBITMAP)SelectObject(_hdcShot, _bmpShot);

	/* 画面上の領域をビットマップに描く */
	//BitBlt(_hdcShot, 0, 0, iWidth, iHeight, hdcScreen, 0, 0, SRCCOPY);
	SetStretchBltMode(_hdcShot, HALFTONE);
	StretchBlt(_hdcShot, 0, 0, 640, 480, hdcScreen, 0, 0, iWidth, iHeight, SRCCOPY);

	/* 画面のデバイスコンテキスト解放 */
	ReleaseDC(NULL, hdcScreen);

}

