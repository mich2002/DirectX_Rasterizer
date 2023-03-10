#pragma once
#include "Mesh.h"

struct SDL_Window;
struct SDL_Surface;

class Camera;
class MeshEffect;
class TransEffect;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer) const;
		void Render() const;

		void ToggleFilteringMethod();
		void ToggleRotation() { m_IsRotating = !m_IsRotating; }

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };
		bool m_IsRotating{ false };

		Camera* m_pCamera;

		Mesh* m_pMesh;
		Mesh* m_pTransparentMesh;

		MeshEffect* m_pShadedEffect;
		TransEffect* m_pTransEffect;

		void TriangleMeshInit();
		void QuadMeshInit();
		void VehicleMeshInit();
		void CombustionMeshInit();

		Mesh::FilteringMethod m_FilteringMethod{ Mesh::FilteringMethod::Point };

		//DIRECTX
		HRESULT InitializeDirectX();

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;

		IDXGIFactory* m_pDXGIFactory;

		IDXGISwapChain* m_pSwapChain;

		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
	};
}
