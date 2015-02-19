#include "Ripple.h"



Ripple::Ripple(float _RA, float _RF, float _RWL, float _CPX, float _CPZ)
{
	m_initAmp = _RA;
	m_initFreq = _RF;
	m_initWL = _RWL;
	m_originX = _CPX;
	m_originZ = _CPZ;
}

float Ripple::Calculate(float _dt, float vertX, float vertZ)
{
	m_time = m_time + _dt;
	float newAmp;
	float xDiff;
	float zDiff;
	float cpOffset;
	xDiff = (m_originX - vertX);
	zDiff = (m_originZ - vertZ);
	cpOffset = sqrtf((zDiff*zDiff) + (xDiff*xDiff));

	newAmp = (m_initAmp *(1 - (cpOffset / 600)));

	m_ripplePos = newAmp * sin((m_initFreq * m_time) + ((cpOffset)* m_initWL));
	return (m_ripplePos);

}