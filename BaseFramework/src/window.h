#pragma once

#pragma comment(lib,"winmm.lib")
#include <Windows.h>

class Window
{
public:
	//�E�B���h�E�n���h���擾
	HWND GethWnd() { return m_hWnd; }
	//�T�C�Y�擾
	u32 GetWindowWidth() { return m_WindowWidth; }
	u32 GetWindowHeight() { return m_WindowHeight; }
	// �}�E�X�z�C�[���̕ω��ʂ��擾
	int GetMouseWheelVal() const { return m_MouseWheelVal; }
	//�E�B���h�E�쐬�ADirect3D�̏������Ȃ�
	bool InitWindow(HINSTANCE hInstance, int w, int h, const char* name);
	//�E�B���h�E�v���V�[�W��
	bool ProcessMessage();
	static LRESULT CALLBACK callWinowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	//�N���C�A���g�T�C�Y�̐ݒ�
	void SetClientSize(HWND hWnd, int w, int h) const;
	//�t���X�N���[���ݒ�
	void SetWindowMode(bool mode)
	{
		m_WindowMode = mode;
	}
	//���
	void Release();
private:
	HINSTANCE m_hInst;//�A�v���P�[�V�����̃C���X�^���X�n���h��
	HWND m_hWnd;//�E�B���h�E�n���h��
	int m_WindowWidth = 0;
	int m_WindowHeight = 0;//�E�B���h�E�N���C�A���g�𑜓x
	DWORD m_FrameCnt = 0;//���݂̃t���[���J�E���g
	int	m_MouseWheelVal = 0;// �}�E�X�z�C�[���l
	int m_Fps = 0;
	bool m_WindowMode = false;
	//�V���O���g��
public:
	static Window& GetInstance()
	{
		static Window Instance;
		return Instance;
	}
private:
	Window() {}//�f�t�H���g�R���X�g���N�^
	Window(Window&);//�R�s�[�R���X�g���N�^
	Window& operator = (Window&);//�����Z�q�̃I�[�o�[���[�h(���)
	~Window() {}//�f�X�g���N�^
};
#define WIN Window::GetInstance()
