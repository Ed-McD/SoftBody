#include "gamedata.h"
#include "VBPLane.h"
#include "drawdata.h"
#include "Turret_base.h"


void VBPlane::init(int _size, ID3D11Device* GD)
{
	m_size = _size;
	time = 0.0f;
	freq = 2.0f;
	amp = 2.5f;
	waveLength = 0.025f;
	rippleFreq = 2.0f;
	rippleAmp = 2.5f;
	rippleWL = 0.025f;
	float scale = 5.0f;
	m_diagonal = 0;
	m_ripple = false;
	m_waves = true;

	
	

	//calculate number of vertices and primatives
	int numVerts = 6 * (m_size - 1) * (m_size - 1);
	m_numPrims = numVerts / 3;
	m_numVertices = numVerts;
	m_vertices = new myVertex[numVerts];
	WORD* indices = new WORD[numVerts];
	
	

	//as using the standard VB shader set the tex-coords somewhere safe
	for (int i = 0; i < numVerts; i++)
	{
		indices[i] = i;
		m_vertices[i].texCoord = Vector2::One;
	}
	

	//in each loop create the two traingles for the matching sub-square on each of the six faces
	int vert = 0;
	for (int i = -(m_size - 1) / 2; i < (m_size - 1) / 2; i++)
	{
		for (int j = -(m_size - 1) / 2; j < (m_size - 1) / 2; j++)
		{
			//top
			m_vertices[vert].Color = Color(0.0f, 0.0f, 1.0f, 1.0f);
			m_vertices[vert++].Pos = Vector3((float)i, 0.5f * (float)(m_size - 1), (float)j);
			m_vertices[vert].Color = Color(0.0f, 0.0f, 1.0f, 1.0f);
			m_vertices[vert++].Pos = Vector3((float)i, 0.5f * (float)(m_size - 1), (float)(j + 1));
			m_vertices[vert].Color = Color(0.0f, 0.0f, 1.0f, 1.0f);
			m_vertices[vert++].Pos = Vector3((float)(i + 1), 0.5f * (float)(m_size - 1), (float)j);

			m_vertices[vert].Color = Color(0.0f, 0.0f, 1.0f, 1.0f);
			m_vertices[vert++].Pos = Vector3((float)(i + 1), 0.5f * (float)(m_size - 1), (float)j);
			m_vertices[vert].Color = Color(0.0f, 0.0f, 1.0f, 1.0f);
			m_vertices[vert++].Pos = Vector3((float)i, 0.5f * (float)(m_size - 1), (float)(j + 1));
			m_vertices[vert].Color = Color(0.0f, 0.0f, 1.0f, 1.0f);
			m_vertices[vert++].Pos = Vector3((float)(i + 1), 0.5f * (float)(m_size - 1), (float)(j + 1));
			
		}
	}
	for (int i = 0; i < m_numPrims * 3; i++)
	{
		Vector3 vertScale = m_vertices[i].Pos;

		Matrix scaleMat = Matrix::CreateScale(scale, 1.0f,scale);

		Vector3 newScale = Vector3::Transform(vertScale, scaleMat);

		m_vertices[i].Pos = newScale;
	}
	//carry out some kind of transform on these vertices to make this object more interesting
	//Transform();

	//calculate the normals for the basic lighting in the base shader
	for (int i = 0; i < m_numPrims; i++)
	{ 
		WORD V1 = 3 * i;
		WORD V2 = 3 * i + 1;
		WORD V3 = 3 * i + 2;

		//build normals
		Vector3 norm;
		Vector3 vec1 = m_vertices[V1].Pos - m_vertices[V2].Pos;
		Vector3 vec2 = m_vertices[V3].Pos - m_vertices[V2].Pos;
		norm = vec1.Cross(vec2);
		norm.Normalize();

		m_vertices[V1].Norm = norm;
		m_vertices[V2].Norm = norm;
		m_vertices[V3].Norm = norm;
	}


	BuildIB(GD, indices);
	BuildDVB(GD, numVerts, m_vertices);

	m_centrepoint = ((m_numVertices / 2) + (m_size / 2));

	//delete[] m_vertices; //this is no longer needed as this is now in the Vertex Buffer
}



void VBPlane::Tick(GameData* GD)
{
	if ((GD->keyboard[DIK_RETURN] & 0x80) && !(GD->prevKeyboard[DIK_RETURN] & 0x80))
	{


		for (int j = 0; j < m_numVertices; j++)
		{
			float xDiff;
			float zDiff;
			float playerPosOffset;
			xDiff = (GD->playerPos.x - m_vertices[j].Pos.x);
			zDiff = (GD->playerPos.z - m_vertices[j].Pos.z);
			playerPosOffset = sqrtf((zDiff*zDiff) + (xDiff*xDiff));

			float cpxDiff;
			float cpzDiff;
			float cpOffset;
			cpxDiff = (m_vertices[m_centrepoint].Pos.x - GD->playerPos.x);
			cpzDiff = (m_vertices[m_centrepoint].Pos.z - GD->playerPos.z);
			cpOffset = sqrtf((cpzDiff*cpzDiff) + (cpxDiff*cpxDiff));

			if (playerPosOffset < cpOffset);
			{
				m_centrepoint = j;
			}
		}

	}
	
	time = time + GD->dt;
	Transform();
	
	

	if (GD->mouse->rgbButtons[0])
	{ 
		freq = freq + 1.0f;
	}
	if (GD->mouse->rgbButtons[1])
	{
		freq = freq - 1.0f;
	}
	if (GD->keyboard[DIK_Q] & 0x80)
	{
		amp = amp + 1.0f;
	}
	if (GD->keyboard[DIK_E] & 0x80)
	{
		amp = amp - 1.0f;
	}
	if (GD->keyboard[DIK_A] & 0x80)
	{
		waveLength = waveLength + 0.0005f;
	}
	if (GD->keyboard[DIK_D] & 0x80)
	{
		waveLength = waveLength - 0.0005f;
	}
	if (GD->keyboard[DIK_U] & 0x80)
	{
		rippleFreq = rippleFreq + 1.0f;
	}
	if (GD->keyboard[DIK_I] & 0x80)
	{
		rippleFreq = rippleFreq - 1.0f;
	}
	if (GD->keyboard[DIK_J] & 0x80)
	{
		rippleAmp = rippleAmp + 1.0f;
	}
	if (GD->keyboard[DIK_K] & 0x80)
	{
		rippleAmp = rippleAmp - 1.0f;
	}
	if (GD->keyboard[DIK_N] & 0x80)
	{
		rippleWL = rippleWL + 0.0005f;
	}
	if (GD->keyboard[DIK_M] & 0x80)
	{
		rippleWL = rippleWL - 0.0005f;
	}
	if ((GD->keyboard[DIK_LSHIFT] & 0x80) && !(GD->prevKeyboard[DIK_LSHIFT] & 0x80))
	{
 		m_diagonal ++;
		if (m_diagonal > 3)
			m_diagonal = 0;
	}
	if ((GD->keyboard[DIK_R] & 0x80) && !(GD->prevKeyboard[DIK_R] & 0x80)) //Reset to default values
	{
		freq = 2.0f;
		amp = 2.5f;
		waveLength = 0.025f;
		rippleFreq = 2.0f;
		rippleAmp = 2.5f;
		rippleWL = 0.025f;
	}
	if ((GD->keyboard[DIK_W] & 0x80) && !(GD->prevKeyboard[DIK_W] & 0x80))
	{
		m_waves = !m_waves;
	}
	if ((GD->keyboard[DIK_X] & 0x80) && !(GD->prevKeyboard[DIK_X] & 0x80))
	{
		m_ripple = !m_ripple;
	}

	
}




void VBPlane::Transform()
{ 
	for (int j = 0; j < m_numVertices; j++)
	{
		float m_wavesPos = 0.0f;
		float m_ripplePos = 0.0f;

		if (m_waves == true)
		{
			switch (m_diagonal)
			{
			case 0:
				
				m_wavesPos = amp * sin((freq * time) + ((m_vertices[j].Pos.x) * waveLength));

				break;
			case 1:
				
				m_wavesPos = amp * sin((freq * time) + ((m_vertices[j].Pos.x) * waveLength) + ((m_vertices[j].Pos.z) * waveLength));
								
				break;
			case 2:
				
				m_wavesPos = amp * sin((freq * time) + ((m_vertices[j].Pos.z) * waveLength));
				
				break;
			case 3:
				
				m_wavesPos = amp * sin((freq * time) - ((m_vertices[j].Pos.x) * waveLength) + ((m_vertices[j].Pos.z) * waveLength));

				break;
			}
		}
		if (m_ripple == true)
		{
				float xDiff;
				float zDiff;
				float cpOffset;
				xDiff = (m_vertices[m_centrepoint].Pos.x - m_vertices[j].Pos.x);
				zDiff = (m_vertices[m_centrepoint].Pos.z - m_vertices[j].Pos.z);
				cpOffset = sqrtf((zDiff*zDiff) + (xDiff*xDiff));


				m_ripplePos = rippleAmp * sin((rippleFreq * time) + ((cpOffset)* rippleWL));

		}
		
		if (m_ripple == true && m_waves == false)
		{
			m_vertices[j].Pos.y = (m_ripplePos);
		}
		if (m_ripple == false && m_waves == true)
		{
			m_vertices[j].Pos.y = (m_wavesPos);
		}
		if (m_ripple == true && m_waves == true)
		{
			m_vertices[j].Pos.y = ((m_ripplePos + m_wavesPos)/2);
		}
	}


	


	//calculate the normals for the basic lighting in the base shader
	for (int i = 0; i < m_numPrims; i++)
	{
		WORD V1 = 3 * i;
		WORD V2 = 3 * i + 1;
		WORD V3 = 3 * i + 2;

		//build normals
		Vector3 norm;
		Vector3 vec1 = m_vertices[V1].Pos - m_vertices[V2].Pos;
		Vector3 vec2 = m_vertices[V3].Pos - m_vertices[V2].Pos;
		norm = vec1.Cross(vec2);
		norm.Normalize();

		m_vertices[V1].Norm = norm;
		m_vertices[V2].Norm = norm;
		m_vertices[V3].Norm = norm;
	}
	



	/*for (int i = 0; i < m_numVertices; i++)
	{
		float sineWave = amp * sin(((2 * XM_PI*freq)*time) + phase);

		float vertPos = m_vertices[i].Pos.y;

		float newPos = vertPos + sineWave;
		
		if (i % 2 == 0)
		{
			m_vertices[i].Pos.y = newPos;
			phase = phase + (3 * i);
		}
		else
		{
			m_vertices[i].Pos.y = - newPos;
			phase = phase + (3 * i);
		}

	}*/
}
	


void VBPlane::Draw(DrawData* _DD)
{

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//Disable GPU access to the vertex buffer data.
	_DD->pd3dImmediateContext->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	
	////Update the vertex buffer here.
	//memcpy(mappedResource.pData, m_vertices, sizeof(m_vertices));

	myVertex* p_vertices = (myVertex*)mappedResource.pData;

	//Update the vertex buffer here.
	memcpy(p_vertices, (void*)m_vertices, sizeof(myVertex) * m_numVertices);

	//Reenable GPU access to the vertex buffer data.
 	_DD->pd3dImmediateContext->Unmap(m_VertexBuffer, 0);

 	VBGO::Draw(_DD);
	
}
