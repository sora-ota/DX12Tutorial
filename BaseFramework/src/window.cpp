#include "window.h"
#include <stdio.h>

bool Window::InitWindow(HINSTANCE hInstance, int w, int h, const char * name)
{
#ifdef _DEBUG
	// �R���\�[�����쐬����
	AllocConsole();
	// �W�����o�͂Ɋ��蓖�Ă�
	FILE* fp = NULL;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONIN$", "r", stdin);
#endif

	//�J�����g�f�B���N�g���擾
	m_hInst = hInstance;
	//====================================================================================
	//���C���E�B���h�E�쐬
	//====================================================================================
	//�E�B���h�E�N���X�̒�`
	WNDCLASSEX wc;//�E�B���h�E�N���X�̒�`�p
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

	//�E�B���h�E�N���X�̓o�^
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	//�E�B���h�E�̍쐬
	m_hWnd = CreateWindow(
		"Window",//�쐬����E�B���h�E��
		name,//�E�B���h�E�̃^�C�g��
		WS_OVERLAPPEDWINDOW - WS_THICKFRAME,//�E�B���h�E�^�C�v��W���^�C�v��
		0,//�E�B���h�E�̈ʒu(X���W)
		0,//�E�B���h�E�̈ʒu(Y���W)
		800,//�E�B���h�E�̕�
		600,//�E�B���h�E�̍���
		nullptr,//�e�E�C���h�E�̃n���h��
		nullptr,//���j���[�̃n���h��
		m_hInst,//�C���X�^���X�n���h��
		nullptr);//�ǉ����

	//�N���C�A���g�̃T�C�Y���w��
	SetClientSize(m_hWnd, w, h);

	//�^�C�}�[�̍ŏ����x��1msec�ɂ���
	timeBeginPeriod(1);

	//=========================================================================
	//DirectX�̏�����
	//=========================================================================


	//=========================================================================
	//�E�B���h�E�̕\��
	//=========================================================================
	ShowWindow(m_hWnd, SW_SHOW);
	//�E�B���h�E�̍X�V
	UpdateWindow(m_hWnd);

	return true;
}

bool Window::ProcessMessage()
{
	m_MouseWheelVal = 0;

	while (true)
	{
		MSG msg;
		// ���b�Z�[�W�擾
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// �I�����b�Z�[�W��������A���[�v�I��
			if (msg.message == WM_QUIT) {
				return false;
			}

			//=========================================
			//���b�Z�[�W����
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
	//���b�Z�[�W�ɂ���ď�����I��
	switch (message) {
	case WM_SIZE:
		RECT rcWnd, rcCli;
		GetWindowRect(hWnd, &rcWnd); // �E�B���h�E��RECT�擾
		GetClientRect(hWnd, &rcCli); // �N���C�A���g�̈��RECT�擾
		m_WindowWidth = rcCli.right;
		m_WindowHeight = rcCli.bottom;

		break;
	// �z�C�[���X�N���[����
	case WM_MOUSEWHEEL:
		{
			m_MouseWheelVal = (short)HIWORD(wParam);
		}
		break;
	// X�{�^���������ꂽ
	case WM_CLOSE:
		// �j��
		Release();
		break;
	//�E�B���h�E����Ƃ�
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		//�s�v�ȃ��b�Z�[�W�̌�n��
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Window::SetClientSize(HWND hWnd, int w, int h) const
{
	RECT rcWnd, rcCli;

	GetWindowRect(hWnd, &rcWnd); // �E�B���h�E��RECT�擾
	GetClientRect(hWnd, &rcCli); // �N���C�A���g�̈��RECT�擾

								 // �E�B���h�E�̗]�����l���āA�N���C�A���g�̃T�C�Y���w��T�C�Y�ɂ���B
	MoveWindow(hWnd,
		rcWnd.left,// X���W
		rcWnd.top,// Y���W
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

	//�^�C�}�[�̍ŏ����x��߂�
	timeEndPeriod(1);

#ifdef _DEBUG
	// �R���\�[�����
	FreeConsole();
#endif
}
