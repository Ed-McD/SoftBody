#ifndef _VB_PLANE_H_
#define _VB_PLANE_H_
#include "VBGO.h"
#include "vertex.h"
#include "Ripple.h"
#include <list>

using namespace std;


//procedurally generate a VBGO Cube
//each side be divided in to _size * _size squares (2 triangles per square)

class VBPlane : public VBGO
{
public:
	
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
	int m_diagonal;
	int m_centrepoint;
	int rippleCount= 0;
	bool m_ripple;
	bool m_waves;
	bool useSinSim;
	bool useRippleClass;

	list <Ripple*> myRipples;
	
	VBPlane(){};
	virtual ~VBPlane(){};

	//initialise the Veretx and Index buffers for the cube
	void init(int _size, ID3D11Device* _GD);
	void Tick(GameData* GD);

	virtual void Draw(DrawData* _DD);

protected:
	//this is to allow custom versions of this which create the basic cube and then distort it
	//see Spirla, SpikedVB and Pillow
	virtual void TransformSin();
	
	int m_size;
	myVertex* m_vertices;	
	

};

#endif