#include "D3DGraphics.h"

#define _DEBUG

D3DGraphics Graphics;

struct Vertex	//Overloaded Vertex Structure
{
	Vertex(){}
	Vertex(float x, float y, float z,
		 float u, float v)
		: pos(x,y,z), texCoord(u, v){}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
};

struct perObject
{
	XMMATRIX WVP;
};

perObject perObj;

float rot = 0.01f;

D3DGraphics::D3DGraphics() : m_pSwapChain(NULL), m_pDevice(NULL), m_pDeviceContext(NULL), m_pVertBuffer(NULL),
							 VS(NULL), PS(NULL), InputLayout(NULL), VertexBlob(NULL), PixelBlob(NULL), depthStencilView(NULL), 
							 depthStencilBuffer(NULL), WireFrame(NULL), cubeTexture(NULL), cubeSampleState(NULL),
							 CCCullmode(NULL), CWCullmode(NULL), Transparency(NULL), noCull(NULL) {}

bool D3DGraphics::RunDirectX(HWND hwnd, int width, int height, bool fullscreen, bool wireframe)
{
	DXGI_SWAP_CHAIN_DESC dc;
	HRESULT result;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ZeroMemory(&dc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dc.BufferCount = 1;
	dc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dc.BufferDesc.Height = height;
	dc.BufferDesc.Width = width;
	dc.BufferDesc.RefreshRate.Numerator = 60;
	dc.BufferDesc.RefreshRate.Denominator = 1;
	dc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dc.OutputWindow = hwnd;
	dc.SampleDesc.Count = 1;
	dc.SampleDesc.Quality = 0;
	dc.Windowed = fullscreen;

	D3D_DRIVER_TYPE Drivers[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_SOFTWARE,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT NumDrivers = ARRAYSIZE(Drivers);

	D3D_FEATURE_LEVEL FeatureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	UINT FeatureSize = ARRAYSIZE(FeatureLevels);

	for( UINT index = 0; index < NumDrivers; index++)
	{
		m_pDriverType = Drivers[index];

		result = D3D11CreateDeviceAndSwapChain(NULL, m_pDriverType, NULL, createDeviceFlags, FeatureLevels, FeatureSize, D3D11_SDK_VERSION, &dc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext);
		if(SUCCEEDED(result))
			break;
	}
	if(FAILED(result))
		return false;

	m_pHWND = hwnd;

	D3D11_TEXTURE2D_DESC depthBufferDesc;

	depthBufferDesc.Height = height;
	depthBufferDesc.Width = width;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	m_pDevice->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);
	m_pDevice->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	ID3D11Texture2D *pBackBuffer;
	if(FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
		return S_FALSE;
	result = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTarget);
	pBackBuffer->Release();
	if(FAILED(result))
		return S_FALSE;
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTarget, depthStencilView);

	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(perObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	if(FAILED(m_pDevice->CreateBuffer(&cbbd, NULL, &perObjectBuffer)))
		return S_FALSE;
	
	//cam settings
	camPos = XMVectorSet(0.0f, 3.0f, -10.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	camView = XMMatrixLookAtLH(camPos, camTarget, camUp);

	camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, (float)width/height, 1.0f, 1000.0f);

	if(FAILED(D3DX11CreateShaderResourceViewFromFile(m_pDevice, "pls.jpg", NULL, NULL, &cubeTexture, NULL)))
		return S_FALSE;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	

	if(FAILED(m_pDevice->CreateSamplerState(&sampDesc, &cubeSampleState)))
		return S_FALSE;

	//Useful for debug. Render the cubes in wireframe state.
	if(wireframe)
	{
		D3D11_RASTERIZER_DESC wfdesc;
		ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));

		wfdesc.FillMode = D3D11_FILL_WIREFRAME;
		wfdesc.CullMode = D3D11_CULL_NONE;

		if(FAILED(m_pDevice->CreateRasterizerState(&wfdesc, &WireFrame)))
			return S_FALSE;

		m_pDeviceContext->RSSetState(WireFrame);
	}

	D3D11_RASTERIZER_DESC Rastdesc;
	ZeroMemory(&Rastdesc, sizeof(D3D11_RASTERIZER_DESC));

	Rastdesc.FillMode = D3D11_FILL_SOLID;
	Rastdesc.CullMode = D3D11_CULL_NONE;

	m_pDevice->CreateRasterizerState(&Rastdesc, &noCull);
	
	D3D11_VIEWPORT vp;
	vp.Height = (FLOAT)height;
	vp.Width = (FLOAT)width;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &vp);

	if(!InitScene())
	{
		MessageBox(NULL, "Failed to create scene!", "ERROR", MB_OK);
		DestroyWindow(hwnd);
		return false;
	}

	SetActiveWindow(hwnd);

	return true;
}	

bool D3DGraphics::InitScene()
{
	
	ID3D11Buffer* squareIndexBuffer;
	
	ID3DBlob *Errorblob = NULL;
	if(FAILED(D3DX11CompileFromFile("Effects.fx", 0, 0, "VS", "vs_5_0", 0, 0, 0, &VertexBlob, &Errorblob, 0)))
	{
		if(Errorblob != NULL)
		{
			OutputDebugStringA( (char*)Errorblob->GetBufferPointer() );
		}
		if(Errorblob)
			Errorblob->Release();
		return false;
	}

	if(FAILED(m_pDevice->CreateVertexShader(VertexBlob->GetBufferPointer(), VertexBlob->GetBufferSize(), NULL, &VS)))
		return false;

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT NumLayout = ARRAYSIZE(layout);

	if(FAILED(m_pDevice->CreateInputLayout(layout, NumLayout, VertexBlob->GetBufferPointer(), VertexBlob->GetBufferSize(), &InputLayout)))
		return false;

	m_pDeviceContext->IASetInputLayout(InputLayout);

	if(FAILED(D3DX11CompileFromFile("Effects.fx", 0, 0, "PS", "ps_5_0", 0, 0, 0, &PixelBlob, &Errorblob, 0)))
	{
		if(Errorblob != NULL)
		{
			OutputDebugStringA( (char*)Errorblob->GetBufferPointer() );
		}
		if(Errorblob)
			Errorblob->Release();
		return false;
	}
	
	if(FAILED(m_pDevice->CreatePixelShader(PixelBlob->GetBufferPointer(), PixelBlob->GetBufferSize(), NULL, &PS)))
		return false;

	Vertex v[] =
	{
		// Front Face
		Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f),

		// Back Face
		Vertex(-1.0f, -1.0f, 1.0f, 1.0f, 1.0f),
		Vertex( 1.0f, -1.0f, 1.0f, 0.0f, 1.0f),
		Vertex( 1.0f,  1.0f, 1.0f, 0.0f, 0.0f),
		Vertex(-1.0f,  1.0f, 1.0f, 1.0f, 0.0f),

		// Top Face
		Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f),
		Vertex(-1.0f, 1.0f,  1.0f, 0.0f, 0.0f),
		Vertex( 1.0f, 1.0f,  1.0f, 1.0f, 0.0f),
		Vertex( 1.0f, 1.0f, -1.0f, 1.0f, 1.0f),

		// Bottom Face
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f),
		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 0.0f),
		Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 0.0f),

		// Left Face
		Vertex(-1.0f, -1.0f,  1.0f, 0.0f, 1.0f),
		Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f),
		Vertex(-1.0f,  1.0f, -1.0f, 1.0f, 0.0f),
		Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f),

		// Right Face
		Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f),
		Vertex( 1.0f,  1.0f, -1.0f, 0.0f, 0.0f),
		Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f),
		Vertex( 1.0f, -1.0f,  1.0f, 1.0f, 1.0f),
	};

	D3D11_BUFFER_DESC VertexBuffer;
	ZeroMemory(&VertexBuffer, sizeof(VertexBuffer));

	VertexBuffer.Usage = D3D11_USAGE_DEFAULT;
	VertexBuffer.ByteWidth = sizeof(Vertex) * 24;
	VertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBuffer.CPUAccessFlags = 0;
	VertexBuffer.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA VertexData;
	ZeroMemory(&VertexData, sizeof(VertexData));

	VertexData.pSysMem = v;

	if(FAILED(m_pDevice->CreateBuffer(&VertexBuffer, &VertexData, &m_pVertBuffer)))
		return false;

	DWORD indices[] = {
		// Front Face
		0,  1,  2,
		0,  2,  3,

		// Back Face
		4,  5,  6,
		4,  6,  7,

		// Top Face
		8,  9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory( &indexBufferDesc, sizeof(indexBufferDesc) );

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 12 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices;
	m_pDevice->CreateBuffer(&indexBufferDesc, &iinitData, &squareIndexBuffer);

	m_pDeviceContext->IASetIndexBuffer( squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	UINT Stride = sizeof(Vertex);
	UINT offset = 0;

	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertBuffer, &Stride, &offset);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_BLEND_DESC blenddesc;
	ZeroMemory(&blenddesc, sizeof(D3D11_BLEND_DESC));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blenddesc.AlphaToCoverageEnable = false;
	blenddesc.RenderTarget[0] = rtbd;

	m_pDevice->CreateBlendState(&blenddesc, &Transparency);

	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;

	if (FAILED(m_pDevice->CreateRasterizerState(&cmdesc, &CCCullmode)))
		return S_FALSE;

	cmdesc.FrontCounterClockwise = false;

	if (FAILED(m_pDevice->CreateRasterizerState(&cmdesc, &CWCullmode)))
		return S_FALSE;

	return true;
}

void D3DGraphics::UpdateScene()
{
	//Keep the cube spinning.
	rot += .0005f;
	if(rot > 6.28f)
		rot = 0.0f;

	cube1 = XMMatrixIdentity();

	//cube1 world space and matrix

	//circle round cube2 (source point)
	XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//rotate the cube from the vector given
	Rotation = XMMatrixRotationAxis(rotaxis, rot);

	//move the cube
	Translation = XMMatrixTranslation(0.0f, 0.0f, 4.0f);

	cube1 = Translation * Rotation;
	
	//clear the cube2 matrix
	cube2 = XMMatrixIdentity();
	
	//assign rotaxis a new value. cube2 will rotate on it's x and z axis to give a spinning effect
	rotaxis = XMVectorSet(1.0f, 0.0f, 1.0f, 0.0f);

	//rotate the cube from the vector given
	Rotation = XMMatrixRotationAxis(rotaxis, rot);

	Scale = XMMatrixScaling(1.3f, 1.3f, 1.3f);

	cube2 = Rotation * Scale;
	
}

void D3DGraphics::Render()
{
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTarget, ClearColor);
	m_pDeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);


	m_pDeviceContext->RSSetState(NULL);
	//render objects using backface culling

	m_pDeviceContext->RSSetState(noCull);

	float blendfactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };

	m_pDeviceContext->OMSetBlendState(0, 0, 0xffffffff);

	//opaque objects

	m_pDeviceContext->OMSetBlendState(Transparency, blendfactor, 0xffffffff);

	XMVECTOR cubepos = XMVectorZero();

	cubepos = XMVector3TransformCoord(cubepos, cube1);

	float distx = XMVectorGetX(cubepos) - XMVectorGetX(camPos);
	float disty = XMVectorGetY(cubepos) - XMVectorGetY(camPos);
	float distz = XMVectorGetZ(cubepos) - XMVectorGetZ(camPos);

	float cube1Pos = distx*distx + disty*disty + distz*distz;


	cubepos = XMVectorZero();

	cubepos = XMVector3TransformCoord(cubepos, cube2);

	distx = XMVectorGetX(cubepos) - XMVectorGetX(camPos);
	disty = XMVectorGetY(cubepos) - XMVectorGetY(camPos);
	distz = XMVectorGetZ(cubepos) - XMVectorGetZ(camPos);

	float cube2Pos = distx*distx + disty*disty + distz*distz;


	if (cube1Pos < cube2Pos)
	{
		//cube one is in front
		XMMATRIX temp = cube1;
		cube1 = cube2;
		cube2 = temp;
	}

	m_pDeviceContext->VSSetShader( VS, NULL, 0 );
	m_pDeviceContext->PSSetShader( PS, NULL, 0 );

	WVP = cube1 * camView * camProjection;

	perObj.WVP = XMMatrixTranspose(WVP);

	m_pDeviceContext->UpdateSubresource(perObjectBuffer, 0, NULL, &perObj, 0, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &perObjectBuffer);

	m_pDeviceContext->PSSetShaderResources(0, 1, &cubeTexture);
	m_pDeviceContext->PSSetSamplers(0, 1, &cubeSampleState);

	m_pDeviceContext->RSSetState(CCCullmode);

	m_pDeviceContext->DrawIndexed( 36, 0, 0 );
	
	m_pDeviceContext->RSSetState(CWCullmode);

	m_pDeviceContext->DrawIndexed(36, 0, 0);

	WVP = cube2 * camView * camProjection;

	perObj.WVP = XMMatrixTranspose(WVP);

	m_pDeviceContext->UpdateSubresource(perObjectBuffer, 0, NULL, &perObj, 0, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &perObjectBuffer);

	m_pDeviceContext->PSSetShaderResources(0, 1, &cubeTexture);
	m_pDeviceContext->PSSetSamplers(0, 1, &cubeSampleState);

	m_pDeviceContext->RSSetState(CCCullmode);

	m_pDeviceContext->DrawIndexed(36, 0, 0);

	m_pDeviceContext->RSSetState(CWCullmode);

	m_pDeviceContext->DrawIndexed( 36, 0, 0 );

	m_pSwapChain->Present(0, 0);

}

void D3DGraphics::CleanUp()
{
	Memory::Release(m_pRenderTarget);
	Memory::Release(m_pDeviceContext);
	Memory::Release(m_pDevice);
	Memory::Release(m_pSwapChain);
	Memory::Release(m_pVertBuffer);
	Memory::Release(VS);
	Memory::Release(PS);
	Memory::Release(VertexBlob);
	Memory::Release(PixelBlob);
	Memory::Release(InputLayout);
	Memory::Release(depthStencilView);
	Memory::Release(depthStencilBuffer);
	Memory::Release(perObjectBuffer);
	Memory::Release(WireFrame);
	Memory::Release(cubeTexture);
	Memory::Release(cubeSampleState);
	Memory::Release(CCCullmode);
	Memory::Release(CWCullmode);
	Memory::Release(Transparency);
	Memory::Release(noCull);
}