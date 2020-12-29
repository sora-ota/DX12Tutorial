#pragma once

#pragma comment(lib,"winmm.lib")
#include <Windows.h>

class Window
{
public:
	//ウィンドウハンドル取得
	HWND GethWnd() { return m_hWnd; }
	//サイズ取得
	u32 GetWindowWidth() { return m_WindowWidth; }
	u32 GetWindowHeight() { return m_WindowHeight; }
	// マウスホイールの変化量を取得
	int GetMouseWheelVal() const { return m_MouseWheelVal; }
	//ウィンドウ作成、Direct3Dの初期化など
	bool InitWindow(HINSTANCE hInstance, int w, int h, const char* name);
	//ウィンドウプロシージャ
	bool ProcessMessage();
	static LRESULT CALLBACK callWinowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	//クライアントサイズの設定
	void SetClientSize(HWND hWnd, int w, int h) const;
	//フルスクリーン設定
	void SetWindowMode(bool mode)
	{
		m_WindowMode = mode;
	}
	//解放
	void Release();
private:
	HINSTANCE m_hInst;//アプリケーションのインスタンスハンドル
	HWND m_hWnd;//ウィンドウハンドル
	int m_WindowWidth = 0;
	int m_WindowHeight = 0;//ウィンドウクライアント解像度
	DWORD m_FrameCnt = 0;//現在のフレームカウント
	int	m_MouseWheelVal = 0;// マウスホイール値
	int m_Fps = 0;
	bool m_WindowMode = false;
	//シングルトン
public:
	static Window& GetInstance()
	{
		static Window Instance;
		return Instance;
	}
private:
	Window() {}//デフォルトコンストラクタ
	Window(Window&);//コピーコンストラクタ
	Window& operator = (Window&);//＝演算子のオーバーロード(代入)
	~Window() {}//デストラクタ
};
#define WIN Window::GetInstance()
