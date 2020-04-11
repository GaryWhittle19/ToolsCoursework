#include <string>

#include "DisplayChunk.h"
#include "Game.h"
#include "Toolbox.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

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
	// Texture?
	m_terrainEffect->SetTextureEnabled(false);
	m_terrainEffect->SetTexture(m_texture_diffuse);
	// Colour!
	m_terrainEffect->SetVertexColorEnabled(true);


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
	for (size_t x = 0; x < TERRAINRESOLUTION - 1; x++)
	{	// Loop through vertices again to do a distance check on the selected vertex. If within radius, expand based on mapped ranged value.		
		for (int y = 0; y < TERRAINRESOLUTION - 1; y++)
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
	for (size_t x = 0; x < TERRAINRESOLUTION - 1; x++)
	{	// Loop through vertices again to do a distance check on the selected vertex. If within radius, paint terrain.
		for (int y = 0; y < TERRAINRESOLUTION - 1; y++)
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


			leftRightVector.Cross(upDownVector, normalVector);	//get cross product
			normalVector.Normalize();			//normalise it.

			m_terrainGeometry[i][j].normal = normalVector;	//set the normal for this point based on our result
		}
	}
}