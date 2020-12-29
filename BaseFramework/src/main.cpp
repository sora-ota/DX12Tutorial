#include "Window.h"
#include "Direct3D.h"
#include <crtdbg.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{

	//メモリリークの確認
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	WIN.InitWindow(hInstance, 1280, 720, "D3D12study");

	{
		Direct3D d3d;
		if (d3d.Initialize() == false)
		{
			printf("初期化失敗");
			return 0;
		}

		if (d3d.TestInit() == false)
		{
			printf("初期化失敗");
			return 0;
		}

		//================================================
		//メインループ
		//================================================
		while (WIN.ProcessMessage())
		{
			d3d.Update();
			d3d.Draw();
		}

		d3d.Release();
	}

	WIN.Release();

	return 0;
}