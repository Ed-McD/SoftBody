#ifndef _VB_PLANE_H_
#define _VB_PLANE_H_
#include "VBGO.h"
#include "vertex.h"
#include "Ripple.h"
#include <list>


class Turret_Base;

using namespace std;


//procedurally generate a VBGO Cube
//each side be divided in to _size * _size squares (2 triangles per square)

class VBPlane : public VBGO
{
public:
	
	float verletAccel;
	float amp;
	float freq;
	float waveLength;
	float rippleAmp;
	float rippleFreq;
	float rippleWL;
	float rippleFalloff;
	float time;
	float m_numVertices;
	float m_dt;

	float m_scale;
	float disturbance;

	float verletDamping;
	float springCoeff;
	

	int m_diagonal;
	int m_centrepoint;
	int rippleCount= 0;
	bool m_ripple;
	bool m_waves;
	bool useSinSim;
	bool useRippleClass;
	bool useVerlet;

	Turret_Base* playerPnt;
	GameData* m_GD;
	ID3D11Device* m_Device;

	list <Ripple*> myRipples;
	
	VBPlane(){};
	virtual ~VBPlane()
	{
		delete newVertices;
		delete currVertices;
		//delete dummyVertices;
	};

	//initialise the Veretx and Index buffers for the cube
	void init(int _size, float _scale, GameData* GD ,ID3D11Device* _GD);
	void Tick(GameData* GD);

	virtual void Draw(DrawData* _DD);

protected:
	//this is to allow custom versions of this which create the basic cube and then distort it
	//see Spirla, SpikedVB and Pillow
	virtual void TransformSin();
	void TransformVerlet(GameData* _GD);
	int getLoc(int _i, int _j);
	float springForce(float _height);
	
	int m_size;
	myVertex* m_vertices;
	float* newVertices;
	float* currVertices;
	float* dummyVertices;
	
	

};

#endif