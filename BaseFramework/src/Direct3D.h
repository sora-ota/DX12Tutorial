#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace DirectX;

class Direct3D
{
public:
	b8 Initialize();

	void Release();

	//���ʂ͍��Ȃ����ǃe�X�g�p
	b8 TestInit();
	void Update();
	void Draw();

	struct Vertex
	{
		XMFLOAT3 pos;//���_���W
		XMFLOAT2 uv;//UV
	};

	struct TexRGBA
	{
		unsigned char R = 0;
		unsigned char G = 0;
		unsigned char B = 0;
		unsigned char A = 0;
	};

private:
	void InitTexture();

	//�f�o�C�X
	ComPtr<ID3D12Device5>  m_device = nullptr;

	//�R�}���h�֘A
	ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;
	ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;
	
	//�X���b�v�`�F�C��
	ComPtr<IDXGISwapChain4> m_swapChain = nullptr;

	//�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap = nullptr;

	//�o�b�N�o�b�t�@��Buffer�i������ƌ������������ǁA�킩��₷���悤�Ɂj
	std::vector<ComPtr<ID3D12Resource>> m_backBuffers;

	//�t�F���X
	ComPtr<ID3D12Fence> m_fence = nullptr;
	u32 m_fenceVal = 0;

	//���_
	ComPtr<ID3D12Resource> m_vertexBuf = nullptr;//���_�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW m_vbView;//���_�o�b�t�@�\�r���[
	ComPtr<ID3D12Resource> m_indexBuf = nullptr;//�C���f�b�N�X�o�b�t�@
	D3D12_INDEX_BUFFER_VIEW m_indexView;//�C���f�b�N�X�o�b�t�@�r���[

	//�V�F�[�_
	ComPtr<ID3DBlob> m_vsBlob = nullptr;
	ComPtr<ID3DBlob> m_psBlob = nullptr;

	//�O���t�B�b�N�p�C�v���C���X�e�[�g
	ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;

	//���[�g�V�O�l�`��
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	//�r���[�|�[�g�ƃV�U�[
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissor;

	//�e�N�X�`��
	std::vector<TexRGBA> m_textreData;
};
