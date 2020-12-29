#include "Direct3D.h"
#include "Window.h"

#include <d3dcompiler.h>//�V�F�[�_�R���p�C���p

#pragma comment(lib,"d3dcompiler.lib")

b8 Direct3D::Initialize()
{
	UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
	//�f�o�b�O���C���[��L����
	ID3D12Debug* debugLayer = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
	{
		debugLayer->EnableDebugLayer();
		debugLayer->Release();

		//�f�o�b�O���C���[��L���ɂ���
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	HRESULT hr;

	//DirectX12�܂�菉����

	//DXGI Factory�̐���
	ComPtr<IDXGIFactory6> factory;
	hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
	if (FAILED(hr))
	{
		return false;
	}

	//�n�[�h�E�F�A�A�_�v�^�̎擾
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &hardwareAdapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		hardwareAdapter->GetDesc1(&desc);

		//�\�t�g�E�F�A�f�o�C�X�������ꍇ�R���e�j���[
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		//�A�_�v�^�[��DirectX12���T�|�[�g���Ă��邩�ǂ������m�F
		//�f�o�C�X���쐬���Ă݂Đ���������g�p�\(���ۂɎg���f�o�C�X�ł͂Ȃ�)
		if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	//�f�o�C�X�̍쐬
	hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateDevice failed.");
	}

	//�R�}���h�A���P�[�^�ƃR�}���h���X�g�쐬
	hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
	//�R�}���h�L���[�쐬
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//�^�C���A�E�g�Ȃ�
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//�v���C�I���e�B���Ɏw��Ȃ�
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//�����̓R�}���h���X�g�ƍ��킹�Ă�������
	hr = m_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_commandQueue));//�R�}���h�L���[����

	//�X���b�v�`�F�C���쐬
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
	//IDXGISwapChain4 �擾
	swapChain.As(&m_swapChain);

	//�f�B�X�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//�����_�[�^�[�Q�b�g�r���[�Ȃ̂œ��RRTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;//�\���̂Q��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//���Ɏw��Ȃ�
	hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap));

	//�X���b�v�`�F�C���̏��擾�i������ō쐬���Ă��邩�璼�ڌ���邯�ǂ������������������Ƃ����Ӗ��Łc�j
	DXGI_SWAP_CHAIN_DESC1 swcDesc = {};
	hr = m_swapChain->GetDesc1(&swcDesc);

	//RTV�쐬
	m_backBuffers.resize(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < swcDesc.BufferCount; ++i) {
		hr = m_swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&m_backBuffers[i]));
		m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);
		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//�t�F���X�쐬
	hr = m_device->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));


	//�e�X�g�p������

	return true;
}

b8 Direct3D::TestInit()
{
	HRESULT result;

	//�e�N�X�`��������
	InitTexture();

	//���_������
	Vertex vertices[] = 
	{
		{{-0.4f	,-0.7f	,0.0f}, {0.0f, 1.0f}},//����
		{{-0.4f	,0.7f	,0.0f}, {0.0f, 0.0f}},//����
		{{0.4f	,-0.7f	,0.0f}, {1.0f, 1.0f}},//�E��
		{{0.4f	,0.7f	,0.0f}, {1.0f, 0.0f}},//����
	};

	//�C���f�b�N�X
	unsigned int indices[] =
	{
		0,1,2,
		2,1,3
	};

	//�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//���\�[�X���
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

	//���_�o�b�t�@���\�[�X�쐬
	m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuf));

	//�}�b�v
	Vertex* vertexMap = nullptr;
	m_vertexBuf->Map(0, nullptr, (void**)&vertexMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexMap);
	m_vertexBuf->Unmap(0, nullptr);

	//���_�o�b�t�@�r���[�쐬
	m_vbView.BufferLocation = m_vertexBuf->GetGPUVirtualAddress();//�o�b�t�@�[�̉��z�A�h���X
	m_vbView.SizeInBytes = sizeof(vertices);//�S�o�C�g
	m_vbView.StrideInBytes = sizeof(vertices[0]);//1���_������

	//�C���f�b�N�X�o�b�t�@�쐬
	resDesc.Width = sizeof(indices);
	m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_indexBuf));

	//�}�b�v
	unsigned int* map = nullptr;
	m_indexBuf->Map(0, nullptr, (void**)&map);
	std::copy(std::begin(indices), std::end(indices), map);
	m_indexBuf->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	m_indexView.BufferLocation = m_indexBuf->GetGPUVirtualAddress();
	m_indexView.Format = DXGI_FORMAT_R32_UINT;
	m_indexView.SizeInBytes = sizeof(indices);

	//�V�F�[�_�R���p�C��
	ComPtr<ID3DBlob> errorBlob = nullptr;
	result = D3DCompileFromFile(L"hlsl/BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_vsBlob, &errorBlob);
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			printf("�t�@�C����������܂���ł���");
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
			printf("�t�@�C����������܂���ł���");
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

	//���_���C�A�E�g�쐬
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	//���[�g�V�O�l�`���[�쐬
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	result = m_device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

	//�O���t�B�b�N�p�C�v���C���X�e�[�g�쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	//���[�g�V�O�l�`���Z�b�g
	gpsDesc.pRootSignature = m_rootSignature.Get();

	//�V�F�[�_�Z�b�g
	gpsDesc.VS.pShaderBytecode = m_vsBlob->GetBufferPointer();
	gpsDesc.VS.BytecodeLength = m_vsBlob->GetBufferSize();
	gpsDesc.PS.pShaderBytecode = m_psBlob->GetBufferPointer();
	gpsDesc.PS.BytecodeLength = m_psBlob->GetBufferSize();

	//�T���v���}�X�N
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//�u�����h�X�e�[�g
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;//�u�����h�Ȃ�
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;//�������ނƂ��̃}�X�N�iRGBA�ɒl���������ނ��ǂ����BALL�͂��ׂĂ̐F�ɏ������ށj
	renderTargetBlendDesc.LogicOpEnable = false;//�ЂƂ܂��_�����Z�͎g�p���Ȃ�

	gpsDesc.BlendState.AlphaToCoverageEnable = false;//�A���t�@�J�o���b�W�I�t
	gpsDesc.BlendState.IndependentBlendEnable = false;//������true�ɂ���ƍő�8����RT�Ɍʐݒ肪�ł���
	gpsDesc.BlendState.RenderTarget[0] = renderTargetBlendDesc;//

	//���X�^���C�U�[�X�e�[�g
	gpsDesc.RasterizerState.MultisampleEnable = false;//�}���`�T���v�����OOFF
	gpsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����OOFF
	gpsDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//�h��Ԃ�
	gpsDesc.RasterizerState.DepthClipEnable = true;//�[�x�����̃N���b�s���O�L��

	gpsDesc.RasterizerState.FrontCounterClockwise = false;
	gpsDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpsDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpsDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpsDesc.RasterizerState.AntialiasedLineEnable = false;
	gpsDesc.RasterizerState.ForcedSampleCount = 0;
	gpsDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//�f�v�X�X�e���V���X�e�[�g
	gpsDesc.DepthStencilState.DepthEnable = false;
	gpsDesc.DepthStencilState.StencilEnable = false;

	//�C���v�b�g���C�A�E�g
	gpsDesc.InputLayout.pInputElementDescs = inputLayout;//���C�A�E�g�擪�A�h���X
	gpsDesc.InputLayout.NumElements = _countof(inputLayout);//���C�A�E�g�z��

	//�v���~�e�B�u�g�|���W�^�C�v�i�O�p�`�A���A�_������j
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

	//�����_�[�^�[�Q�b�g�̐ݒ�
	gpsDesc.NumRenderTargets = 1;//1��
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//�t�H�[�}�b�g

	//�T���v���ݒ�(AA�̂��߂̐ݒ�B����͂Ȃ��Ȃ̂ňȉ��̐ݒ�j
	gpsDesc.SampleDesc.Count = 1;//�T���v����1�s�N�Z���ɂ�1
	gpsDesc.SampleDesc.Quality = 0;//�N�I���e�B�Œ�

	//�g���C�A���O���X�g���b�v���Ɏg���ݒ�i�قڎg��Ȃ��j
	gpsDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�

	//�O���t�B�b�N�p�C�v���C���X�e�[�g�I�u�W�F�N�g����
	result = m_device->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&m_pipelineState));

	//�r���[�|�[�g�ƃV�U�[
	m_viewport.Width = WIN.GetWindowWidth();//�o�͐�̕�(�s�N�Z����)
	m_viewport.Height = WIN.GetWindowHeight();//�o�͐�̍���(�s�N�Z����)
	m_viewport.TopLeftX = 0;//�o�͐�̍�����WX
	m_viewport.TopLeftY = 0;//�o�͐�̍�����WY
	m_viewport.MaxDepth = 1.0f;//�[�x�ő�l
	m_viewport.MinDepth = 0.0f;//�[�x�ŏ��l

	m_scissor.left = 0;//�؂蔲�������W
	m_scissor.top = 0;//�؂蔲������W
	m_scissor.right = WIN.GetWindowWidth();//�؂蔲���E���W
	m_scissor.bottom = WIN.GetWindowHeight();//�؂蔲�������W

	return true;
}

void Direct3D::Update()
{
}

void Direct3D::Draw()
{
	u32 bbIdx = m_swapChain->GetCurrentBackBufferIndex();

	//���\�[�X�o���A�ύX
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = m_backBuffers[bbIdx].Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_commandList->ResourceBarrier(1, &BarrierDesc);

	//�����_�[�^�[�Q�b�g�w��
	auto rtvH = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	m_commandList->OMSetRenderTargets(1, &rtvH, false, nullptr);

	//��ʃN���A
	float clearColor[] = { 0.0f,0.0f,0.0f,1.0f };//���F
	m_commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	//�`�揀��
	m_commandList->SetPipelineState(m_pipelineState.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissor);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vbView);

	//�`��
	m_commandList->IASetIndexBuffer(&m_indexView);
	m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	//m_commandList->DrawInstanced(3, 1, 0, 0);

	//���\�[�X�o���A�ύX
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_commandList->ResourceBarrier(1, &BarrierDesc);

	//���߂̃N���[�Y
	m_commandList->Close();

	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, cmdlists);

	//�R�}���h�I���҂�
	m_commandQueue->Signal(m_fence.Get(), ++m_fenceVal);

	if (m_fence->GetCompletedValue() != m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//�R�}���h�N���A
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	//��ʃt���b�v
	m_swapChain->Present(1, 0);
}

void Direct3D::InitTexture()
{
	//�e�N�X�`���쐬
	m_textreData.resize(256 * 256);
	for (auto&& tex : m_textreData)
	{
		tex.R = rand() % 256;
		tex.G = rand() % 256;
		tex.B = rand() % 256;
		tex.A = 256;
	}

	//�e�N�X�`���o�b�t�@�쐬

}

void Direct3D::Release()
{
	//�I���O��GPU�̏������I���̂�҂�
	m_commandQueue->Signal(m_fence.Get(), ++m_fenceVal);
	if (m_fence->GetCompletedValue() != m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//�t���X�N���[���̏ꍇ�͂��Ƃɖ߂�
	BOOL isFullScreen;
	m_swapChain->GetFullscreenState(&isFullScreen, nullptr);
	if (isFullScreen)
	{
		m_swapChain->SetFullscreenState(FALSE, nullptr);
	}
	m_backBuffers.clear();
}
