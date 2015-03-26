#include "gamedata.h"
#include "VBPLane.h"
#include "drawdata.h"
#include "Turret_base.h"
#include "AntTweakTest.h"

void VBPlane::init(int _size, float _scale, GameData* _GD , ID3D11Device* GD)
{
	m_size = _size;
	time = 0.0f;
	freq = 2.0f;
	amp = 2.5f;
	waveLength = 0.025f;
	rippleFreq = 20.0f;
	rippleAmp = 15.0f;
	rippleWL = 0.1f;
	rippleFalloff = 0.75f;
	m_scale = _scale;
	m_diagonal = 0;
	m_ripple = false;
	m_waves = true;
	rippleCount = 0;
	useRippleClass = true;
	useSinSim = false;
	useVerlet = !useSinSim;
	springCoeff = 0.1f;
	disturbance = 5.0f;

	m_GD = _GD;
	m_Device = GD;

	float aLvl = 0.25f;
	Color surfaceColour = {0.0f, 0.0f, 1.0f, aLvl};




	//calculate number of vertices and primatives
	int numVerts = 6 * (m_size - 1) * (m_size - 1);
	m_numPrims = numVerts / 3;
	m_numVertices = numVerts;
	m_vertices = new myVertex[numVerts];
	WORD* indices = new WORD[numVerts];

	newVertices = new float [m_size*m_size];
	currVertices = new float [m_size*m_size];
	memset(newVertices, 0, sizeof (float)*m_size*m_size);
	memset(currVertices, 0, sizeof (float)*m_size*m_size);
	

	//as using the standard VB shader set the tex-coords somewhere safe
	for (int i = 0; i < numVerts; i++)
	{
		indices[i] = i;
		m_vertices[i].texCoord = Vector2::One;
	}
	

	//in each loop create the two traingles for the matching sub-square on each of the six faces
	int vert = 0;
	for (int i = 0; i < (m_size-1); i++)
	{
		for (int j = 0; j < (m_size - 1); j++)
		{
			//top
			m_vertices[vert].Color = surfaceColour;
			m_vertices[vert++].Pos = Vector3((float)i, 0.5f * (float)(m_size - 1), (float)j);
			m_vertices[vert].Color = surfaceColour;
			m_vertices[vert++].Pos = Vector3((float)i, 0.5f * (float)(m_size - 1), (float)(j + 1));
			m_vertices[vert].Color = surfaceColour;
			m_vertices[vert++].Pos = Vector3((float)(i + 1), 0.5f * (float)(m_size - 1), (float)j);

			m_vertices[vert].Color = surfaceColour;
			m_vertices[vert++].Pos = Vector3((float)(i + 1), 0.5f * (float)(m_size - 1), (float)j);
			m_vertices[vert].Color = surfaceColour;
			m_vertices[vert++].Pos = Vector3((float)i, 0.5f * (float)(m_size - 1), (float)(j + 1));
			m_vertices[vert].Color = surfaceColour;
			m_vertices[vert++].Pos = Vector3((float)(i + 1), 0.5f * (float)(m_size - 1), (float)(j + 1));
			
		}
	}



	/*for (int i = 0; i < m_numPrims * 3; i++)
	{
		Vector3 vertScale = m_vertices[i].Pos;

		Matrix scaleMat = Matrix::CreateScale(m_scale, 1.0f, m_scale);

		Vector3 newScale = Vector3::Transform(vertScale, scaleMat);

		m_vertices[i].Pos = newScale;
	}*/
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

	//currVertices[getLoc(m_size / 2, m_size / 2)] += 1.0f;


	//delete[] m_vertices; //this is no longer needed as this is now in the Vertex Buffer
	TwAddVarRW(m_GD->myBar, "Amplitude", TW_TYPE_FLOAT, &rippleAmp, " min=0 max=20 step=0.01 group= Ripple ");
	TwAddVarRW(m_GD->myBar, "Frequency", TW_TYPE_FLOAT, &rippleFreq, " min=0 max=30 step=1 group= Ripple ");
	TwAddVarRW(m_GD->myBar, "Wave Length", TW_TYPE_FLOAT, &rippleWL, " min=0 max=0.5 step=0.01 group= Ripple ");
}



void VBPlane::Tick(GameData* GD)
{
	if (useVerlet)
	{
		if ((GD->keyboard[DIK_R] & 0x80) && !(GD->prevKeyboard[DIK_R] & 0x80))
		{
			memset(newVertices, 0, sizeof(float)*m_size*m_size);
			memset(currVertices, 0, sizeof(float)*m_size*m_size);
		}

		

		if (playerPnt->moving)
		{ 
			
			currVertices[getLoc(playerPnt->publicPos.x/ m_scale, playerPnt->publicPos.z/m_scale)] += disturbance;

			currVertices[getLoc(playerPnt->publicPos.x / m_scale + 1, playerPnt->publicPos.z / m_scale)] -= disturbance/4;
			currVertices[getLoc(playerPnt->publicPos.x / m_scale - 1, playerPnt->publicPos.z / m_scale)] -= disturbance/4;
			currVertices[getLoc(playerPnt->publicPos.x / m_scale, playerPnt->publicPos.z / m_scale + 1)] -= disturbance/4;
			currVertices[getLoc(playerPnt->publicPos.x / m_scale, playerPnt->publicPos.z / m_scale - 1)] -= disturbance/4;


		}		
		TransformVerlet(GD);

	}
	if (useSinSim) //tick for if the sin function based simulation is being used;
	{

		if ((GD->keyboard[DIK_RETURN] & 0x80) && !(GD->prevKeyboard[DIK_RETURN] & 0x80))
		{
			

			/*for (int j = 0; j < m_numVertices; j++)
			{
				float xDiff;
				float zDiff;
				float playerPosOffset;
				xDiff = (GD->player->m_pos.x - m_vertices[j].Pos.x);
				zDiff = (GD->playerPos.z - m_vertices[j].Pos.z);
				playerPosOffset = sqrtf((zDiff*zDiff) + (xDiff*xDiff));

				float cpxDiff;
				float cpzDiff;
				float cpOffset;
				cpxDiff = (m_vertices[m_centrepoint].Pos.x - GD->player.x);
				cpzDiff = (m_vertices[m_centrepoint].Pos.z - GD->playerPos.z);
				cpOffset = sqrtf((cpzDiff*cpzDiff) + (cpxDiff*cpxDiff));

				if (playerPosOffset < cpOffset);
				{
					m_centrepoint = j;
				}
			}*/
			if (useRippleClass)
			{
				m_centrepoint = rand() % (100000);
				//create a new ripple with different origin.
				myRipples.push_back(new Ripple(rippleAmp, rippleFreq, rippleWL, m_vertices[m_centrepoint].Pos.x, m_vertices[m_centrepoint].Pos.z));
				rippleCount++;
			}


		}

		for (list<Ripple *>::iterator it = myRipples.begin(); it != myRipples.end(); it++)
		{

			(*it)->m_time += m_dt;
			(*it)->m_initAmp -= 0.01f;
			if ((*it)->m_initAmp < 0.0f)
			{

				delete (*it);
				it = myRipples.erase(it);
			}

		}


		TransformSin();


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
			m_diagonal++;
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
	
	time += GD->dt;
	VBGO::Tick(GD);
}

void VBPlane::TransformVerlet(GameData* GD)
{
	
	float verl_dt = 0.01f;
	for (int i = 0; i < m_size; i++)
	{

		for (int j = 0; j < m_size; j++)
		{
			

			float UP;
			float DOWN;
			float LEFT;
			float RIGHT;
			UP = currVertices[getLoc(i-1, j)];
			DOWN = currVertices[getLoc(i+1, j)];
			LEFT = currVertices[getLoc(i, j-1)];
			RIGHT = currVertices[getLoc(i, j+1)];
			
			float diffGrad = 30.0f *(UP + DOWN + LEFT + RIGHT - 4.0f *currVertices[getLoc(i,j)]);


			newVertices[getLoc(i, j)] = ((2 * currVertices[getLoc(i, j)]) - newVertices[getLoc(i, j)] + (springForce(currVertices[getLoc(i,j)]) * (verl_dt*verl_dt)));

			newVertices[getLoc(i, j)] += verl_dt * diffGrad;



		}

	}
	
	/*for (int i = 0; i < m_size; i++)
	{
		newVertices[getLoc(i, 0)] = 0.5f * sin(2.0f * time);
	}*/

	for (int i = 0; i < m_numVertices; i++)
	{
		m_vertices[i].Pos.y = newVertices[getLoc(m_vertices[i].Pos.x, m_vertices[i].Pos.z)];
	}
	


 	dummyVertices = newVertices;
	newVertices = currVertices;
	currVertices = dummyVertices;





}

float VBPlane::springForce(float _height)
{
	/*if (_height < 0.0f)
	{
		_height = 0.0f - _height;
	}*/


	float MAX = 10.0f;
	float force = 0.0f;
	force = -(springCoeff * _height);
	if (fabs(force) > MAX)
	{
		force = force > 0 ? MAX : -MAX;
	}

	return(force);

}

int VBPlane::getLoc( int _i, int _j)
{
	//wrap arounds
	if (_i == -1)
	{
		_i = m_size-1;
		//_i = 0;
	}
	if (_i == m_size)
	{
		_i = 0;
		//_i = m_size - 1;
	}

	if (_j == -1)
	{
		_j = m_size-1;
		//_j = 0;
	}
	if (_j == m_size)
	{
		_j = 0;
		//_j = m_size - 1;
	}

	int location = _i * m_size + _j;

	return (location);
}

void VBPlane::TransformSin()
{ 
	for (int j = 0; j < m_numVertices; j++)
	{
		float m_wavesPos = 0.0f;
		float m_ripplePos = 0.0f;

		if (m_waves)
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
		
		if (m_ripple)
		{
			if (useRippleClass)
			{
				for (list<Ripple *>::iterator it = myRipples.begin(); it != myRipples.end(); it++)
				{
					
					m_ripplePos += (*it)->Calculate(m_vertices[j].Pos.x, m_vertices[j].Pos.z);

				}
				m_vertices[j].Pos.y = (m_wavesPos+ m_ripplePos) / (rippleCount +1);

			}
			else
			{
				float newAmp;
				float xDiff;
				float zDiff;
				float cpOffset;
				xDiff = (m_vertices[m_centrepoint].Pos.x - m_vertices[j].Pos.x);
				zDiff = (m_vertices[m_centrepoint].Pos.z - m_vertices[j].Pos.z);
				cpOffset = sqrtf((zDiff*zDiff) + (xDiff*xDiff));

				newAmp = (rippleAmp *(1 - (cpOffset / 600)));

				m_ripplePos = newAmp * sin((rippleFreq * time) + ((cpOffset)* rippleWL));

			}
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
			if (useRippleClass)
			{
				m_vertices[j].Pos.y = ((m_wavesPos + m_ripplePos)/2);
			}
			else
			{
				m_vertices[j].Pos.y = ((m_ripplePos + m_wavesPos)/2);
			}			
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
