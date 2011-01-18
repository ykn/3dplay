// 3dplay.cpp : アプリケーションのエントリ ポイントを定義します。
//
#include "stdafx.h"
#include "3dplay.h"
#include <stdlib.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "GL/glut.h"
#include "GL/glui.h"
#include "GL/glext.h"
#include "GL/wglext.h"
#include <math.h>

#define MAX_LOADSTRING 100
#define TIMER_ID     (100)      // 作成するタイマの識別ID
#define TIMER_ELAPSE (33)     // WM_TIMERの発生間隔 (30fps)
#define PLANE_SIZE 10.0  
#define SPLIT_THRESHOLD		10.0  // ポリゴンが繋がっていないと判断する閾値（単位はOpenGLにおける長さ）
#define PI		3.14159265358979	// 円周率
#define CTRLID_CAMERA_RESET		0	// カメラコントロールを初期値に戻す
// グローバル変数:
HINSTANCE hInst;								// 現在のインターフェイス
TCHAR szTitle[MAX_LOADSTRING];					// タイトル バーのテキスト
TCHAR szWindowClass[MAX_LOADSTRING];			// メイン ウィンドウ クラス名

HBITMAP _bmpShot = NULL, _bmpOld;
HDC _hdcShot = NULL;
HWND hWnd_target;
int _iWidth, _iHeight;
BOOL bCap = FALSE, bCaptured = FALSE;

BYTE* g_pTexMap = NULL;
unsigned int g_nTexMapX = 0;
unsigned int g_nTexMapY = 0;

float* g_depth = NULL;
float* g_pVertices3f = NULL;			// 頂点バッファ
float* g_pNormals3f = NULL;				// 法線バッファ
int gMainWindowID = NULL;	// 描画ウィンドウのID（gluiで使用します）
float gTime     = 0.0;		// 時刻 t (秒)
float gTimeMax  = 10.0;		// 時刻 t の最大値(秒)
float gTimeMin  = 0.0;		// 時刻 t の最小値(秒)
int   gShowAxes = 0;		// 座標軸を表示させる場合は 1
float gRotateMat[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };	// カメラ回転行列
float gPanVec[]      = { 0.0, 0.0, 0.0 };						// カメラ平行移動量
GLUI *goGlui;
GLUI_Rotation    *goCamRotation;
GLUI_Translation *goCamTransrationXY;
GLUI_Translation *goCamTransrationZ;
int gStartElapsedTime = 0;		// アニメーション開始時のGLUT_ELAPSED_TIMEを保持する変数
int gLoopFlag	= 1;
double GLUING_TIME = 0.0;		// アニメーション開始からの経過時刻[sec]


// ↓分割数。これを小さくすると表示が荒く、大きくすると細かくなる
unsigned int g_nPolyX = 200; //640;		// 横方向の格子数
unsigned int g_nPolyY = 150; //480;		// 縦方向の格子数

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


// Function prototypes.

void getScreenShot(int iX, int iY, int iWidth, int iHeight);
void ShowPoint(HWND hWnd, POINTS pt);

void glutmain();
void initGL();
void initGLUI();
void glutReshape(int, int);
void glutDisplay();
void glutIdle();
void glutMenu(int);
void controlCallback(int);
void drawGraphics();


void GLUING_Draw();
bool GLUING_Ready();
void glutKeyboard (unsigned char , int , int);

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
	SetRect(&rc, 0, 0, 512, 512);
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
	//	//InvalidateRect( hWnd, NULL, TRUE);
		InvalidateRect( hWnd, NULL, FALSE);
		break;
	case WM_PAINT:
        if (!bCaptured) {
			if (!bCap) {
				RECT rc;
				LPRECT lprc = &rc;
				GetClientRect( hWnd_target, lprc );
				getScreenShot(pos.x, pos.y, lprc->right, lprc->bottom);
				hdc = BeginPaint(hWnd, &ps);
				if (_bmpShot != NULL) {
					BitBlt(hdc, 0, 0, 640, 480, _hdcShot, 0, 0, SRCCOPY);
				}
				EndPaint(hWnd, &ps);
				ReleaseDC( hWnd, hdc );
				
			}
		} else {
			KillTimer( hWnd, TIMER_ID );
			glutmain();
		}
		break;

    case WM_LBUTTONDOWN:
        if (!bCaptured) {
			bCap = TRUE;
			SetCapture(hWnd);
			GetCursorPos(&pos);
			pts.x=(short)pos.x;
			pts.y=(short)pos.y;
			ShowPoint(hWnd,pts);
		}
		break;
    case WM_MOUSEMOVE:
			if (bCap) {
				SetCursor(LoadCursor(NULL, IDC_CROSS));
				GetCursorPos(&pos);
				hWnd_target = WindowFromPoint(pos);
				pts.x=(short)pos.x;
				pts.y=(short)pos.y;
				ShowPoint(hWnd,pts);
			} else
				SetCursor(LoadCursor(NULL, IDC_ARROW));
        break;
    case WM_LBUTTONUP:
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		ReleaseCapture();
		bCap = FALSE;
		{
			RECT rc;
			LPRECT lprc = &rc;
			GetClientRect( hWnd_target, lprc );
			getScreenShot(pos.x, pos.y, lprc->right, lprc->bottom);
			hdc = BeginPaint(hWnd, &ps);
			if (_bmpShot != NULL) {
				BitBlt(hdc, 0, 0, 640, 480, _hdcShot, 0, 0, SRCCOPY);
				bCaptured = TRUE;
			}
			EndPaint(hWnd, &ps);
			ReleaseDC( hWnd, hdc );		
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
		//KillTimer( hWnd, TIMER_ID );
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
	_bmpShot = CreateCompatibleBitmap(hdcScreen, 512, 512);

	/* ビットマップ描画用デバイスコンテキスト作成 */
	_hdcShot = CreateCompatibleDC(hdcScreen);

	/* デバイスコンテキストにビットマップを設定 */
	_bmpOld = (HBITMAP)SelectObject(_hdcShot, _bmpShot);

	/* 画面上の領域をビットマップに描く */
	//BitBlt(_hdcShot, 0, 0, iWidth, iHeight, hdcScreen, 0, 0, SRCCOPY);
	SetStretchBltMode(_hdcShot, HALFTONE);
	StretchBlt(_hdcShot, 0, 0, 512, 512, hdcScreen, 0, 0, iWidth, iHeight, SRCCOPY);

	/* 画面のデバイスコンテキスト解放 */
	ReleaseDC(NULL, hdcScreen);

}

 void glutmain()
{
    glutInitWindowSize(640, 480);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	
	int ac=1; //fake argc
	char* av="\0"; //fake argv

    glutInit(&ac, &av);

	if (!GLUING_Ready()) {
		return;
	}

    gMainWindowID = glutCreateWindow("OpenGL");

    // Set glut callback functions.
	GLUI_Master.set_glutDisplayFunc(glutDisplay);
	GLUI_Master.set_glutKeyboardFunc(glutKeyboard);
	GLUI_Master.set_glutReshapeFunc(glutReshape);
	GLUI_Master.set_glutIdleFunc(glutIdle);

    initGL();
	initGLUI();

    glutMainLoop();

    return ;
}

// Draw
void drawGraphics(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// アニメーション用現在時刻代入
	GLUING_TIME = (double)gTime;

	// アプリの描画ルーチン呼び出し
	GLUING_Draw();

	// 座標軸の描画
	if (gShowAxes) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glDisable(GL_NORMALIZE);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_TEXTURE_2D);

		glBegin(GL_LINES);
		// X軸
		glColor3f(1.0, 0.0, 0.0);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(1.0, 0.0, 0.0);

		// Y軸
		glColor3f(0.0, 1.0, 0.0);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, 1.0, 0.0);

		// Z軸
		glColor3f(0.0, 0.0, 1.0);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, 0.0, 1.0);
		glEnd();
		glPopAttrib();
	}
	glColor3f(1.0, 1.0, 1.0);
}

// Set up general OpenGL rendering properties: lights, depth buffering, etc.
void initGL()
{
	// Lighting
    static const GLfloat light_model_ambient[] = {0.5f, 0.5f, 0.5f, 1.0f};

    static const GLfloat light0_diffuse[] = {0.9f, 0.9f, 0.9f, 0.1f};
    //static const GLfloat light0_direction[] = {-0.2f, -0.4f, 0.5f, 0.0f};
    static const GLfloat light0_direction[] = {0.408f, 0.408f, -0.816f, 0.0f};

    static const GLfloat light1_diffuse[] = {0.4f, 0.4f, 0.4f, 0.1f};
    //static const GLfloat light1_direction[] = {-0.2f, 0.4f, 0.2f, 0.0f};
    static const GLfloat light1_direction[] = {0.0f, 0.0f, -1.0f, 0.0f};
    
	static const GLfloat light2_direction[] = {-0.5f, -0.2f, -0.8f, 0.0f};

    // Enable depth buffering for hidden surface removal.
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    
    // Setup other misc features.
    //glEnable(GL_LIGHTING);
    //glEnable(GL_NORMALIZE);
    //glShadeModel(GL_SMOOTH);

	/* 透明度の設定 */
	/* 色の合成をするようにする */
	glEnable(GL_BLEND);
	
	/* 色の合成方法を指定 */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	//glEnable(GL_COLOR_MATERIAL);
	//glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    //// Setup lighting model.
    //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);    
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);

    //glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    //glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
    //glEnable(GL_LIGHT0);

    //glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    //glLightfv(GL_LIGHT1, GL_POSITION, light1_direction);
    //glEnable(GL_LIGHT1);

    //glLightfv(GL_LIGHT2, GL_DIFFUSE, light0_diffuse);
    //glLightfv(GL_LIGHT2, GL_POSITION, light2_direction);
    //glEnable(GL_LIGHT2);


	// Background color
	glClearColor(0.7, 0.7, 0.7, 1.0);
}

// Initializes GLUI user interface.
void initGLUI()
{
    goGlui = GLUI_Master.create_glui_subwindow(gMainWindowID, GLUI_SUBWINDOW_RIGHT);
    
    // Camera control.
    GLUI_Panel *camctrl_panel = goGlui->add_rollout("Camera control", true);

	// Add 'Show axes' checkbox.
	GLUI_Checkbox *axescheckbox = goGlui->add_checkbox_to_panel(camctrl_panel, "Show axes", &gShowAxes);
	axescheckbox->set_alignment(GLUI_ALIGN_CENTER);

    goCamRotation = goGlui->add_rotation_to_panel(
        camctrl_panel, "Rotate View", gRotateMat);
    goCamRotation->set_spin( 1.0 );
    
    goCamTransrationXY = goGlui->add_translation_to_panel(
        camctrl_panel, "Pan", GLUI_TRANSLATION_XY, gPanVec);
    //goCamTransrationXY->set_speed( 0.01 );
    
    goCamTransrationZ = goGlui->add_translation_to_panel(
        camctrl_panel, "Zoom", GLUI_TRANSLATION_Z, &gPanVec[2]);
    //goCamTransrationZ->set_speed( 0.01 );

	// Add 'Reset' button.
	goGlui->add_button_to_panel(camctrl_panel, "Reset", CTRLID_CAMERA_RESET, controlCallback);

	//// Time span.
 //   GLUI_Panel *time_panel = goGlui->add_rollout("Animation", true);
	//GLUI_Spinner *timemin_spinner = goGlui->add_spinner_to_panel(time_panel, "Start [sec]", GLUI_SPINNER_FLOAT, &gTimeMin);
	//timemin_spinner->set_float_limits(-100.0, 100.0);
	//timemin_spinner->set_alignment(GLUI_ALIGN_RIGHT);

	//GLUI_Spinner *timemax_spinner = goGlui->add_spinner_to_panel(time_panel, "End [sec]", GLUI_SPINNER_FLOAT, &gTimeMax);
	//timemax_spinner->set_float_limits(-100.0, 100.0);
	//timemax_spinner->set_alignment(GLUI_ALIGN_RIGHT);

	////GLUI_Spinner *timestep_spinner = goGlui->add_spinner_to_panel(time_panel, "Step", GLUI_SPINNER_FLOAT, &gTimeStep);
	////timestep_spinner->set_float_limits(0.001, 10.0);
	////timestep_spinner->set_alignment(GLUI_ALIGN_RIGHT);

	//GLUI_Checkbox *loop_checkbox = goGlui->add_checkbox_to_panel(time_panel, "Loop", &gLoopFlag);
	//loop_checkbox->set_alignment(GLUI_ALIGN_CENTER);

	//// Add 'Animation Start/Stop' button.
	//goGlui->add_separator_to_panel(time_panel);
	//goGlui->add_button_to_panel(time_panel, "Start animation", CTRLID_ANI_START, controlCallback);
	//goGlui->add_button_to_panel(time_panel, "Stop animation", CTRLID_ANI_STOP, controlCallback);

	// Add 'Quit' button.
    goGlui->add_button("Quit", 0, (GLUI_Update_CB) exit);
    
    goGlui->set_main_gfx_window(gMainWindowID);
}

// Callback function for UI.
//
// Parameter
//	control : Identifier
void controlCallback(int control)
{
	switch (control) {
		case CTRLID_CAMERA_RESET:	// カメラ操作のリセットボタンが押された
			goCamRotation->reset();
			goCamTransrationXY->set_x(0.0);
			goCamTransrationXY->set_y(0.0);
			goCamTransrationZ->set_z(0.0);
			break;

	}
}

// GLUT callback for reshaping the window.
// This is the main place where the viewing and workspace transforms get initialized.
//
// Pamareters
//	width	: Window width
//	height	: Window height
void glutReshape(int width, int height)
{
	static const double kFovY = 40;
	int tx, ty;
    double nearDist, farDist, aspect;

	GLUI_Master.get_viewport_area(&tx, &ty, &width, &height);
    glViewport(tx, ty, width, height);

    // Compute the viewing parameters based on a fixed fov and viewing
    // a canonical box centered at the origin.
    nearDist = 1.0 / tan((kFovY / 2.0) * PI / 180.0);
    farDist = nearDist + 150.0;
    aspect = (double) width / height;
   
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(kFovY, aspect, nearDist, farDist);

    // Place the camera down the Z axis looking at the origin.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();            
    gluLookAt(0, 0, nearDist +15.0,
              0, 0, 0,
              0, 1, 0);
}

// GLUT callback for redrawing the view.
void glutDisplay()
{
	glPushMatrix();
	glTranslatef(gPanVec[0] / 50.0, gPanVec[1] / 50.0, -gPanVec[2] / 10.0);		// Is "trans->set_spped()" not working?
	glMultMatrixf(gRotateMat);

    drawGraphics();			// Draw shapes.

	glPopMatrix();

    glutSwapBuffers();		// Change drawing buffer.
}

// GLUT callback for idling.
//  ここでアニメーションの制御を行う
void glutIdle()
{
	// おまじない
	if (glutGetWindow() != gMainWindowID) {
		glutSetWindow(gMainWindowID);
	}
    glutPostRedisplay();

}



void glutKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
			exit (1);
	}
}




//---------------------------------------------------------------------------
// ベクトル・行列演算関数

// ベクトルの長さを返す
float length(const float* vec)
{
	return sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

// ベクトルの外積を計算
void cross(const float* vec0, const float* vec1, float* dest)
{
	dest[0] = vec0[1] * vec1[2] - vec0[2] * vec1[1];
	dest[1] = vec0[2] * vec1[0] - vec0[0] * vec1[2];
	dest[2] = vec0[0] * vec1[1] - vec0[1] * vec1[0];
}

// 三角形ポリゴンの法線ベクトルを計算
void normal(const float* vec0, const float* vec1, const float* vec2, float* norm)
{
	float r10[3], r20[3], crs[3];
	for (int i = 0; i < 3; ++i)
	{
		r10[i] = vec0[i] - vec1[i];
		r20[i] = vec0[i] - vec2[i];
	}
	cross(r10, r20, crs);

	float len = length(crs);
	if (len == 0.0) {
		norm[0] = 0;
		norm[1] = 0;
		norm[2] = 1;
	} else {
		norm[0] = crs[0] / len;
		norm[1] = crs[1] / len;
		norm[2] = crs[2] / len;
	}
}


//---------------------------------------------------------------------------
// 初期化処理

// 最初に１回だけ行う処理

bool GLUING_Ready()
{
	// Texture map init
	g_nTexMapX = 512;
	g_nTexMapY = 512;
	g_pTexMap = (BYTE*)malloc(g_nTexMapX * g_nTexMapY * 3 * sizeof(BYTE));

	// 頂点バッファ確保
	g_pVertices3f = (float*)calloc(g_nPolyX * g_nPolyY * 3, sizeof(float));

	return TRUE;
}


//---------------------------------------------------------------------------
// 描画処理

// OpenGLの描画処理をここに書く。

void GLUING_Draw (void)
{
	//HBITMAPからbitmap(R8,G8,B8)を取り出したい
	RECT rc;
	LPRECT lprc = &rc;
	GetClientRect( hWnd_target, lprc );
	HDC hdcScreen = GetDC(hWnd_target);
	_bmpShot = CreateCompatibleBitmap(hdcScreen, 512, 512);
	_hdcShot = CreateCompatibleDC(hdcScreen);
	_bmpOld = (HBITMAP)SelectObject(_hdcShot, _bmpShot);
	SetStretchBltMode(_hdcShot, HALFTONE);
	StretchBlt(_hdcShot, 0, 0, 512, 512, hdcScreen, 0, 0, lprc->right, lprc->bottom, SRCCOPY);
	_bmpOld = (HBITMAP)SelectObject(_hdcShot, _bmpShot);
	ReleaseDC(NULL, hdcScreen);

	BITMAP bmptemp;
	LPBYTE lp;
	GetObject(_bmpOld, sizeof(BITMAP), &bmptemp);
	lp = (LPBYTE)bmptemp.bmBits;

	// 頂点座標初期化
	for (unsigned int y = 0; y < g_nPolyY; ++y)
	{
		for (unsigned int x = 0; x < g_nPolyX; ++x)
		{
			for (int i = 0; i < 3; ++i)
			{
				g_pVertices3f[(y * g_nPolyX + x) * 3 + i] = 0;
			}
		}
	}

	// 対応するポリゴン毎の深度の平均を求める。頂点バッファを一時的に作業領域に使っている
	int px, py;
	int n=0, Depth=0, red=0, blue=0, green=0;
	for (int y=0; y < 512; ++y)
	{
		py = y * g_nPolyY / 512;

		//const XnDepthPixel* pDepth = pDepthRow;
		//XnRGB24Pixel* pTex = pTexRow + g_depthMD.XOffset();

		for (int x = 0; x < 512; ++x)
		{
			blue = lp[n];
			green = lp[n+1];
			red = lp[n+2];
			n+=3;

			Depth = ((red   & 2) << 6) + ((red   & 1) << 4) +
				    ((green & 2) << 5) + ((green & 1) << 3) +
					((blue  & 2) << 4) + ((blue  & 1) << 2);


			px = x * g_nPolyX / 512;

			if (Depth != 0)	// 深度0となっている画素は平均値の計算から除く
			{
				float DepthValue = 0;
				
				if ( Depth < 2500) {
					DepthValue = (float)(Depth - 500.0) * 0.048;
				} else {
					DepthValue = 0.012 *(float)(Depth - 2500.0) + 96.0;
				}

				// Z値の集計
				g_pVertices3f[(py * g_nPolyX + px) * 3 + 2] +=  128.0 - (float)DepthValue;	// 頂点ベクトルの第3要素に入れておく（ホントは値が違うけど）

				// 面積の集計
				g_pVertices3f[(py * g_nPolyX + px) * 3] += 1.0;		// 頂点ベクトルの第1要素に入れておく（ホントは用途が違うけど）
			}
		}
	}

	// 頂点座標を計算
	float dx = (float)PLANE_SIZE / (float)g_nPolyX;
	float dy = (float)PLANE_SIZE * (float)512 / (float)512 / (float)g_nPolyY;
	float xoffset = -(float)(PLANE_SIZE / 2);
	float yoffset =  (float)(PLANE_SIZE / 2) * (float)512 / (float)512;
	float zoffset = -(float)(PLANE_SIZE / 2);
	for (unsigned int y = 0; y < g_nPolyY; ++y)
	{
		for (unsigned int x = 0; x < g_nPolyX; ++x)
		{
			int index = (y * g_nPolyX + x) * 3;

			if (g_pVertices3f[index] > 0.0) {
				// 奥行き計測面積ゼロでなければZ座標を計算
				g_pVertices3f[index + 2] /= (g_pVertices3f[index] * (float)PLANE_SIZE);
			} else {
				// 奥行き計測面積ゼロなら奥行きゼロとする
				//g_pVertices3f[index + 2] = 0;
				g_pVertices3f[index + 2] = -10.0;
			}

			g_pVertices3f[index] = dx * (float)x + xoffset;			// X座標
			g_pVertices3f[index + 1] = dy * -(float)y + yoffset;	// Y座標
			//g_pVertices3f[index + 2] = (g_pVertices3f[index + 2] / 20.0) + zoffset;	// Z座標。5.0で割っているのは適当な凹凸に調節するため。
			g_pVertices3f[index + 2] = (g_pVertices3f[index + 2]) + zoffset;	// Z座標。5.0で割っているのは適当な凹凸に調節するため。
		}
	}

	// Create the OpenGL texture map
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, bmptemp.bmWidth, bmptemp.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmptemp.bmBits);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);

	// Enable texture
	glEnable(GL_TEXTURE_2D);

	// 凹凸を付けて描画
	glBegin(GL_TRIANGLES);
	float texWidth = (float)512 / (float)g_nTexMapX;
	float texHeight = (float)512 / (float)g_nTexMapY;
	for (unsigned int y = 0; y < g_nPolyY - 1; ++y)
	{
		for (unsigned int x = 0; x < g_nPolyX - 1; ++x)
		{
			// 対応するテクスチャの座標
			float tx0 = ((float)x) / (float)(g_nPolyX - 1) * texWidth;
			float ty0 = ((float)y) / (float)(g_nPolyY - 1) * texHeight;
			float tx1 = ((float)x + 1) / (float)(g_nPolyX - 1) * texWidth;
			float ty1 = ((float)y + 1) / (float)(g_nPolyY - 1) * texHeight;

			// ポリゴンの頂点
			float* pUL = g_pVertices3f + ((y * g_nPolyX + x) * 3);
			float* pUR = pUL + 3;
			float* pBR = pUL + (g_nPolyX * 3) + 3;
			float* pBL = pUL + (g_nPolyX * 3);

			// 法線を求める
			//float norm[3];

			// 閾値以上離れているメッシュの処理
			if (abs(pUR[2] - pUL[2]) > SPLIT_THRESHOLD || abs(pBR[2] - pBL[2]) > SPLIT_THRESHOLD
				|| abs(pBL[2] - pUL[2]) > SPLIT_THRESHOLD || abs(pBR[2] - pUR[2]) > SPLIT_THRESHOLD) {
// ちょっとトリッキーな書き方だけど、#ifの値で表示方法の選択ができる
// ↓ 1…離れたポリゴンは切り離す，0…切り離さず色を変える
#if 1
					glColor4f(1.0, 1.0, 1.0, 0.001);
					//continue;					// 描画しない。つまり隣のポリゴンと切り離す
#else
					glColor3f(0.0, 1.0, 0.0);	// 切り離さず緑にする
#endif
			} else {
				glColor4f(1.0, 1.0, 1.0, 1.0);	// 通常はテクスチャの色になるよう、白とする
			}

			//glColor3f(1.0, 1.0, 1.0);

			// 四角形の格子を書くには三角形が２つ要る

			// 三角形その１
			//normal(pUL, pUR, pBR, norm);
			//glNormal3fv(norm);
			// upper left
			glTexCoord2f(tx0, ty0);
			glVertex3fv(pUL);
			// upper right
			glTexCoord2f(tx1, ty0);
			glVertex3fv(pUR);
			// bottom right
			glTexCoord2f(tx1, ty1);
			glVertex3fv(pBR);

			// 三角形その２
			//normal(pBL, pUL, pBR, norm);
			//glNormal3fv(norm);
			// bottom left
			glTexCoord2f(tx0, ty1);
			glVertex3fv(pBL);
			// upper right
			glTexCoord2f(tx0, ty0);
			glVertex3fv(pUL);
			// bottom right
			glTexCoord2f(tx1, ty1);
			glVertex3fv(pBR);
		}
	}

	glEnd();
}

