#include "Direct3D.h"
#include "Window.h"

#include <d3dcompiler.h>//シェーダコンパイル用

#pragma comment(lib,"d3dcompiler.lib")

b8 Direct3D::Initialize()
{
	UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
	//デバッグレイヤーを有効化
	ID3D12Debug* debugLayer = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
	{
		debugLayer->EnableDebugLayer();
		debugLayer->Release();

		//デバッグレイヤーを有効にする
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	HRESULT hr;

	//DirectX12まわり初期化

	//DXGI Factoryの生成
	ComPtr<IDXGIFactory6> factory;
	hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
	if (FAILED(hr))
	{
		return false;
	}

	//ハードウェアアダプタの取得
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &hardwareAdapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		hardwareAdapter->GetDesc1(&desc);

		//ソフトウェアデバイスだった場合コンテニュー
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		//アダプターがDirectX12をサポートしているかどうかを確認
		//デバイスを作成してみて成功したら使用可能(実際に使うデバイスではない)
		if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	//デバイスの作成
	hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateDevice failed.");
	}

	//コマンドアロケータとコマンドリスト作成
	hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
	//コマンドキュー作成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//タイムアウトなし
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//プライオリティ特に指定なし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//ここはコマンドリストと合わせてください
	hr = m_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_commandQueue));//コマンドキュー生成

	//スワップチェイン作成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = WIN.GetWindowWidth();
	swapchainDesc.Height = WIN.GetWindowHeight();
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1> swapChain;
	hr = factory->CreateSwapChainForHwnd(m_commandQueue.Get(), WIN.GethWnd(), &swapchainDesc, nullptr, nullptr, &swapChain);
	//IDXGISwapChain4 取得
	swapChain.As(&m_swapChain);

	//ディスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダーターゲットビューなので当然RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;//表裏の２つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//特に指定なし
	hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap));

	//スワップチェインの情報取得（すぐ上で作成しているから直接見れるけどこういうやり方もあるよという意味で…）
	DXGI_SWAP_CHAIN_DESC1 swcDesc = {};
	hr = m_swapChain->GetDesc1(&swcDesc);

	//RTV作成
	m_backBuffers.resize(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < swcDesc.BufferCount; ++i) {
		hr = m_swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&m_backBuffers[i]));
		m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);
		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//フェンス作成
	hr = m_device->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));


	//テスト用初期化

	return true;
}

b8 Direct3D::TestInit()
{
	HRESULT result;

	//テクスチャ初期化
	InitTexture();

	//頂点初期化
	Vertex vertices[] = 
	{
		{{-0.4f	,-0.7f	,0.0f}, {0.0f, 1.0f}},//左下
		{{-0.4f	,0.7f	,0.0f}, {0.0f, 0.0f}},//左上
		{{0.4f	,-0.7f	,0.0f}, {1.0f, 1.0f}},//右下
		{{0.4f	,0.7f	,0.0f}, {1.0f, 0.0f}},//左下
	};

	//インデックス
	unsigned int indices[] =
	{
		0,1,2,
		2,1,3
	};

	//ヒープ設定
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//リソース情報
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeof(vertices);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//頂点バッファリソース作成
	m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuf));

	//マップ
	Vertex* vertexMap = nullptr;
	m_vertexBuf->Map(0, nullptr, (void**)&vertexMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexMap);
	m_vertexBuf->Unmap(0, nullptr);

	//頂点バッファビュー作成
	m_vbView.BufferLocation = m_vertexBuf->GetGPUVirtualAddress();//バッファーの仮想アドレス
	m_vbView.SizeInBytes = sizeof(vertices);//全バイト
	m_vbView.StrideInBytes = sizeof(vertices[0]);//1頂点当たり

	//インデックスバッファ作成
	resDesc.Width = sizeof(indices);
	m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_indexBuf));

	//マップ
	unsigned int* map = nullptr;
	m_indexBuf->Map(0, nullptr, (void**)&map);
	std::copy(std::begin(indices), std::end(indices), map);
	m_indexBuf->Unmap(0, nullptr);

	//インデックスバッファビューを作成
	m_indexView.BufferLocation = m_indexBuf->GetGPUVirtualAddress();
	m_indexView.Format = DXGI_FORMAT_R32_UINT;
	m_indexView.SizeInBytes = sizeof(indices);

	//シェーダコンパイル
	ComPtr<ID3DBlob> errorBlob = nullptr;
	result = D3DCompileFromFile(L"hlsl/BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_vsBlob, &errorBlob);
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			printf("ファイルが見つかりませんでした");
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			printf(errstr.c_str());
		}
		return false;
	}

	result = D3DCompileFromFile(L"hlsl/BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_psBlob, &errorBlob);
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			printf("ファイルが見つかりませんでした");
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			printf(errstr.c_str());
		}
		return false;
	}

	//頂点レイアウト作成
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	//ルートシグネチャー作成
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	result = m_device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

	//グラフィックパイプラインステート作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	//ルートシグネチャセット
	gpsDesc.pRootSignature = m_rootSignature.Get();

	//シェーダセット
	gpsDesc.VS.pShaderBytecode = m_vsBlob->GetBufferPointer();
	gpsDesc.VS.BytecodeLength = m_vsBlob->GetBufferSize();
	gpsDesc.PS.pShaderBytecode = m_psBlob->GetBufferPointer();
	gpsDesc.PS.BytecodeLength = m_psBlob->GetBufferSize();

	//サンプルマスク
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//ブレンドステート
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;//ブレンドなし
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;//書き込むときのマスク（RGBAに値を書き込むかどうか。ALLはすべての色に書き込む）
	renderTargetBlendDesc.LogicOpEnable = false;//ひとまず論理演算は使用しない

	gpsDesc.BlendState.AlphaToCoverageEnable = false;//アルファカバレッジオフ
	gpsDesc.BlendState.IndependentBlendEnable = false;//こいつをtrueにすると最大8このRTに個別設定ができる
	gpsDesc.BlendState.RenderTarget[0] = renderTargetBlendDesc;//

	//ラスタライザーステート
	gpsDesc.RasterizerState.MultisampleEnable = false;//マルチサンプリングOFF
	gpsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//カリングOFF
	gpsDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//塗りつぶし
	gpsDesc.RasterizerState.DepthClipEnable = true;//深度方向のクリッピング有効

	gpsDesc.RasterizerState.FrontCounterClockwise = false;
	gpsDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpsDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpsDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpsDesc.RasterizerState.AntialiasedLineEnable = false;
	gpsDesc.RasterizerState.ForcedSampleCount = 0;
	gpsDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//デプスステンシルステート
	gpsDesc.DepthStencilState.DepthEnable = false;
	gpsDesc.DepthStencilState.StencilEnable = false;

	//インプットレイアウト
	gpsDesc.InputLayout.pInputElementDescs = inputLayout;//レイアウト先頭アドレス
	gpsDesc.InputLayout.NumElements = _countof(inputLayout);//レイアウト配列数

	//プリミティブトポロジタイプ（三角形、線、点がある）
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形で構成

	//レンダーターゲットの設定
	gpsDesc.NumRenderTargets = 1;//1枚
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//フォーマット

	//サンプル設定(AAのための設定。今回はなしなので以下の設定）
	gpsDesc.SampleDesc.Count = 1;//サンプルは1ピクセルにつき1
	gpsDesc.SampleDesc.Quality = 0;//クオリティ最低

	//トライアングルストリップ時に使う設定（ほぼ使わない）
	gpsDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//ストリップ時のカットなし

	//グラフィックパイプラインステートオブジェクト生成
	result = m_device->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&m_pipelineState));

	//ビューポートとシザー
	m_viewport.Width = WIN.GetWindowWidth();//出力先の幅(ピクセル数)
	m_viewport.Height = WIN.GetWindowHeight();//出力先の高さ(ピクセル数)
	m_viewport.TopLeftX = 0;//出力先の左上座標X
	m_viewport.TopLeftY = 0;//出力先の左上座標Y
	m_viewport.MaxDepth = 1.0f;//深度最大値
	m_viewport.MinDepth = 0.0f;//深度最小値

	m_scissor.left = 0;//切り抜き下座標
	m_scissor.top = 0;//切り抜き上座標
	m_scissor.right = WIN.GetWindowWidth();//切り抜き右座標
	m_scissor.bottom = WIN.GetWindowHeight();//切り抜き下座標

	return true;
}

void Direct3D::Update()
{
}

void Direct3D::Draw()
{
	u32 bbIdx = m_swapChain->GetCurrentBackBufferIndex();

	//リソースバリア変更
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = m_backBuffers[bbIdx].Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_commandList->ResourceBarrier(1, &BarrierDesc);

	//レンダーターゲット指定
	auto rtvH = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	m_commandList->OMSetRenderTargets(1, &rtvH, false, nullptr);

	//画面クリア
	float clearColor[] = { 0.0f,0.0f,0.0f,1.0f };//黄色
	m_commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	//描画準備
	m_commandList->SetPipelineState(m_pipelineState.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissor);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vbView);

	//描画
	m_commandList->IASetIndexBuffer(&m_indexView);
	m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	//m_commandList->DrawInstanced(3, 1, 0, 0);

	//リソースバリア変更
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_commandList->ResourceBarrier(1, &BarrierDesc);

	//命令のクローズ
	m_commandList->Close();

	//コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, cmdlists);

	//コマンド終了待ち
	m_commandQueue->Signal(m_fence.Get(), ++m_fenceVal);

	if (m_fence->GetCompletedValue() != m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//コマンドクリア
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	//画面フリップ
	m_swapChain->Present(1, 0);
}

void Direct3D::InitTexture()
{
	//テクスチャ作成
	m_textreData.resize(256 * 256);
	for (auto&& tex : m_textreData)
	{
		tex.R = rand() % 256;
		tex.G = rand() % 256;
		tex.B = rand() % 256;
		tex.A = 256;
	}

	//テクスチャバッファ作成

}

void Direct3D::Release()
{
	//終了前にGPUの処理が終わるのを待つ
	m_commandQueue->Signal(m_fence.Get(), ++m_fenceVal);
	if (m_fence->GetCompletedValue() != m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//フルスクリーンの場合はもとに戻す
	BOOL isFullScreen;
	m_swapChain->GetFullscreenState(&isFullScreen, nullptr);
	if (isFullScreen)
	{
		m_swapChain->SetFullscreenState(FALSE, nullptr);
	}
	m_backBuffers.clear();
}
