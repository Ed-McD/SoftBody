#include "Ripple.h"


//An instance of this class will hold the variables needed for calculating each ripple

Ripple::Ripple(float _RA, float _RF, float _RWL, float _CPX, float _CPZ)
{
	m_initAmp = _RA;
	m_initFreq = _RF;
	m_initWL = _RWL;
	m_originX = _CPX;
	m_originZ = _CPZ;
}

float Ripple::Calculate( float vertX, float vertZ)
{
	
	float newAmp;
	float newWL;
	float newFreq;
	float xDiff;
	float zDiff;
	float cpOffset;
	
	
	xDiff = (m_originX - vertX);
	zDiff = (m_originZ - vertZ);
	cpOffset = sqrtf((zDiff*zDiff) + (xDiff*xDiff));

	//Reduces the freq, WL and Amp the further the vertx is from the centrepoint.
	newAmp = (m_initAmp *(1 - (cpOffset / 1000)));
	newWL = (m_initWL *(1 - (cpOffset / 1000)));
	newFreq = (m_initFreq *(1 - (cpOffset / 1000)));

	m_ripplePos = newAmp * sin((newFreq * m_time) + ((cpOffset)* newWL));
	return (m_ripplePos);

}