#ifndef _RIPPLE_H_
#define _RIPPLE_H_
#include "gameobject.h"

class Ripple
{
public:
	Ripple(float _RA, float _RF, float _RWL, float _CPX, float _CPZ);
	~Ripple(){};
	
	float Calculate(float vertX, float vertZ);
	float m_initAmp;
	float m_initFreq;
	float m_initWL;
	float m_originX;
	float m_originZ;
	float m_time = 0;
	float m_ripplePos;

};


#endif