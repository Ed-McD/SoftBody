#include "gamedata.h"
#include "VBPLane.h"
#include "drawdata.h"
#include "Turret_base.h"
#include <AntTweakBar.h>

void VBPlane::init(int _size, float _scale, GameData* _GD , ID3D11Device* GD)
{
	///Set initial values for variables of the simulation///

	//Set pointers and global toggles.
	m_GD = _GD;
	m_Device = GD;
	m_size = _size;
	m_scale = _scale;
	useRippleClass = true;
	useVerlet = !useSinSim;
	useSinSim = false; 
	recalculateNorms = true;
	m_ripple = false;
	m_waves = true; 

	//Sin sim variables
	time = 0.0f;
	freq = 2.0f;
	amp = 2.5f;
	waveLength = 0.025f;
	rippleFreq = 20.0f;
	rippleAmp = 15.0f;
	rippleWL = 0.1f;
	rippleFalloff = 0.75f;
	m_diagonal = 0;	
	rippleCount = 0;

	//Verlet sim variables
	verletAmp = 1.0f;
	verletFreq = 5.0f;
	verletWL = 2;	
	springCoeff = 0.1f;
	disturbance = 0.1f;	
	WaveSpeed = 0.5f;
	dampingForce = 0.05f;	
	wrapAround = false;
	invertDisturbance = true;	

	//Colour and alpha of the surface
	float aLvl = 0.25f;
	Color surfaceColour = {0.0f, 0.0f, 1.0f, aLvl};


	//calculate number of vertices and primatives
	int numVerts = 6 * (m_size - 1) * (m_size - 1);
	m_numPrims = numVerts / 3;
	m_numVertices = numVerts;
	m_vertices = new myVertex[numVerts];
	WORD* indices = new WORD[numVerts];

	//Initialize the arrays for the verlet intergration.
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

		//store the normals of the flat plane so it can be reset if normals arent being recalculated every tick
		initNormals.push_back(norm);
	}


	BuildIB(GD, indices);

	//Calls the inherited function to set up the Dynamic Vertex Buffer for this object.
	BuildDVB(GD, numVerts, m_vertices);

	//Calls the inherited function to set up a raster state with wireframe enabled.
	BuildWFRaster(GD);

	//Adding all the variables to the Teak Bar
	TwAddVarRW(m_GD->myBar, "Using Sin Based Simulation", TW_TYPE_BOOLCPP, &useSinSim, "group= Main Toggles ");	
	TwAddVarRO(m_GD->myBar, "Using Verlet Simulation", TW_TYPE_BOOLCPP, &useVerlet, " group= Main Toggles ");
	TwAddVarRW(m_GD->myBar, "Recalculate Normals", TW_TYPE_BOOLCPP, &recalculateNorms, " group= Main Toggles ");
	TwAddVarRW(m_GD->myBar, "WireFrame", TW_TYPE_BOOLCPP, &wireframe, " group= Main Toggles ");
	

	TwAddVarRW(m_GD->myBar, "Wave direction", TW_TYPE_INT8, &m_diagonal, "min = 0 max = 3 step = 1 group= SinBased ");
	TwAddVarRW(m_GD->myBar, "Amplitude", TW_TYPE_FLOAT, &amp, " min=0 max=20 step=0.01 group= SinBased");
	TwAddVarRW(m_GD->myBar, "Frequency", TW_TYPE_FLOAT, &freq, " min=0 max=20 step=0.01 group= SinBased");
	TwAddVarRW(m_GD->myBar, "Wave Length", TW_TYPE_FLOAT, &waveLength, " min=0 max=20 step=0.01 group= SinBased");

	TwAddVarRW(m_GD->myBar, "Ripple Amplitude", TW_TYPE_FLOAT, &rippleAmp, " min=0 max=20 step=0.01 group= Ripple ");
	TwAddVarRW(m_GD->myBar, "Ripple Frequency", TW_TYPE_FLOAT, &rippleFreq, " min=0 max=30 step=1 group= Ripple ");
	TwAddVarRW(m_GD->myBar, "Ripple Wavelength", TW_TYPE_FLOAT, &rippleWL, " min=0 max=0.5 step=0.01 group= Ripple ");
	TwAddVarRW(m_GD->myBar, "Ripple Falloff", TW_TYPE_FLOAT, &rippleFalloff, " min=0 max=2 step=0.01 group= Ripple ");

	//Regrouping and labeling
	TwDefine("VariableMenu/Ripple group=SinBased");
	TwDefine("VariableMenu/Ripple label = 'Ripple (Press Enter to create a ripple)'");

	//Adding Verlet Variables
	TwAddVarRW(m_GD->myBar, "WaveSpeed", TW_TYPE_FLOAT, &WaveSpeed, " min=0 max=40 step=0.05 group= Verlet label = 'Wave Speed' ");
	TwAddVarRW(m_GD->myBar, "Disturbance", TW_TYPE_FLOAT, &disturbance, " min=0 max=10 step=0.01 group= Verlet");
	TwAddVarRW(m_GD->myBar, "Damping Force", TW_TYPE_FLOAT, &dampingForce, " min=0 max=0.2 step=0.001 group= Verlet");	
	TwAddVarRW(m_GD->myBar, "Spring Coefficient", TW_TYPE_FLOAT, &springCoeff, "min=0 max=1 step = 0.01 group= Verlet ");

	TwAddVarRW(m_GD->myBar, "Disturbance WaveLength", TW_TYPE_INT8, &verletWL, "min = 1 max = 5 step = 1 group= Verlet ");
	TwAddVarRW(m_GD->myBar, "Wave Wrap Around", TW_TYPE_BOOLCPP, &wrapAround, "group= Verlet ");
	TwAddVarRW(m_GD->myBar, "Invert Distubance", TW_TYPE_BOOLCPP, &invertDisturbance, " group= Verlet ");
	TwAddVarRW(m_GD->myBar, "Sin Wave propogation", TW_TYPE_BOOLCPP, &verletSin, " group= Verlet ");
	TwAddVarRW(m_GD->myBar, "Verlet Amplitude", TW_TYPE_FLOAT, &verletAmp, " min=0 max=5 step=0.1 group= VerletSin ");
	TwAddVarRW(m_GD->myBar, "Verlet Frequency", TW_TYPE_FLOAT, &verletFreq, " min=0 max=10 step=0.5 group= VerletSin ");	

	//Regrouping
	TwDefine("VariableMenu/VerletSin group=Verlet");
	

}



void VBPlane::Tick(GameData* GD)
{
	//Scales the step value in the Tweak Bar depending on current magnitude;
	if (disturbance >= 5)
	{
		TwDefine("VariableMenu/Disturbance step = 1.0");
	}
	else if (disturbance >= 0.5)
	{
		TwDefine("VariableMenu/Disturbance step = 0.05");
	}
	else
	{
		TwDefine("VariableMenu/Disturbance step = 0.01");
	}
	//Different Varibale Scaling.
	if (WaveSpeed >= 5)
	{
		TwDefine("VariableMenu/WaveSpeed step = 1.0");
	}
	else if (WaveSpeed>=0.5)
	{
		TwDefine("VariableMenu/WaveSpeed step = 0.05");
	}
	else
	{
		TwDefine("VariableMenu/WaveSpeed step = 0.01");
	}


	//Ensures the different simulations aren't running at once
	if (useSinSim)
	{
		useVerlet = false;
		m_ripple = true;
	}
	else
	{
		useVerlet = true;
		m_ripple = false;
	}

	//If using the verlet simulation...
	if (useVerlet)
	{
		//Close the sin based simulation menu and open the verlet menu
		TwDefine("VariableMenu/SinBased opened=false");
		TwDefine("VariableMenu/Verlet opened=true");

		//Sets all the vertex heights to 0 if 'R' is pressed
		if ((GD->keyboard[DIK_R] & 0x80) && !(GD->prevKeyboard[DIK_R] & 0x80))
		{
			memset(newVertices, 0, sizeof(float)*m_size*m_size);
			memset(currVertices, 0, sizeof(float)*m_size*m_size);
		}

		
		//Check to see if the player is moving...
		if (playerPnt->moving)
		{
			if (invertDisturbance)
			{
				//...increase the vertex the player is closest to by the disturbance value...
				currVertices[getLoc(playerPnt->publicPos.x / m_scale, playerPnt->publicPos.z / m_scale)] += disturbance;

				//...and decrease 4 of the vertices around the player by a scaled factor of 4 to average to 0
				//VerletWL changes the distance between the current centre of this ripple and the ones around it being incremented. 

				currVertices[getLoc(playerPnt->publicPos.x / m_scale + (verletWL * 1), playerPnt->publicPos.z / m_scale)] -= disturbance / (verletWL * 4);
				currVertices[getLoc(playerPnt->publicPos.x / m_scale - (verletWL * 1), playerPnt->publicPos.z / m_scale)] -= disturbance / (verletWL * 4);
				currVertices[getLoc(playerPnt->publicPos.x / m_scale, playerPnt->publicPos.z / m_scale + (verletWL * 1))] -= disturbance / (verletWL * 4);
				currVertices[getLoc(playerPnt->publicPos.x / m_scale, playerPnt->publicPos.z / m_scale - (verletWL * 1))] -= disturbance / (verletWL * 4);
			}
			else
			{
				//...decrease the vertex the player is closest to by the disturbance value...
				currVertices[getLoc(playerPnt->publicPos.x / m_scale, playerPnt->publicPos.z / m_scale)] -= disturbance;

				//...and increase 4 of the vertices around the player by a scaled factor of 4 to average to 0
				//VerletWL changes the distance between the current centre of this ripple and the ones around it being incremented. 

				currVertices[getLoc(playerPnt->publicPos.x / m_scale + (verletWL * 1), playerPnt->publicPos.z / m_scale)] += disturbance / (verletWL * 4);
				currVertices[getLoc(playerPnt->publicPos.x / m_scale - (verletWL * 1), playerPnt->publicPos.z / m_scale)] += disturbance / (verletWL * 4);
				currVertices[getLoc(playerPnt->publicPos.x / m_scale, playerPnt->publicPos.z / m_scale + (verletWL * 1))] += disturbance / (verletWL * 4);
				currVertices[getLoc(playerPnt->publicPos.x / m_scale, playerPnt->publicPos.z / m_scale - (verletWL * 1))] += disturbance / (verletWL * 4);
			}
		}
		//Call the verlet transform function;
		TransformVerlet(GD);

	}

	//If using the sin simulation...
	if (useSinSim) 
	{	
		//..close the verlet menu and open the sin menu
		TwDefine("VariableMenu/Verlet opened=false");
		TwDefine("VariableMenu/SinBased opened=true");
		
		//Spawn a ripple on 'Enter'
		if ((GD->keyboard[DIK_RETURN] & 0x80) && !(GD->prevKeyboard[DIK_RETURN] & 0x80))
		{
			
			if (useRippleClass)
			{	
				//create a random origin
				m_centrepoint = rand() % (100000);
				//create a new ripple instance and add it to the ripple list.
				myRipples.push_back(new Ripple(rippleAmp, rippleFreq, rippleWL, m_vertices[m_centrepoint].Pos.x, m_vertices[m_centrepoint].Pos.z));
				//increment the ripple count
				rippleCount++;
			}

		}

		//Iterate through the ripples
		for (list<Ripple *>::iterator it = myRipples.begin(); it != myRipples.end(); it++)
		{
			//Add the dt to the ripples time value ripples
			(*it)->m_time += m_dt;
			//reduce the amplitude of the ripple by the falloff factor;
			(*it)->m_initAmp -= (rippleFalloff *0.01f);
			
			//If the amplitude is below 0, or the reset has been pressed...
			if ((*it)->m_initAmp < 0.0f || (GD->keyboard[DIK_R] & 0x80) && !(GD->prevKeyboard[DIK_R] & 0x80))
			{
				//...delete the instance of the ripple class...
				delete (*it);
				//...and remove the pointer from the list
				it = myRipples.erase(it);
			}

		}

		//Call the sin transform function
		TransformSin();		

	}

	//If reclaulate norms is true...
	if (recalculateNorms)
	{ 
		//..recalculate the normals for the basic lighting in the base shader
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
		//Norms are not reset to initial values.
		normsReset = false;
	}
	//If norms arent reset to initial values...
	else if (!normsReset)
	{
		//run through all the vertices and set to the flat plane values
		for (int i = 0; i < m_numPrims; i++)
		{
			WORD V1 = 3 * i;
			WORD V2 = 3 * i + 1;
			WORD V3 = 3 * i + 2;

			m_vertices[V1].Norm = initNormals[i];
			m_vertices[V2].Norm = initNormals[i];
			m_vertices[V3].Norm = initNormals[i];
		}
		//Norms have been reset to default
		normsReset = true;
	}
	
	//Add the tick to the time counter;
	time += GD->dt;
	VBGO::Tick(GD);
}

void VBPlane::TransformVerlet(GameData* GD)
{
	
	float verl_dt = 0.01f; //Fixed value for dt as it can be unstable otherwise.

	//nested for loops run through all the vertices.
	for (int i = 0; i < m_size; i++)
	{

		for (int j = 0; j < m_size; j++)
		{
			
			//Find the current heights of all the vertices around the current one.
			float UP;
			float DOWN;
			float LEFT;
			float RIGHT;

			UP = currVertices[getLoc(i - 1, j)];
			DOWN = currVertices[getLoc(i + 1, j)];
			LEFT = currVertices[getLoc(i, j - 1)];
			RIGHT = currVertices[getLoc(i, j + 1)];

			// The actual Verlet intergration.
			float diffGrad = WaveSpeed *(UP + DOWN + LEFT + RIGHT - 4.0f *currVertices[getLoc(i,j)]);

			newVertices[getLoc(i, j)] = (((2 - dampingForce)* currVertices[getLoc(i, j)]) - (1 - dampingForce)*newVertices[getLoc(i, j)] + (springForce(currVertices[getLoc(i, j)])* (verl_dt*verl_dt)));

			newVertices[getLoc(i, j)] += verl_dt * diffGrad;



		}

	}
	
	if (verletSin)
	{	//Keep the verlet sin menu group open.
		TwDefine("VariableMenu/VerletSin opened=true");

		for (int i = 0; i < m_size; i++)
		{	//Overide the new positions for the edge vertices with a height base upon a sin wave
			newVertices[getLoc(i, 0)] = verletAmp * sin(verletFreq * time); 
		}
	}
	else
	{	//Keep the verlet sin menu group closed.
		TwDefine("VariableMenu/VerletSin opened=false"); 
	}

	for (int i = 0; i < m_numVertices; i++)
	{	
		//copy the valuse from new vertices into the y value for the vertices.
		m_vertices[i].Pos.y = newVertices[getLoc(m_vertices[i].Pos.x, m_vertices[i].Pos.z)]; 
	}
	

	//Swap the arrays around ready for the verlet intergration next tick.
 	dummyVertices = newVertices;
	newVertices = currVertices;
	currVertices = dummyVertices;

}

//Hookes law calculation of springforce;
float VBPlane::springForce(float _height)
{
	float MAX = 10.0f;
	float force = 0.0f;
	force = -(springCoeff * _height);
	if (fabs(force) > MAX)
	{
		force = force > 0 ? MAX : -MAX;
	}

	return(force);

}


//This functions takes 2 dimensional co-ordinates and converts them to a 1-dimensional location in an array.
int VBPlane::getLoc( int _i, int _j)
{
	//If wraparound is true...
	if (wrapAround)
	{
		//...sets to opposite side. This is used for the vertices around the current vertex.
		if (_i <= -1)
		{
			_i = m_size - 1;			
		}
		if (_i >= m_size)
		{
			_i = 0;			
		}
		if (_j <= -1)
		{
			_j = m_size - 1;			
		}
		if (_j >= m_size)
		{
			_j = 0;			
		}
	}
	else
	{
		//Sets to self
		if (_i <= -1)
		{
			_i = 0;
		}
		if (_i >= m_size)
		{
			_i = m_size - 1;
		}
		if (_j <= -1)
		{
			_j = 0;
		}
		if (_j >= m_size)
		{
			_j = m_size - 1;
		}

	}

	int location = _i * m_size + _j;

	return (location);
}

void VBPlane::TransformSin()
{ 
	// Runs through all the vertices
	for (int j = 0; j < m_numVertices; j++)
	{
		float m_wavesPos = 0.0f;
		float m_ripplePos = 0.0f;

		if (m_waves)
		{
			//Switch direction of the waves
			switch (m_diagonal)
			{
			case 0:
				//Basic sin function, the position in x/z is taken in to determine each vertexs height.
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
		//If ripples are being used..
		if (m_ripple)
		{
			//
			if (useRippleClass)
			{
				//iterate through the ripples and add the value for each vetex for each ripple to the ripplePos...
				for (list<Ripple *>::iterator it = myRipples.begin(); it != myRipples.end(); it++)
				{
					
					m_ripplePos += (*it)->Calculate(m_vertices[j].Pos.x, m_vertices[j].Pos.z);

				}
				//..then divide ripplePos by the number of ripples for the average height for the vertex.
				
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
}

//F
void VBPlane::Draw(DrawData* _DD)
{
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//Disable GPU access to the vertex buffer data.
	_DD->pd3dImmediateContext->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	myVertex* p_vertices = (myVertex*)mappedResource.pData;

	//Update the vertex buffer here.
	memcpy(p_vertices, (void*)m_vertices, sizeof(myVertex) * m_numVertices);

	//Reenable GPU access to the vertex buffer data.
 	_DD->pd3dImmediateContext->Unmap(m_VertexBuffer, 0);

 	VBGO::Draw(_DD);
	
}
