#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <D3Dcompiler.h>
#include <xnamath.h>
#include "DXUtil.cpp"

class D3DGraphics {

	public:
		D3DGraphics();
		bool RunDirectX(HWND, int, int, bool, bool);
		void UpdateScene();
		void Render();
		void CleanUp();
		bool InitScene();

	private:
		IDXGISwapChain *m_pSwapChain;
		ID3D11Device *m_pDevice;
		ID3D11DeviceContext *m_pDeviceContext;
		D3D_DRIVER_TYPE m_pDriverType;
		D3D_FEATURE_LEVEL m_pFeatureLevel;
		ID3D11RenderTargetView *m_pRenderTarget;
		HWND m_pHWND;
		ID3D11Buffer *m_pVertBuffer;
		ID3D11VertexShader *VS;
		ID3D11PixelShader *PS;
		ID3D10Blob *VertexBlob;
		ID3D10Blob *PixelBlob;
		ID3D11InputLayout *InputLayout;
		ID3D11DepthStencilView *depthStencilView;
		ID3D11Texture2D *depthStencilBuffer;
		ID3D11Buffer *perObjectBuffer;
		ID3D11RasterizerState *WireFrame;

		XMMATRIX WVP;
		XMMATRIX World;
		XMMATRIX camView;
		XMMATRIX camProjection;
		XMVECTOR camPos;
		XMVECTOR camTarget;
		XMVECTOR camUp;

		XMMATRIX cube1;
		XMMATRIX cube2;

		XMMATRIX Rotation;
		XMMATRIX Scale;
		XMMATRIX Translation;



}; extern D3DGraphics Graphics;