//
// Game.h
//

#pragma once

#include "ChunkObject.h"
#include "Debug.h"
#include "DebugDraw.h"
#include "DeviceResources.h"
#include "DisplayChunk.h"
#include "DisplayObject.h"
#include "InputCommands.h"
#include "Picking.h"
#include "SceneObject.h"
#include "SimpleMath.h"
#include "StepTimer.h"
#include "Toolbox.h"
#include <vector>


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game : public DX::IDeviceNotify
{
public: // Members

	Game();
	~Game();

	// Initialization and management
	void Initialize(HWND window, int width, int height);
	void SetGridState(bool state);

	// Basic game loop
	void Tick(InputCommands* Input);
	void Render();
	
	// Rendering helpers
	void Clear();

	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;

	// Messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowSizeChanged(int width, int height);

	// TOOL SPECIFIC ------------------------------
	void BuildDisplayList(std::vector<SceneObject>* SceneGraph);	// Note vector passed by reference 
	void BuildDisplayChunk(ChunkObject *SceneChunk);
	void SaveDisplayChunk(ChunkObject *SceneChunk);					// Saves geometry et al
	//void ClearDisplayList();
	// Getters for relevant variables and resources
	DirectX::SimpleMath::Vector3 GetCameraPosition() { return m_camPosition; };
	DirectX::SimpleMath::Matrix GetWorldMatrix() { return m_world; };
	DirectX::SimpleMath::Matrix GetViewMatrix() { return m_view; };
	DirectX::SimpleMath::Matrix GetProjectionMatrix() { return m_projection; };
	std::vector<DisplayObject>& GetDisplayList() { return m_displayList; };
	DisplayChunk& GetDisplayChunk() { return m_displayChunk; };
	std::shared_ptr<DX::DeviceResources>& GetDeviceResourcesRef() { return m_deviceResources; };
	void SetRayForVisualization(DirectX::SimpleMath::Ray ray) { picking_ray = ray; };
	void SetBrushForVisualization(DirectX::SimpleMath::Vector3 center, float radius) { brush_center = center; brush_radius = radius; };
	// Visual aid
	void RenderRay(ID3D11DeviceContext* context);
	void RenderBrush(ID3D11DeviceContext* context);
	// TOOL SPECIFIC ------------------------------ // END

#ifdef DXTK_AUDIO
	void NewAudioDevice();
#endif

private: // Members

	// Update the editor
	void Update(DX::StepTimer const& timer);
	void UpdateInput();
	void UpdateCamera();
	//
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	//
	void XM_CALLCONV DrawGrid(DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color);

private: // Variables


	// TOOL SPECIFIC ------------------------------
	// Mouse location on viewport and logging function for debugging etc.
	int prevMouseX = 0;
	int prevMouseY = 0;
	InputCommands						m_InputCommands;
	// Functionality
	float								m_movespeed;
	// Camera
	DirectX::SimpleMath::Vector3		m_camPosition;
	DirectX::SimpleMath::Vector3		m_camOrientation;
	DirectX::SimpleMath::Vector3		m_camLookAt;
	DirectX::SimpleMath::Vector3		m_camLookDirection;
	DirectX::SimpleMath::Vector3		m_camRight;
	// Control variables
	bool m_grid;							//grid rendering on / off
	// Visualization
	DirectX::SimpleMath::Ray picking_ray;
	DirectX::SimpleMath::Vector3 brush_center; 
	float brush_radius;
	// TOOL SPECIFIC ------------------------------ // END

	// Objects and chunk
	std::vector<DisplayObject>			m_displayList;
	DisplayChunk						m_displayChunk;

	// Device resources.
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Input devices.
    std::unique_ptr<DirectX::GamePad>       m_gamePad;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX::Mouse>         m_mouse;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::GeometricPrimitive>                            m_shape;
    std::unique_ptr<DirectX::Model>                                         m_model;
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
#endif

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture2;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;

#ifdef DXTK_AUDIO
    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;
#endif

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;

	RECT m_ScreenDimensions;
};

std::wstring StringToWCHART(std::string s);