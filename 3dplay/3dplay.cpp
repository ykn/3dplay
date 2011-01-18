// 3dplay.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
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
#define TIMER_ID     (100)      // �쐬����^�C�}�̎���ID
#define TIMER_ELAPSE (33)     // WM_TIMER�̔����Ԋu (30fps)
#define PLANE_SIZE 10.0  
#define SPLIT_THRESHOLD		10.0  // �|���S�����q�����Ă��Ȃ��Ɣ��f����臒l�i�P�ʂ�OpenGL�ɂ����钷���j
#define PI		3.14159265358979	// �~����
#define CTRLID_CAMERA_RESET		0	// �J�����R���g���[���������l�ɖ߂�
// �O���[�o���ϐ�:
HINSTANCE hInst;								// ���݂̃C���^�[�t�F�C�X
TCHAR szTitle[MAX_LOADSTRING];					// �^�C�g�� �o�[�̃e�L�X�g
TCHAR szWindowClass[MAX_LOADSTRING];			// ���C�� �E�B���h�E �N���X��

HBITMAP _bmpShot = NULL, _bmpOld;
HDC _hdcShot = NULL;
HWND hWnd_target;
int _iWidth, _iHeight;
BOOL bCap = FALSE, bCaptured = FALSE;

BYTE* g_pTexMap = NULL;
unsigned int g_nTexMapX = 0;
unsigned int g_nTexMapY = 0;

float* g_depth = NULL;
float* g_pVertices3f = NULL;			// ���_�o�b�t�@
float* g_pNormals3f = NULL;				// �@���o�b�t�@
int gMainWindowID = NULL;	// �`��E�B���h�E��ID�iglui�Ŏg�p���܂��j
float gTime     = 0.0;		// ���� t (�b)
float gTimeMax  = 10.0;		// ���� t �̍ő�l(�b)
float gTimeMin  = 0.0;		// ���� t �̍ŏ��l(�b)
int   gShowAxes = 0;		// ���W����\��������ꍇ�� 1
float gRotateMat[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };	// �J������]�s��
float gPanVec[]      = { 0.0, 0.0, 0.0 };						// �J�������s�ړ���
GLUI *goGlui;
GLUI_Rotation    *goCamRotation;
GLUI_Translation *goCamTransrationXY;
GLUI_Translation *goCamTransrationZ;
int gStartElapsedTime = 0;		// �A�j���[�V�����J�n����GLUT_ELAPSED_TIME��ێ�����ϐ�
int gLoopFlag	= 1;
double GLUING_TIME = 0.0;		// �A�j���[�V�����J�n����̌o�ߎ���[sec]


// ���������B���������������ƕ\�����r���A�傫������ƍׂ����Ȃ�
unsigned int g_nPolyX = 200; //640;		// �������̊i�q��
unsigned int g_nPolyY = 150; //480;		// �c�����̊i�q��

// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̐錾��]�����܂�:
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
	SetRect(&rc, 0, 0, 512, 512);
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
		/* �r�b�g�}�b�v���쐬����Ă�����֘A���\�[�X���폜 */
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
	_bmpShot = CreateCompatibleBitmap(hdcScreen, 512, 512);

	/* �r�b�g�}�b�v�`��p�f�o�C�X�R���e�L�X�g�쐬 */
	_hdcShot = CreateCompatibleDC(hdcScreen);

	/* �f�o�C�X�R���e�L�X�g�Ƀr�b�g�}�b�v��ݒ� */
	_bmpOld = (HBITMAP)SelectObject(_hdcShot, _bmpShot);

	/* ��ʏ�̗̈���r�b�g�}�b�v�ɕ`�� */
	//BitBlt(_hdcShot, 0, 0, iWidth, iHeight, hdcScreen, 0, 0, SRCCOPY);
	SetStretchBltMode(_hdcShot, HALFTONE);
	StretchBlt(_hdcShot, 0, 0, 512, 512, hdcScreen, 0, 0, iWidth, iHeight, SRCCOPY);

	/* ��ʂ̃f�o�C�X�R���e�L�X�g��� */
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

	// �A�j���[�V�����p���ݎ������
	GLUING_TIME = (double)gTime;

	// �A�v���̕`�惋�[�`���Ăяo��
	GLUING_Draw();

	// ���W���̕`��
	if (gShowAxes) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glDisable(GL_NORMALIZE);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_TEXTURE_2D);

		glBegin(GL_LINES);
		// X��
		glColor3f(1.0, 0.0, 0.0);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(1.0, 0.0, 0.0);

		// Y��
		glColor3f(0.0, 1.0, 0.0);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, 1.0, 0.0);

		// Z��
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

	/* �����x�̐ݒ� */
	/* �F�̍���������悤�ɂ��� */
	glEnable(GL_BLEND);
	
	/* �F�̍������@���w�� */
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
		case CTRLID_CAMERA_RESET:	// �J��������̃��Z�b�g�{�^���������ꂽ
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
//  �����ŃA�j���[�V�����̐�����s��
void glutIdle()
{
	// ���܂��Ȃ�
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
// �x�N�g���E�s�񉉎Z�֐�

// �x�N�g���̒�����Ԃ�
float length(const float* vec)
{
	return sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

// �x�N�g���̊O�ς��v�Z
void cross(const float* vec0, const float* vec1, float* dest)
{
	dest[0] = vec0[1] * vec1[2] - vec0[2] * vec1[1];
	dest[1] = vec0[2] * vec1[0] - vec0[0] * vec1[2];
	dest[2] = vec0[0] * vec1[1] - vec0[1] * vec1[0];
}

// �O�p�`�|���S���̖@���x�N�g�����v�Z
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
// ����������

// �ŏ��ɂP�񂾂��s������

bool GLUING_Ready()
{
	// Texture map init
	g_nTexMapX = 512;
	g_nTexMapY = 512;
	g_pTexMap = (BYTE*)malloc(g_nTexMapX * g_nTexMapY * 3 * sizeof(BYTE));

	// ���_�o�b�t�@�m��
	g_pVertices3f = (float*)calloc(g_nPolyX * g_nPolyY * 3, sizeof(float));

	return TRUE;
}


//---------------------------------------------------------------------------
// �`�揈��

// OpenGL�̕`�揈���������ɏ����B

void GLUING_Draw (void)
{
	//HBITMAP����bitmap(R8,G8,B8)�����o������
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

	// ���_���W������
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

	// �Ή�����|���S�����̐[�x�̕��ς����߂�B���_�o�b�t�@���ꎞ�I�ɍ�Ɨ̈�Ɏg���Ă���
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

			if (Depth != 0)	// �[�x0�ƂȂ��Ă����f�͕��ϒl�̌v�Z���珜��
			{
				float DepthValue = 0;
				
				if ( Depth < 2500) {
					DepthValue = (float)(Depth - 500.0) * 0.048;
				} else {
					DepthValue = 0.012 *(float)(Depth - 2500.0) + 96.0;
				}

				// Z�l�̏W�v
				g_pVertices3f[(py * g_nPolyX + px) * 3 + 2] +=  128.0 - (float)DepthValue;	// ���_�x�N�g���̑�3�v�f�ɓ���Ă����i�z���g�͒l���Ⴄ���ǁj

				// �ʐς̏W�v
				g_pVertices3f[(py * g_nPolyX + px) * 3] += 1.0;		// ���_�x�N�g���̑�1�v�f�ɓ���Ă����i�z���g�͗p�r���Ⴄ���ǁj
			}
		}
	}

	// ���_���W���v�Z
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
				// ���s���v���ʐσ[���łȂ����Z���W���v�Z
				g_pVertices3f[index + 2] /= (g_pVertices3f[index] * (float)PLANE_SIZE);
			} else {
				// ���s���v���ʐσ[���Ȃ牜�s���[���Ƃ���
				//g_pVertices3f[index + 2] = 0;
				g_pVertices3f[index + 2] = -10.0;
			}

			g_pVertices3f[index] = dx * (float)x + xoffset;			// X���W
			g_pVertices3f[index + 1] = dy * -(float)y + yoffset;	// Y���W
			//g_pVertices3f[index + 2] = (g_pVertices3f[index + 2] / 20.0) + zoffset;	// Z���W�B5.0�Ŋ����Ă���͓̂K���ȉ��ʂɒ��߂��邽�߁B
			g_pVertices3f[index + 2] = (g_pVertices3f[index + 2]) + zoffset;	// Z���W�B5.0�Ŋ����Ă���͓̂K���ȉ��ʂɒ��߂��邽�߁B
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

	// ���ʂ�t���ĕ`��
	glBegin(GL_TRIANGLES);
	float texWidth = (float)512 / (float)g_nTexMapX;
	float texHeight = (float)512 / (float)g_nTexMapY;
	for (unsigned int y = 0; y < g_nPolyY - 1; ++y)
	{
		for (unsigned int x = 0; x < g_nPolyX - 1; ++x)
		{
			// �Ή�����e�N�X�`���̍��W
			float tx0 = ((float)x) / (float)(g_nPolyX - 1) * texWidth;
			float ty0 = ((float)y) / (float)(g_nPolyY - 1) * texHeight;
			float tx1 = ((float)x + 1) / (float)(g_nPolyX - 1) * texWidth;
			float ty1 = ((float)y + 1) / (float)(g_nPolyY - 1) * texHeight;

			// �|���S���̒��_
			float* pUL = g_pVertices3f + ((y * g_nPolyX + x) * 3);
			float* pUR = pUL + 3;
			float* pBR = pUL + (g_nPolyX * 3) + 3;
			float* pBL = pUL + (g_nPolyX * 3);

			// �@�������߂�
			//float norm[3];

			// 臒l�ȏ㗣��Ă��郁�b�V���̏���
			if (abs(pUR[2] - pUL[2]) > SPLIT_THRESHOLD || abs(pBR[2] - pBL[2]) > SPLIT_THRESHOLD
				|| abs(pBL[2] - pUL[2]) > SPLIT_THRESHOLD || abs(pBR[2] - pUR[2]) > SPLIT_THRESHOLD) {
// ������ƃg���b�L�[�ȏ����������ǁA#if�̒l�ŕ\�����@�̑I�����ł���
// �� 1�c���ꂽ�|���S���͐؂藣���C0�c�؂藣�����F��ς���
#if 1
					glColor4f(1.0, 1.0, 1.0, 0.001);
					//continue;					// �`�悵�Ȃ��B�܂�ׂ̃|���S���Ɛ؂藣��
#else
					glColor3f(0.0, 1.0, 0.0);	// �؂藣�����΂ɂ���
#endif
			} else {
				glColor4f(1.0, 1.0, 1.0, 1.0);	// �ʏ�̓e�N�X�`���̐F�ɂȂ�悤�A���Ƃ���
			}

			//glColor3f(1.0, 1.0, 1.0);

			// �l�p�`�̊i�q�������ɂ͎O�p�`���Q�v��

			// �O�p�`���̂P
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

			// �O�p�`���̂Q
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

