//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "DisplayObject.h"
#include "Debug.h"
#include "Toolbox.h"
#include <string>
#include <limits>

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

using std::numeric_limits;

Game::Game()
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);
	m_displayList.clear();

	// Initial Settings
	// Modes
	m_grid = false;

	// Functional
	m_movespeed = 0.30;

	// Camera
	m_camPosition.x = 0.0f;
	m_camPosition.y = 0.0f;
	m_camPosition.z = 0.0f;
	m_camOrientation.x = 0;
	m_camOrientation.y = 0;
	m_camOrientation.z = 0;
	m_camLookAt.x = 0.0f;
	m_camLookAt.y = 0.0f;
	m_camLookAt.z = 0.0f;
	m_camLookDirection.x = 0.0f;
	m_camLookDirection.y = 0.0f;
	m_camLookDirection.z = 0.0f;
	m_camRight.x = 0.0f;
	m_camRight.y = 0.0f;
	m_camRight.z = 0.0f;
}

Game::~Game()
{

#ifdef DXTK_AUDIO
	if (m_audEngine)
	{
		m_audEngine->Suspend();
	}
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	m_gamePad = std::make_unique<GamePad>();

	m_keyboard = std::make_unique<Keyboard>();

	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);

	m_deviceResources->SetWindow(window, width, height);

	m_deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	m_deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	GetClientRect(window, &m_ScreenDimensions);

#ifdef DXTK_AUDIO
	// Create DirectXTK for Audio objects
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | AudioEngine_Debug;
#endif

	m_audEngine = std::make_unique<AudioEngine>(eflags);

	m_audioEvent = 0;
	m_audioTimerAcc = 10.f;
	m_retryDefault = false;

	m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

	m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
	m_effect1 = m_soundEffect->CreateInstance();
	m_effect2 = m_waveBank->CreateInstance(10);

	m_effect1->Play(true);
	m_effect2->Play();
#endif
}

void Game::SetGridState(bool state)
{
	m_grid = state;
}






#pragma region Frame Update

// Executes the basic game loop.
void Game::Tick(InputCommands* Input)
{
	// Copy over the input commands so we have a local version to use elsewhere.
	m_InputCommands = *Input;
	m_timer.Tick([&]()
		{
			Update(m_timer);
		});

#ifdef DXTK_AUDIO
	// Only update audio engine once per frame
	if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
	{
		// Setup a retry in 1 second
		m_audioTimerAcc = 1.f;
		m_retryDefault = true;
	}
#endif

	Render();
}


// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	// Camera motion is on a plane so kill the other seven deadly dimensions
	Vector3 planarMotionVector = m_camLookDirection;
	planarMotionVector.y = 0.0;

	// Update and log mouse coords and delta 
	int dx = m_InputCommands.x - prevMouseX;
	int dy = m_InputCommands.y - prevMouseY;
	prevMouseX = m_InputCommands.x;
	prevMouseY = m_InputCommands.y;

	// Some less annoying camera controls
	if (m_InputCommands.mouseRight) {
		if (dx != 0 || dy != 0) {
			m_camOrientation.y += dx;
			m_camOrientation.x -= dy;
		}
	}

	// Clamp camera pitch within reasonable values
	m_camOrientation.x = Toolbox::Clamp(m_camOrientation.x, -85.0f, 85.0f);

	// Create look direction from Euler angles in m_camOrientation
	float phi = (m_camOrientation.x) * 3.1415 / 180;
	float theta = (m_camOrientation.y) * 3.1415 / 180;
	m_camLookDirection.x = cos(theta) * cos(phi);
	m_camLookDirection.y = sin(phi);
	m_camLookDirection.z = sin(theta) * cos(phi);
	m_camLookDirection.Normalize();

	// Create right vector from look Direction
	m_camLookDirection.Cross(Vector3::UnitY, m_camRight);

	// Process input and update stuff
	// Sprint
	if (m_InputCommands.sprint) { m_movespeed = 0.90; }
	else m_movespeed = 0.30;
	// Movement
	if (m_InputCommands.forward){m_camPosition += m_camLookDirection * m_movespeed;}
	if (m_InputCommands.back){m_camPosition -= m_camLookDirection * m_movespeed;}
	if (m_InputCommands.right){m_camPosition += m_camRight * m_movespeed;}
	if (m_InputCommands.left){m_camPosition -= m_camRight * m_movespeed;}
	if (m_InputCommands.up){m_camPosition.y += m_movespeed; }
	if (m_InputCommands.down){m_camPosition.y -= m_movespeed; }
	// Wireframe
	bWireframe = m_InputCommands.wireframe_toggle;
	// Picking ray visualization
	bVisualizeRay = m_InputCommands.ray_toggle;
	
	// Update lookat point
	m_camLookAt = m_camPosition + m_camLookDirection;
	
	// Apply camera vectors
	m_view = Matrix::CreateLookAt(m_camPosition, m_camLookAt, Vector3::UnitY);

	//
	m_batchEffect->SetView(m_view);
	m_batchEffect->SetWorld(Matrix::Identity);
	m_displayChunk.m_terrainEffect->SetView(m_view);
	m_displayChunk.m_terrainEffect->SetWorld(Matrix::Identity);

#ifdef DXTK_AUDIO
	m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
	if (m_audioTimerAcc < 0)
	{
		if (m_retryDefault)
		{
			m_retryDefault = false;
			if (m_audEngine->Reset())
			{
				// Restart looping audio
				m_effect1->Play(true);
			}
		}
		else
		{
			m_audioTimerAcc = 4.f;

			m_waveBank->Play(m_audioEvent++);

			if (m_audioEvent >= 11)
				m_audioEvent = 0;
		}
	}
#endif
}


DirectX::XMVECTOR Game::GetPickingVector(int window_x, int window_y)
{
	DX_client_xDim = window_x;
	DX_client_yDim = window_y;

	DirectX::XMVECTOR PickingVector;

	// Set up near and far planes of frustum with mouse X and mouse Y passed down from Toolmain. 
	// They may look the same but note, the difference in Z
	const XMVECTOR nearSource = XMVectorSet(m_InputCommands.x, m_InputCommands.y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(m_InputCommands.x, m_InputCommands.y, 1.0f, 1.0f);

	DirectX::XMVECTOR defaultSca = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMVECTOR defaultRot = Quaternion::Identity;
	DirectX::XMVECTOR defaultTra = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	// Create set the matrix of the selected object in the world based on the translation, scale and rotation.
	XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, defaultSca, g_XMZero, defaultRot, defaultTra);

	// Unproject the points on the near and far plane, with respect to the matrix we just created.
	XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, DX_client_xDim, DX_client_yDim, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);
	XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, DX_client_xDim, DX_client_yDim, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);

	// Turn the transformed points into our picking vector. 
	PickingVector = farPoint - nearPoint;
	PickingVector = XMVector3Normalize(PickingVector);

	// Set PickingRay position and direction correctly to be used in object selection/terrain manipulation
	PickingRay.position	= m_camPosition;
	PickingRay.direction = PickingVector;

	m_displayChunk.GenerateHeightmap(PickingRay, brush_size, brush_intensity);

	return PickingVector;
}


int Game::MousePicking()
{
	DirectX::XMVECTOR PickingVector;
	float pickedDistance;
	float minimumDistance = std::numeric_limits<float>::max();

	// Set up near and far planes of frustum with mouse X and mouse Y passed down from Toolmain. 
	// They may look the same but note, the difference in Z
	const XMVECTOR nearSource = XMVectorSet(m_InputCommands.x, m_InputCommands.y, 0.0f, 1.0f);
	const XMVECTOR farSource = XMVectorSet(m_InputCommands.x, m_InputCommands.y, 1.0f, 1.0f);

	// Loop through entire display list of objects and pick with each in turn. 
	for (int i = 0; i < m_displayList.size(); i++)
	{
		// Get the scale factor and translation of the object
		const XMVECTORF32 scale = { m_displayList[i].m_scale.x,		m_displayList[i].m_scale.y,		m_displayList[i].m_scale.z };
		const XMVECTORF32 translate = { m_displayList[i].m_position.x,	m_displayList[i].m_position.y,	m_displayList[i].m_position.z };

		// Convert euler angles into a quaternion for the rotation of the object
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList[i].m_orientation.y * 3.1415 / 180,
			m_displayList[i].m_orientation.x * 3.1415 / 180,
			m_displayList[i].m_orientation.z * 3.1415 / 180);

		// Create set the matrix of the selected object in the world based on the translation, scale and rotation.
		XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		// Unproject the points on the near and far plane, with respect to the matrix we just created.
		XMVECTOR nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, DX_client_xDim, DX_client_yDim, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);
		XMVECTOR farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, DX_client_xDim, DX_client_yDim, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);

		// Turn the transformed points into our picking vector. 
		PickingVector = farPoint - nearPoint;
		PickingVector = XMVector3Normalize(PickingVector);

		// Loop through mesh list for object
		for (int y = 0; y < m_displayList[i].m_model.get()->meshes.size(); y++)
		{
			// Checking for ray intersection
			if (m_displayList[i].m_model.get()->meshes[y]->boundingBox.Intersects(nearPoint, PickingVector, pickedDistance))
			{
				if (pickedDistance < minimumDistance) {
					minimumDistance = pickedDistance;
					selectedID = i;
				}
			}
		}
	}

	return selectedID;
}

void Game::UpdateSculptSettings()
{
	switch (m_InputCommands.brush_control_int) {
	case 0:
		if (m_InputCommands.decrease)
		{
			brush_size -= 0.333f;
		}
		if (m_InputCommands.increase)
		{
			brush_size += 0.333f;
		}
		brush_size = Toolbox::Clamp(brush_size, 1.0f, 200.0f);
		//Debug::Out(std::to_string(brush_size) + "\n", "Brush size: ");
		break;

	case 1:
		if (m_InputCommands.decrease)
		{
			brush_intensity -= 0.025f;
		}
		if (m_InputCommands.increase)
		{
			brush_intensity += 0.025f;
		}
		brush_intensity = Toolbox::Clamp(brush_intensity, -2.0f, 2.0f);
		//Debug::Out(std::to_string(brush_intensity) + "\n", "Brush intensity: ");
		break;

	}
}

#pragma endregion






#pragma region Frame Render

// Draws the scene.
void Game::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return;
	}

	Clear();

	m_deviceResources->PIXBeginEvent(L"Render");
	auto context = m_deviceResources->GetD3DDeviceContext();



	if (m_grid)
	{
		// Draw procedurally generated dynamic grid
		const XMVECTORF32 xaxis = { 512.f, 0.f, 0.f };
		const XMVECTORF32 yaxis = { 0.f, 0.f, 512.f };
		DrawGrid(xaxis, yaxis, g_XMZero, 512, 512, Colors::Gray);
	}

	//CAMERA POSITION ON HUD
	m_sprites->Begin();
	WCHAR   Buffer[256];
	std::wstring var = L"Cam X: " + std::to_wstring(m_camPosition.x) + L"Cam Z: " + std::to_wstring(m_camPosition.z);
	m_font->DrawString(m_sprites.get(), var.c_str(), XMFLOAT2(100, 10), Colors::Yellow);
	m_sprites->End();

	//RENDER OBJECTS FROM SCENEGRAPH
	int numRenderObjects = m_displayList.size();
	for (int i = 0; i < numRenderObjects; i++)
	{
		m_deviceResources->PIXBeginEvent(L"Draw model");
		const XMVECTORF32 scale = { m_displayList[i].m_scale.x, m_displayList[i].m_scale.y, m_displayList[i].m_scale.z };
		const XMVECTORF32 translate = { m_displayList[i].m_position.x, m_displayList[i].m_position.y, m_displayList[i].m_position.z };

		// Convert degrees into radians for rotation matrix
		XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(m_displayList[i].m_orientation.y * 3.1415 / 180,
			m_displayList[i].m_orientation.x * 3.1415 / 180,
			m_displayList[i].m_orientation.z * 3.1415 / 180);

		XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);

		m_displayList[i].m_model->Draw(context, *m_states, local, m_view, m_projection, false);	//last variable in draw,  make TRUE for wireframe

		m_deviceResources->PIXEndEvent();
	}
	m_deviceResources->PIXEndEvent();

	//RENDER TERRAIN
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullNone());
	if (bWireframe) { context->RSSetState(m_states->Wireframe()); };

	// Render the batch,  This is handled in the Display chunk becuase it has the potential to get complex
	m_displayChunk.RenderBatch(m_deviceResources);

	// Render ray if the rendering bool is true
	if (bVisualizeRay) 
	{
		RenderRay(context);
	}

	m_deviceResources->Present();
}


// Helper method to clear the back buffers.
void Game::Clear()
{
m_deviceResources->PIXBeginEvent(L"Clear");

// Clear the views.
auto context = m_deviceResources->GetD3DDeviceContext();
auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
auto depthStencil = m_deviceResources->GetDepthStencilView();

context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
context->OMSetRenderTargets(1, &renderTarget, depthStencil);

// Set the viewport.
auto viewport = m_deviceResources->GetScreenViewport();
context->RSSetViewports(1, &viewport);

m_deviceResources->PIXEndEvent();
}


void XM_CALLCONV Game::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
m_deviceResources->PIXBeginEvent(L"Draw grid");

auto context = m_deviceResources->GetD3DDeviceContext();
context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
context->OMSetDepthStencilState(m_states->DepthNone(), 0);
context->RSSetState(m_states->CullCounterClockwise());

m_batchEffect->Apply(context);

context->IASetInputLayout(m_batchInputLayout.Get());

m_batch->Begin();

xdivs = std::max<size_t>(1, xdivs);
ydivs = std::max<size_t>(1, ydivs);

for (size_t i = 0; i <= xdivs; ++i)
{
	float fPercent = float(i) / float(xdivs);
	fPercent = (fPercent * 2.0f) - 1.0f;
	XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
	vScale = XMVectorAdd(vScale, origin);

	VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
	VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
	m_batch->DrawLine(v1, v2);
}

for (size_t i = 0; i <= ydivs; i++)
{
	float fPercent = float(i) / float(ydivs);
	fPercent = (fPercent * 2.0f) - 1.0f;
	XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
	vScale = XMVectorAdd(vScale, origin);

	VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
	VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
	m_batch->DrawLine(v1, v2);
}

m_batch->End();

m_deviceResources->PIXEndEvent();
}


void Game::RenderRay(ID3D11DeviceContext* context) {
	// Source: https://github.com/microsoft/DirectXTK/wiki/DebugDraw

	// SETUP
	m_states = std::make_unique<CommonStates>(m_deviceResources->GetD3DDevice());
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	m_batchEffect = std::make_unique<DirectX::BasicEffect>(m_deviceResources->GetD3DDevice());
	m_batchEffect->SetVertexColorEnabled(true);
	m_batchEffect->SetView(m_view);
	m_batchEffect->SetProjection(m_projection);

	{
		void const* shaderByteCode;
		size_t byteCodeLength;

		m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				VertexPositionColor::InputElements, VertexPositionColor::InputElementCount,
				shaderByteCode, byteCodeLength,
				m_batchInputLayout.ReleaseAndGetAddressOf()
				)
			);
	}

	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullNone());

	m_batchEffect->Apply(context);
	context->IASetInputLayout(m_batchInputLayout.Get());

	m_batch->Begin();

	// DRAW HERE
	Vector3 start = PickingRay.position;
	Vector3 direction = PickingRay.direction * 1500;

	DrawRay(
		m_batch.get(),
		XMVectorSet(start.x, start.y, start.z, 1.0f), XMVectorSet(direction.x, direction.y, direction.z, 1.0f),
		false, DirectX::Colors::Red
		);

	std::vector<DirectX::SimpleMath::Vector3> points;
	m_displayChunk.GetSelectedVertices(points);

	for (int i = 0; i < points.size(); i++) {
		DrawRay(
			m_batch.get(),
			XMVectorSet(points[i].x, points[i].y, points[i].z, 1.0f), XMVectorSet(0.0f, brush_intensity, 0.0f, 1.0f),
			true, DirectX::Colors::Red
			);
	}

	m_batch->End();
}

#pragma endregion






#pragma region Message Handlers

// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
	m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
	m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
	m_audEngine->Resume();
#endif
}

void Game::OnWindowSizeChanged(int width, int height)
{
	if (!m_deviceResources->WindowSizeChanged(width, height))
		return;

	CreateWindowSizeDependentResources();
}

void Game::BuildDisplayList(std::vector<SceneObject>* SceneGraph)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto devicecontext = m_deviceResources->GetD3DDeviceContext();

	if (!m_displayList.empty())		//is the vector empty
	{
		m_displayList.clear();		//if not, empty it
	}

	// For every item in the scenegraph
	int numObjects = SceneGraph->size();
	for (int i = 0; i < numObjects; i++)
	{
		// Create a temp display object that we will populate then append to the display list.
		DisplayObject newDisplayObject;

		// Load model
		std::wstring modelwstr = StringToWCHART(SceneGraph->at(i).model_path);							//convect string to Wchar
		newDisplayObject.m_model = Model::CreateFromCMO(device, modelwstr.c_str(), *m_fxFactory, true);	//get DXSDK to load model "False" for LH coordinate system (maya)

		// Load Texture
		std::wstring texturewstr = StringToWCHART(SceneGraph->at(i).tex_diffuse_path);								//convect string to Wchar
		HRESULT rs;
		rs = CreateDDSTextureFromFile(device, texturewstr.c_str(), nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource

		// If texture fails.  load error default
		if (rs)
		{
			CreateDDSTextureFromFile(device, L"database/data/Error.dds", nullptr, &newDisplayObject.m_texture_diffuse);	//load tex into Shader resource
		}

		// Apply new texture to models effect
		newDisplayObject.m_model->UpdateEffects([&](IEffect* effect) // This uses a Lambda function,  if you dont understand it: Look it up.
			{
				auto lights = dynamic_cast<BasicEffect*>(effect);
				if (lights)
				{
					lights->SetTexture(newDisplayObject.m_texture_diffuse);
				}
			});

		// Set position
		newDisplayObject.m_position.x = SceneGraph->at(i).posX;
		newDisplayObject.m_position.y = SceneGraph->at(i).posY;
		newDisplayObject.m_position.z = SceneGraph->at(i).posZ;

		// Setorientation
		newDisplayObject.m_orientation.x = SceneGraph->at(i).rotX;
		newDisplayObject.m_orientation.y = SceneGraph->at(i).rotY;
		newDisplayObject.m_orientation.z = SceneGraph->at(i).rotZ;

		// Set scale
		newDisplayObject.m_scale.x = SceneGraph->at(i).scaX;
		newDisplayObject.m_scale.y = SceneGraph->at(i).scaY;
		newDisplayObject.m_scale.z = SceneGraph->at(i).scaZ;

		// Set wireframe / render flags
		newDisplayObject.m_render = SceneGraph->at(i).editor_render;
		newDisplayObject.m_wireframe = SceneGraph->at(i).editor_wireframe;

		newDisplayObject.m_light_type = SceneGraph->at(i).light_type;
		newDisplayObject.m_light_diffuse_r = SceneGraph->at(i).light_diffuse_r;
		newDisplayObject.m_light_diffuse_g = SceneGraph->at(i).light_diffuse_g;
		newDisplayObject.m_light_diffuse_b = SceneGraph->at(i).light_diffuse_b;
		newDisplayObject.m_light_specular_r = SceneGraph->at(i).light_specular_r;
		newDisplayObject.m_light_specular_g = SceneGraph->at(i).light_specular_g;
		newDisplayObject.m_light_specular_b = SceneGraph->at(i).light_specular_b;
		newDisplayObject.m_light_spot_cutoff = SceneGraph->at(i).light_spot_cutoff;
		newDisplayObject.m_light_constant = SceneGraph->at(i).light_constant;
		newDisplayObject.m_light_linear = SceneGraph->at(i).light_linear;
		newDisplayObject.m_light_quadratic = SceneGraph->at(i).light_quadratic;

		m_displayList.push_back(newDisplayObject);

	}



}

void Game::BuildDisplayChunk(ChunkObject* SceneChunk)
{
	//populate our local DISPLAYCHUNK with all the chunk info we need from the object stored in toolmain
	//which, to be honest, is almost all of it. Its mostly rendering related info so...
	m_displayChunk.PopulateChunkData(SceneChunk);		//migrate chunk data
	m_displayChunk.LoadHeightMap(m_deviceResources);
	m_displayChunk.m_terrainEffect->SetProjection(m_projection);
	m_displayChunk.InitialiseBatch();
}

void Game::SaveDisplayChunk(ChunkObject* SceneChunk)
{
	m_displayChunk.SaveHeightMap();			//save heightmap to file.
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
	if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
	{
		// Setup a retry in 1 second
		m_audioTimerAcc = 1.f;
		m_retryDefault = true;
	}
}
#endif




#pragma endregion

#pragma region Direct3D Resources




// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto device = m_deviceResources->GetD3DDevice();

	m_states = std::make_unique<CommonStates>(device);

	m_fxFactory = std::make_unique<EffectFactory>(device);
	m_fxFactory->SetDirectory(L"database/data/"); //fx Factory will look in the database directory
	m_fxFactory->SetSharing(false);	//we must set this to false otherwise it will share effects based on the initial tex loaded (When the model loads) rather than what we will change them to.

	m_sprites = std::make_unique<SpriteBatch>(context);

	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	m_batchEffect = std::make_unique<BasicEffect>(device);
	m_batchEffect->SetVertexColorEnabled(true);

	{
		void const* shaderByteCode;
		size_t byteCodeLength;

		m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		DX::ThrowIfFailed(
			device->CreateInputLayout(VertexPositionColor::InputElements,
				VertexPositionColor::InputElementCount,
				shaderByteCode, byteCodeLength,
				m_batchInputLayout.ReleaseAndGetAddressOf())
		);
	}

	m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");

	//    m_shape = GeometricPrimitive::CreateTeapot(context, 4.f, 8);

		// SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
	m_model = Model::CreateFromSDKMESH(device, L"tiny.sdkmesh", *m_fxFactory);


	// Load textures
	DX::ThrowIfFailed(
		CreateDDSTextureFromFile(device, L"seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
	);

	DX::ThrowIfFailed(
		CreateDDSTextureFromFile(device, L"windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
	);

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();
	float aspectRatio = float(size.right) / float(size.bottom);
	float fovAngleY = _fov * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	m_projection = Matrix::CreatePerspectiveFieldOfView(
		fovAngleY,
		aspectRatio,
		_near,
		_far
	);

	m_batchEffect->SetProjection(m_projection);
}

void Game::OnDeviceLost()
{
	m_states.reset();
	m_fxFactory.reset();
	m_sprites.reset();
	m_batch.reset();
	m_batchEffect.reset();
	m_font.reset();
	m_shape.reset();
	m_model.reset();
	m_texture1.Reset();
	m_texture2.Reset();
	m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}




#pragma endregion

std::wstring StringToWCHART(std::string s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}