#include "Window.h"
#include "Direct3D.h"
#include <crtdbg.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{

	//���������[�N�̊m�F
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	WIN.InitWindow(hInstance, 1280, 720, "D3D12study");

	{
		Direct3D d3d;
		if (d3d.Initialize() == false)
		{
			printf("���������s");
			return 0;
		}

		if (d3d.TestInit() == false)
		{
			printf("���������s");
			return 0;
		}

		//================================================
		//���C�����[�v
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