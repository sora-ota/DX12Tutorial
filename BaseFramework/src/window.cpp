#include "window.h"
#include <stdio.h>

bool Window::InitWindow(HINSTANCE hInstance, int w, int h, const char * name)
{
#ifdef _DEBUG
	// コンソールを作成する
	AllocConsole();
	// 標準入出力に割り当てる
	FILE* fp = NULL;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONIN$", "r", stdin);
#endif

	//カレントディレクトリ取得
	m_hInst = hInstance;
	//====================================================================================
	//メインウィンドウ作成
	//====================================================================================
	//ウィンドウクラスの定義
	WNDCLASSEX wc;//ウィンドウクラスの定義用
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = &Window::callWinowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInst;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "Window";

	//ウィンドウクラスの登録
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	//ウィンドウの作成
	m_hWnd = CreateWindow(
		"Window",//作成するウィンドウ名
		name,//ウィンドウのタイトル
		WS_OVERLAPPEDWINDOW - WS_THICKFRAME,//ウィンドウタイプを標準タイプに
		0,//ウィンドウの位置(X座標)
		0,//ウィンドウの位置(Y座標)
		800,//ウィンドウの幅
		600,//ウィンドウの高さ
		nullptr,//親ウインドウのハンドル
		nullptr,//メニューのハンドル
		m_hInst,//インスタンスハンドル
		nullptr);//追加情報

	//クライアントのサイズを指定
	SetClientSize(m_hWnd, w, h);

	//タイマーの最小精度を1msecにする
	timeBeginPeriod(1);

	//=========================================================================
	//DirectXの初期化
	//=========================================================================


	//=========================================================================
	//ウィンドウの表示
	//=========================================================================
	ShowWindow(m_hWnd, SW_SHOW);
	//ウィンドウの更新
	UpdateWindow(m_hWnd);

	return true;
}

bool Window::ProcessMessage()
{
	m_MouseWheelVal = 0;

	while (true)
	{
		MSG msg;
		// メッセージ取得
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// 終了メッセージがきたら、ループ終了
			if (msg.message == WM_QUIT) {
				return false;
			}

			//=========================================
			//メッセージ処理
			//=========================================
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			return true;
		}
	}
}

LRESULT Window::callWinowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return WIN.WindowProc(hWnd, message, wParam, lParam);
}

LRESULT Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//メッセージによって処理を選択
	switch (message) {
	case WM_SIZE:
		RECT rcWnd, rcCli;
		GetWindowRect(hWnd, &rcWnd); // ウィンドウのRECT取得
		GetClientRect(hWnd, &rcCli); // クライアント領域のRECT取得
		m_WindowWidth = rcCli.right;
		m_WindowHeight = rcCli.bottom;

		break;
	// ホイールスクロール時
	case WM_MOUSEWHEEL:
		{
			m_MouseWheelVal = (short)HIWORD(wParam);
		}
		break;
	// Xボタンが押された
	case WM_CLOSE:
		// 破棄
		Release();
		break;
	//ウィンドウ閉じるとき
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		//不要なメッセージの後始末
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Window::SetClientSize(HWND hWnd, int w, int h) const
{
	RECT rcWnd, rcCli;

	GetWindowRect(hWnd, &rcWnd); // ウィンドウのRECT取得
	GetClientRect(hWnd, &rcCli); // クライアント領域のRECT取得

								 // ウィンドウの余白を考えて、クライアントのサイズを指定サイズにする。
	MoveWindow(hWnd,
		rcWnd.left,// X座標
		rcWnd.top,// Y座標
		w + (rcWnd.right - rcWnd.left) - (rcCli.right - rcCli.left),
		h + (rcWnd.bottom - rcWnd.top) - (rcCli.bottom - rcCli.top),
		TRUE);
}

void Window::Release()
{
	if (m_hWnd) {
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}

	//タイマーの最小精度を戻す
	timeEndPeriod(1);

#ifdef _DEBUG
	// コンソール解放
	FreeConsole();
#endif
}
