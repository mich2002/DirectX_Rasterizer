#include "pch.h"
#include "Renderer.h"

#include "Camera.h"
#include "Utils.h"

#include "MeshEffect.h"
#include "TransEffect.h"

//#define USE_TRIANGLE
//#define USE_QUAD
#define USE_VEHICLE

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

#ifdef USE_TRIANGLE
		TriangleMeshInit();

		m_pCamera = new Camera();
		m_pCamera->Initialize(45.f, Vector3{ 0.f,0.f,-10.f }, m_Width / (float)m_Height);
#elif defined(USE_QUAD)
		QuadMeshInit();

		m_pCamera = new Camera();
		m_pCamera->Initialize(45.f, Vector3{ 0.f,0.f,-10.f }, m_Width / (float)m_Height);
#elif defined(USE_VEHICLE)
		VehicleMeshInit();
		CombustionMeshInit();

		m_pCamera = new Camera();
		m_pCamera->Initialize(45.f, Vector3{ 0.f,0.f,-50.f }, m_Width / (float)m_Height);
#endif
	}

	Renderer::~Renderer()
	{
		delete m_pMesh;
		delete m_pTransparentMesh;

		delete m_pCamera;

		//Release resources (to prevent resource leaks)
		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
		}

		if (m_pRenderTargetBuffer)
		{
			m_pRenderTargetBuffer->Release();
		}

		if (m_pDepthStencilView)
			m_pDepthStencilView->Release();

		if (m_pDepthStencilBuffer)
			m_pDepthStencilBuffer->Release();

		if (m_pSwapChain)
			m_pSwapChain->Release();

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		if (m_pDevice)
		{
			m_pDevice->Release();
		}

		if (m_pDXGIFactory)
			m_pDXGIFactory->Release();
	}

	void Renderer::Update(const Timer* pTimer) const
	{
		m_pCamera->Update(pTimer);

		if (m_IsRotating)
		{
			constexpr float rotationSpeed{ 30 * TO_RADIANS };
			const Matrix newWorldMatrix = Matrix::CreateRotationY(rotationSpeed * pTimer->GetElapsed()) * m_pMesh->GetWorldMatrix();
			m_pShadedEffect->SetWorldMatrixVariable(newWorldMatrix);
			m_pMesh->SetWorldMatrix(newWorldMatrix);

			m_pTransparentMesh->SetWorldMatrix(newWorldMatrix);
		}
		m_pMesh->SetWorldViewProjectionMatrix(m_pCamera->GetViewMatrix(), m_pCamera->GetProjectionMatrix());
		m_pShadedEffect->SetInvViewMatrixVariable(m_pCamera->GetInvViewMatrix());

		m_pTransparentMesh->SetWorldViewProjectionMatrix(m_pCamera->GetViewMatrix(), m_pCamera->GetProjectionMatrix());
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		//Clear Buffers
		constexpr auto clearColor = ColorRGB(0.f, 0.f, 0.3f);
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		//Render
		m_pMesh->Render(m_pDeviceContext, m_FilteringMethod);
		m_pTransparentMesh->Render(m_pDeviceContext, m_FilteringMethod);

		//Present
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::ToggleFilteringMethod()
	{
		m_FilteringMethod = Mesh::FilteringMethod{ ((int)m_FilteringMethod + 1) % 3 };
	}

	void Renderer::TriangleMeshInit()
	{
		//Create some data for our mesh
		const std::vector<Vertex> vertices
		{
			// defined in world space
			{Vector3(0.f, 3.f, 2.f), ColorRGB(1.f,0.f,0.f)},
			{Vector3(3.f, -3.f, 2.f), ColorRGB(0.f,0.f,1.f)},
			{Vector3(-3.f, -3.f, 2.f), ColorRGB (0.f,1.f,0.f)}

			//{Vector3(0.f, .5f, .5f), ColorRGB(1.f,0.f,0.f)},
			//{Vector3(.5f, -.5f, .5f), ColorRGB(0.f,0.f,1.f)},
			//{Vector3(-.5f, -.5f, .5f), ColorRGB (0.f,1.f,0.f)}
		};
		const std::vector<uint32_t> indices{ 0,1,2 };

//		m_pMesh = new Mesh{ m_pDevice, vertices, indices };

		const Vector3 position{ 0,0,0 };
		const Vector3 scale{ 1,1,1 };
		
//		m_pMesh->SetWorldMatrix(Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(position));
	}
	void Renderer::QuadMeshInit()
	{
		const std::vector<Vertex> vertices{
			{Vector3{-3,3,-2},	colors::White,	Vector2{0,0}},
			{Vector3{0,3,-2},	colors::White,	Vector2{0.5f, 0}},
			{Vector3{3,3,-2},	colors::White,	Vector2{1,0}},
			{Vector3{-3,0,-2},	colors::White,	Vector2{0,0.5f}},
			{Vector3{0,0,-2},	colors::White,	Vector2{0.5f,0.5f}},
			{Vector3{3,0,-2},	colors::White,	Vector2{1,0.5f}},
			{Vector3{-3,-3,-2},	colors::White,	Vector2{0,1}},
			{Vector3{0,-3,-2},	colors::White,	Vector2{0.5f,1}},
			{Vector3{3,-3,-2},	colors::White,	Vector2{1, 1}}
		};
		const std::vector<uint32_t> indices{ 3,0,1, 1,4,3, 4,1,2, 2,5,4, 6,3,4, 4,7,6, 7,4,5, 5,8,7 };

//		m_pMesh = new Mesh{ m_pDevice, vertices, indices, "resources/uv_grid_2.png" };

		const Vector3 position{ 0,0,0 };
		const Vector3 rotation{ 0,0,0 };
		const Vector3 scale{ 1,1,1 };

//		m_pMesh->SetWorldMatrix(Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(position));
	}
	void Renderer::VehicleMeshInit()
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		if (Utils::ParseOBJ("resources/vehicle.obj", vertices, indices) == false)
			std::cout << ".obj not found\n";

		m_pShadedEffect = new MeshEffect{ m_pDevice, L"resources/PosCol3D.fx" };
		m_pShadedEffect->SetDiffuseMap("resources/vehicle_diffuse.png", m_pDevice);
		m_pShadedEffect->SetNormalMap("resources/vehicle_normal.png", m_pDevice);
		m_pShadedEffect->SetSpecularMap("resources/vehicle_specular.png", m_pDevice);
		m_pShadedEffect->SetGlossinessMap("resources/vehicle_gloss.png", m_pDevice);

		const Vector3 position{ 0,0,0 };
		const Vector3 rotation{ 0,0,0 };
		const Vector3 scale{ 1,1,1 };
		const Matrix worldMatrix = Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(position);

		m_pShadedEffect->SetWorldMatrixVariable(worldMatrix);

		m_pMesh = new Mesh{ m_pDevice, vertices, indices, m_pShadedEffect };
		m_pMesh->SetWorldMatrix(worldMatrix);
	}
	void Renderer::CombustionMeshInit()
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		if (Utils::ParseOBJ("resources/fireFX.obj", vertices, indices) == false)
			std::cout << ".obj not found\n";

		m_pTransEffect = new TransEffect{ m_pDevice, L"resources/Transparent3D.fx" };
		m_pTransEffect->SetDiffuseMap("resources/fireFX_diffuse.png", m_pDevice);

		const Vector3 position{ 0,0,0 };
		const Vector3 rotation{ 0,0,0 };
		const Vector3 scale{ 1,1,1 };
		const Matrix worldMatrix = Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(position);

//		m_pTransEffect->SetWorldViewProjectionMatrixVariable()

		m_pTransparentMesh = new Mesh{ m_pDevice, vertices, indices, m_pTransEffect };
		m_pTransparentMesh->SetWorldMatrix(worldMatrix);
	}

	HRESULT Renderer::InitializeDirectX()
	{
		//Create Device and DeviceContext, using hardware acceleration
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		uint32_t createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
			1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		if (FAILED(result))
			return result;

		//Create DXGI Factory to create SwapChain based on hardware
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&m_pDXGIFactory));
		if (FAILED(result)) return result;

		// Create the swapchain description
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle HWND from the SDL backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version)
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create SwapChain and hook it into the handle of the SDL window
		result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result)) 
			return result;

		//Create the Depth/Stencil Buffer and View
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		//Create the Stencil Buffer
		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result)) 
			return result;

		//Create the Stencil View
		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result)) 
			return result;

		//Create the RenderTargetView
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result)) 
			return result;
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result)) 
			return result;

		//Bind the Views to the Output Merger Stage
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//Set the Viewport
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<FLOAT>(m_Width);
		viewport.Height = static_cast<FLOAT>(m_Height);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return S_OK;
	}
}
