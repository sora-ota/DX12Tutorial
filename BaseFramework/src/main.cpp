#define STB_IMAGE_IMPLEMENTATION
#include "window.h"
#include <crtdbg.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{

	//メモリリークの確認
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	WIN.InitWindow(hInstance, 1280, 720, "D3D12study");

	//================================================
	//メインループ
	//================================================
	while (WIN.ProcessMessage())
	{
	}

	WIN.Release();

	return 0;
}