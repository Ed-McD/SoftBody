#ifndef _GAME_DATA_H_
#define _GAME_DATA_H_
#include "gamestate.h"
#include <dinput.h>
#include "SimpleMath.h"
#include "AntTweakTest.h"

using namespace DirectX;
using namespace SimpleMath;

struct GameData
{
	float dt; //duration of previous frame
	unsigned char* keyboard; //current keyboard state
	unsigned char* prevKeyboard; //previous frame's keyboard state
	DIMOUSESTATE* mouse; //current mouse state
	GameState GS; //current game state
	Vector3 playerPos;
	TwBar* myBar;
};

#endif