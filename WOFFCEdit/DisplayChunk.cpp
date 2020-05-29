#include <string>

#include "DisplayChunk.h"
#include "Game.h"
#include "Toolbox.h"
#include "ScreenGrab.h"
#include <DDSTextureLoader.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;

DisplayChunk::DisplayChunk()
{
	// Terrain size in meters. note that this is hard coded here, we COULD get it from the terrain chunk along with the other info from the tool if we want to be more flexible.
	m_terrainSize = 512;
	m_terrainHeightScale = 0.25;						// Convert our 0-256 terrain to 64
	m_textureCoordStep = 1.0 / (TERRAINRESOLUTION - 1);	// -1 becuase its split into chunks. not vertices.  we want tthe last one in each row to have tex coord 1
	m_terrainPositionScalingFactor = m_terrainSize / (TERRAINRESOLUTION - 1);
}


DisplayChunk::~DisplayChunk()
{
}

void DisplayChunk::PopulateChunkData(ChunkObject* SceneChunk)
{
	m_name = SceneChunk->name;
	m_chunk_x_size_metres = SceneChunk->chunk_x_size_metres;
	m_chunk_y_size_metres = SceneChunk->chunk_y_size_metres;
	m_chunk_base_resolution = SceneChunk->chunk_base_resolution;
	m_heightmap_path = SceneChunk->heightmap_path;
	m_tex_diffuse_path = SceneChunk->tex_diffuse_path;
	m_tex_splat_alpha_path = SceneChunk->tex_splat_alpha_path;
	m_tex_splat_1_path = SceneChunk->tex_splat_1_path;
	m_tex_splat_2_path = SceneChunk->tex_splat_2_path;
	m_tex_splat_3_path = SceneChunk->tex_splat_3_path;
	m_tex_splat_4_path = SceneChunk->tex_splat_4_path;
	m_render_wireframe = SceneChunk->render_wireframe;
	m_render_normals = SceneChunk->render_normals;
	m_tex_diffuse_tiling = SceneChunk->tex_diffuse_tiling;
	m_tex_splat_1_tiling = SceneChunk->tex_splat_1_tiling;
	m_tex_splat_2_tiling = SceneChunk->tex_splat_2_tiling;
	m_tex_splat_3_tiling = SceneChunk->tex_splat_3_tiling;
	m_tex_splat_4_tiling = SceneChunk->tex_splat_4_tiling;
}

void DisplayChunk::RenderBatch(std::shared_ptr<DX::DeviceResources>  DevResources)
{
	auto context = DevResources->GetD3DDeviceContext();

	m_terrainEffect->Apply(context);
	context->IASetInputLayout(m_terrainInputLayout.Get());

	m_batch->Begin();
	for (size_t i = 0; i < TERRAINRESOLUTION - 1; i++)		// Looping through QUADS.  so we subtract one from the terrain array or it will try to draw a quad starting with the last vertex in each row. Which wont work
	{
		for (size_t j = 0; j < TERRAINRESOLUTION - 1; j++)	// Same as above
		{
			m_batch->DrawQuad(m_terrainGeometry[i][j], m_terrainGeometry[i][j + 1], m_terrainGeometry[i + 1][j + 1], m_terrainGeometry[i + 1][j]); // Bottom left bottom right, top right top left.
		}
	}
	m_batch->End();
}

void DisplayChunk::InitialiseBatch()
{
	// Build geometry for our terrain array
	// Iterate through all the vertices of our required resolution terrain.
	int index = 0;

	for (size_t i = 0; i < TERRAINRESOLUTION; i++)
	{
		for (size_t j = 0; j < TERRAINRESOLUTION; j++)
		{
			index = (TERRAINRESOLUTION * i) + j;
			m_terrainGeometry[i][j].position = Vector3(j * m_terrainPositionScalingFactor - (0.5 * m_terrainSize), (float)(m_heightMap[index]) * m_terrainHeightScale, i * m_terrainPositionScalingFactor - (0.5 * m_terrainSize));	//This will create a terrain going from -64->64.  rather than 0->128.  So the center of the terrain is on the origin
			m_terrainGeometry[i][j].normal = Vector3(0.0f, 1.0f, 0.0f);						//standard y =up
			m_terrainGeometry[i][j].textureCoordinate = Vector2(((float)m_textureCoordStep * j) * m_tex_diffuse_tiling, ((float)m_textureCoordStep * i) * m_tex_diffuse_tiling);				//Spread tex coords so that its distributed evenly across the terrain from 0-1
			m_terrainGeometry[i][j].color = XMFLOAT4(DirectX::Colors::White);
		}
	}
	CalculateTerrainNormals();
}


void DisplayChunk::LoadHeightMap(std::shared_ptr<DX::DeviceResources>  DevResources)
{
	auto device = DevResources->GetD3DDevice();
	auto devicecontext = DevResources->GetD3DDeviceContext();

	//load in heightmap .raw
	FILE* pFile = NULL;

	// Open The File In Read / Binary Mode.

	pFile = fopen(m_heightmap_path.c_str(), "rb");
	// Check To See If We Found The File And Could Open It
	if (pFile == NULL)
	{
		// Display Error Message And Stop The Function
		MessageBox(NULL, L"Can't Find The Height Map!", L"Error", MB_OK);
		return;
	}

	// Here We Load The .RAW File Into Our pHeightMap Data Array
	// We Are Only Reading In '1', And The Size Is (Width * Height)
	fread(m_heightMap, 1, TERRAINRESOLUTION * TERRAINRESOLUTION, pFile);

	fclose(pFile);

	//load the diffuse texture
	std::wstring texturewstr = StringToWCHART(m_tex_diffuse_path);
	HRESULT rs;
	rs = CreateDDSTextureFromFile(device, texturewstr.c_str(), NULL, &m_texture_diffuse);	//load tex into Shader resource	view and resource

	// Setup terrain effect
	m_terrainEffect = std::make_unique<BasicEffect>(device);
	m_terrainEffect->EnableDefaultLighting();
	m_terrainEffect->SetLightingEnabled(true);
	// Colour!/Texture?
	bool is_vertex_painted = true;
	m_terrainEffect->SetVertexColorEnabled(is_vertex_painted);
	m_terrainEffect->SetTextureEnabled(!is_vertex_painted);
	m_terrainEffect->SetTexture(m_texture_diffuse);

	void const* shaderByteCode;
	size_t byteCodeLength;

	m_terrainEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	//setup batch
	DX::ThrowIfFailed(
		device->CreateInputLayout(VertexPositionNormalColorTexture::InputElements,
			VertexPositionNormalColorTexture::InputElementCount,
			shaderByteCode,
			byteCodeLength,
			m_terrainInputLayout.GetAddressOf())
	);

	m_batch = std::make_unique<PrimitiveBatch<VertexPositionNormalColorTexture>>(devicecontext);
}

void DisplayChunk::LoadVertexColours(std::shared_ptr<DX::DeviceResources> DevResources)
{
	// Setup D3DDeviceContext and D3DDevice
	auto devicecontext = DevResources->GetD3DDeviceContext();
	auto device = DevResources->GetD3DDevice();
	// Create Resource, Texture2D and Texture2D Description
	ID3D11Resource* texture_resource;
	ID3D11Texture2D* dds_texture;		// dds texture is for storing the loaded texture
	ID3D11Texture2D* terrain_texture;	// terrain texture is for accessing the texture on the cpu
	D3D11_TEXTURE2D_DESC texture_desc;
	// Set up a texture description 
	texture_desc.Width = TERRAINRESOLUTION;
	texture_desc.Height = TERRAINRESOLUTION;
	texture_desc.MipLevels = texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_STAGING;
	texture_desc.BindFlags = 0;
	texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texture_desc.MiscFlags = 0;

	// Load the DirectDraw Surface and create a texture2D 
	HRESULT hr = CreateDDSTextureFromFile(device, L"terrain_vertex_colours.dds", &texture_resource, nullptr);
	if (FAILED(hr)) {
		// Failed to load texture. Don't do anything. 
	}
	else {
		// Cast texture_resource to dds_texture (ID3D11Resource to ID3D11Texture2D)
		texture_resource->QueryInterface(IID_ID3D11Texture2D, (void**)&dds_texture);

		// Create a new texture with the above texture description - for CPU access
		device->CreateTexture2D(&texture_desc, nullptr, &terrain_texture);

		// Copy the texture over for reading (dds_texture copied to terrain_texture)
		devicecontext->CopyResource(terrain_texture, dds_texture);

		// Create mapped subresource
		D3D11_MAPPED_SUBRESOURCE mapped_resource;

		// Map terrain_texture using subresource
		devicecontext->Map(terrain_texture, 0, D3D11_MAP_READ, 0, &mapped_resource);

		// Access the mapped resource pixel values from pSysMem
		XMFLOAT4* floatval = reinterpret_cast<XMFLOAT4*>(mapped_resource.pData);

		// Loop through terrain geometry and set 
		for (int i = 0; i < TERRAINRESOLUTION; i++) {
			for (int j = 0; j < TERRAINRESOLUTION; j++) {
				int index = (i * TERRAINRESOLUTION) + j;
				m_terrainGeometry[i][j].color = floatval[index];
			}
		}
	}
	
}

void DisplayChunk::SaveVertexColours(std::shared_ptr<DX::DeviceResources> DevResources)
{
	// Setup D3DDeviceContext and D3DDevice
	auto devicecontext = DevResources->GetD3DDeviceContext();
	auto device = DevResources->GetD3DDevice();
	// Create Texture2D and Texture2D Description
	ID3D11Texture2D* terrain_texture;
	D3D11_TEXTURE2D_DESC texture_desc;
	// Set up a texture description 
	texture_desc.Width = TERRAINRESOLUTION;
	texture_desc.Height = TERRAINRESOLUTION;
	texture_desc.MipLevels = texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DYNAMIC;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texture_desc.MiscFlags = 0;

	// Create vertex colour vector
	std::vector<float> colour_vector;
	for (int i = 0; i < TERRAINRESOLUTION; i++) {
		for (int j = 0; j < TERRAINRESOLUTION; j++) {
			colour_vector.push_back((float)m_terrainGeometry[i][j].color.x);
			colour_vector.push_back((float)m_terrainGeometry[i][j].color.y);
			colour_vector.push_back((float)m_terrainGeometry[i][j].color.z);
			colour_vector.push_back((float)m_terrainGeometry[i][j].color.w);
		}
	}

	// Initialise buffer parameters
	const int components = 4;
	const int length = components * TERRAINRESOLUTION * TERRAINRESOLUTION;
	// Fill buffer with vertex colours
	float* buffer = new float[length * sizeof(float)];
	for (int i = 0; i < length; i++)
		buffer[i] = colour_vector[i];

	// Set the texture data using the buffer contents
	D3D11_SUBRESOURCE_DATA texture_data;
	texture_data.pSysMem = (void*)buffer;
	texture_data.SysMemPitch = TERRAINRESOLUTION * components * sizeof(float);

	// Create the texture using the terrain colour data
	device->CreateTexture2D(&texture_desc, &texture_data, &terrain_texture);

	// Save the texture to a .dds file
	HRESULT hr = SaveDDSTextureToFile(devicecontext, terrain_texture, L"terrain_vertex_colours.dds");

	// Delete the buffer
	delete[] buffer;
}

// Screenshot functionality
	//auto deviceresource = DevResources->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D),
	//	reinterpret_cast<LPVOID*>(terrain_texture.GetAddressOf()));

void DisplayChunk::SaveHeightMap()
{
	UpdateHeightmap();

	FILE* pFile = NULL;

	// Open The File In Read / Binary Mode.
	pFile = fopen(m_heightmap_path.c_str(), "wb+");;
	// Check To See If We Found The File And Could Open It
	if (pFile == NULL)
	{
		// Display Error Message And Stop The Function
		MessageBox(NULL, L"Can't Find The Height Map!", L"Error", MB_OK);
		return;
	}

	fwrite(m_heightMap, 1, TERRAINRESOLUTION * TERRAINRESOLUTION, pFile);
	fclose(pFile);
}


void DisplayChunk::UpdateTerrain()
{
	//all this is doing is transferring the height from the heightmap into the terrain geometry.
	int index;
	for (size_t i = 0; i < TERRAINRESOLUTION; i++)
	{
		for (size_t j = 0; j < TERRAINRESOLUTION; j++)
		{
			index = (TERRAINRESOLUTION * i) + j;
			m_terrainGeometry[i][j].position.y = (float)(m_heightMap[index]) * m_terrainHeightScale;
		}
	}
	CalculateTerrainNormals();
}

void DisplayChunk::UpdateHeightmap()
{
	//all this is doing is transferring the height from the terrain geometry into the heightmap.
	int index;
	for (size_t i = 0; i < TERRAINRESOLUTION; i++)
	{
		for (size_t j = 0; j < TERRAINRESOLUTION; j++)
		{
			index = (TERRAINRESOLUTION * i) + j;
			m_heightMap[index] = m_terrainGeometry[i][j].position.y / m_terrainHeightScale;
		}
	}
}

DirectX::SimpleMath::Vector3 DisplayChunk::GetBrushCenter(DirectX::SimpleMath::Ray PickingVector, bool* DidHit)
{
	DirectX::SimpleMath::Vector3 v1, v2, v3, v4;			// Position vector veriables 
	float distance = 10000.0f;								// Distance 
	Vector3 center;											// Center of brush xyz

	for (size_t i = 0; i < TERRAINRESOLUTION - 1; i++) {
		for (int j = 0; j < TERRAINRESOLUTION - 1; j++)
		{
			// Initialize the quad vertices to prepare for intersection test.
			v1 = m_terrainGeometry[i][j].position;
			v2 = m_terrainGeometry[i][j + 1].position;
			v3 = m_terrainGeometry[i + 1][j + 1].position;
			v4 = m_terrainGeometry[i + 1][j].position;

			// Perform intersection test. 
			if (PickingVector.Intersects(v1, v2, v3, distance) || PickingVector.Intersects(v1, v4, v3, distance)) {
				if (m_terrainGeometry[i][j].position.y < PickingVector.position.y && m_terrainGeometry[i][j].position.y > PickingVector.direction.y * distance) {
					center = Vector3(m_terrainGeometry[i][j].position + m_terrainGeometry[i + 1][j + 1].position) / 2;
					*DidHit = true;
				}
			}
		}
	}
	return center;
}

void DisplayChunk::GenerateHeightmap(float radius, float intensity, DirectX::SimpleMath::Vector3 center)
{
	for (size_t x = 0; x < TERRAINRESOLUTION; x++)
	{	// Loop through vertices again to do a distance check on the selected vertex. If within radius, expand based on mapped ranged value.		
		for (int y = 0; y < TERRAINRESOLUTION; y++)
		{
			float proximity = DirectX::SimpleMath::Vector3::Distance(m_terrainGeometry[x][y].position, center);
			if (proximity < radius)
			{
				// Calculate multiplier based on vertex distance from center (0.0f being center)
				float mult = Toolbox::MappedClamp(proximity, 0.0f, radius, 1.0f, 0.0f);
				// Perform the edit
				m_terrainGeometry[x][y].position.y += intensity * mult;
			}
		}
	}
}

void DisplayChunk::PaintTerrain(float radius, float intensity, DirectX::SimpleMath::Vector3 center, XMFLOAT4 color)
{
	for (size_t x = 0; x < TERRAINRESOLUTION; x++)
	{	// Loop through vertices again to do a distance check on the selected vertex. If within radius, paint terrain.
		for (int y = 0; y < TERRAINRESOLUTION; y++)
		{
			float proximity = DirectX::SimpleMath::Vector3::Distance(m_terrainGeometry[x][y].position, center);
			if (proximity < radius)
			{
				m_terrainGeometry[x][y].color = color;
			}
		}
	} 
}

void DisplayChunk::CalculateTerrainNormals()
{
	int index1, index2, index3, index4;
	DirectX::SimpleMath::Vector3 upDownVector, leftRightVector, normalVector;

	for (int i = 0; i < (TERRAINRESOLUTION - 1); i++)
	{
		for (int j = 0; j < (TERRAINRESOLUTION - 1); j++)
		{
			upDownVector.x = (m_terrainGeometry[i + 1][j].position.x - m_terrainGeometry[i - 1][j].position.x);
			upDownVector.y = (m_terrainGeometry[i + 1][j].position.y - m_terrainGeometry[i - 1][j].position.y);
			upDownVector.z = (m_terrainGeometry[i + 1][j].position.z - m_terrainGeometry[i - 1][j].position.z);

			leftRightVector.x = (m_terrainGeometry[i][j - 1].position.x - m_terrainGeometry[i][j + 1].position.x);
			leftRightVector.y = (m_terrainGeometry[i][j - 1].position.y - m_terrainGeometry[i][j + 1].position.y);
			leftRightVector.z = (m_terrainGeometry[i][j - 1].position.z - m_terrainGeometry[i][j + 1].position.z);


			leftRightVector.Cross(upDownVector, normalVector);	// get cross product
			normalVector.Normalize();							// normalise it.

			m_terrainGeometry[i][j].normal = normalVector;	// set the normal for this point based on our result
		}
	}
}