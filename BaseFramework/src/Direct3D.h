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

	//普通は作らないけどテスト用
	b8 TestInit();
	void Update();
	void Draw();

	struct Vertex
	{
		XMFLOAT3 pos;//頂点座標
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

	//デバイス
	ComPtr<ID3D12Device5>  m_device = nullptr;

	//コマンド関連
	ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;
	ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;
	
	//スワップチェイン
	ComPtr<IDXGISwapChain4> m_swapChain = nullptr;

	//ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap = nullptr;

	//バックバッファのBuffer（ちょっと言い方悪いけど、わかりやすいように）
	std::vector<ComPtr<ID3D12Resource>> m_backBuffers;

	//フェンス
	ComPtr<ID3D12Fence> m_fence = nullptr;
	u32 m_fenceVal = 0;

	//頂点
	ComPtr<ID3D12Resource> m_vertexBuf = nullptr;//頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW m_vbView;//頂点バッファ―ビュー
	ComPtr<ID3D12Resource> m_indexBuf = nullptr;//インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW m_indexView;//インデックスバッファビュー

	//シェーダ
	ComPtr<ID3DBlob> m_vsBlob = nullptr;
	ComPtr<ID3DBlob> m_psBlob = nullptr;

	//グラフィックパイプラインステート
	ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;

	//ルートシグネチャ
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	//ビューポートとシザー
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissor;

	//テクスチャ
	std::vector<TexRGBA> m_textreData;
};
