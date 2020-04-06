//
// Game.h
//

#pragma once

#include "DebugDraw.h"
#include "SimpleMath.h"
#include "DeviceResources.h"
#include "StepTimer.h"
#include "SceneObject.h"
#include "DisplayObject.h"
#include "DisplayChunk.h"
#include "ChunkObject.h"
#include "InputCommands.h"
#include "Picking.h"
#include "Debug.h"
#include "Toolbox.h"
#include <vector>


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game : public DX::IDeviceNotify
{
public:

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

	// Tool specific
	void BuildDisplayList(std::vector<SceneObject>* SceneGraph);	// Note vector passed by reference 
	void BuildDisplayChunk(ChunkObject *SceneChunk);
	void SaveDisplayChunk(ChunkObject *SceneChunk);					// Saves geometry et al
	//void ClearDisplayList();

	// Picking
	void Pick(int& object_ID);
	void Pick();
	// Mouse editing will edit the terrain if edit_toggle is true (m_toolInputCommands)
	void UpdateSculptSettings();																

#ifdef DXTK_AUDIO
	void NewAudioDevice();
#endif

private:

	// Update the editor
	void Update(DX::StepTimer const& timer);
	void UpdateInput();
	void UpdateCamera();
	// Picking ray
	void RenderRay(ID3D11DeviceContext* context);

	// Mouse location on viewport and logging function for debugging etc.
	int prevMouseX = 0;
	int prevMouseY = 0;
	// DirectX Client Size
	int DX_client_xDim;
	int DX_client_yDim;

	// Handles our object and terrain picking
	PickingHandler picking_handler;
	DirectX::SimpleMath::Ray picking_ray;

	// Terrain manipulation
	int	target_brush_var = 0;					// For targeting brush variables... 0 is size, 1 is intensity (see below)
	float brush_size = 25.0f;					// 0 
	float brush_intensity = 1.0f;				// 1

	// Object selection
	int selectedID = -1;

	//
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	//
	float _fov = 70.0f;
	float _near = 0.01f;
	float _far = 1000.0f;

	void XM_CALLCONV DrawGrid(DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color);

	// Tool specific
	std::vector<DisplayObject>			m_displayList;
	DisplayChunk						m_displayChunk;
	InputCommands						m_InputCommands;

	// Functionality
	float								m_movespeed;
	bool bWireframe						= false;
	bool bVisualizeRay					= false;

	// Camera
	DirectX::SimpleMath::Vector3		m_camPosition;
	DirectX::SimpleMath::Vector3		m_camOrientation;
	DirectX::SimpleMath::Vector3		m_camLookAt;
	DirectX::SimpleMath::Vector3		m_camLookDirection;
	DirectX::SimpleMath::Vector3		m_camRight;
	
	// Control variables
	bool m_grid;							//grid rendering on / off
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