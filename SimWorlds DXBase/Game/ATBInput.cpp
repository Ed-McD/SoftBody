#include "ATBInput.h"


void ATBInput::processInput( GameData *GD)
{
	if (GD->mouse->rgbButtons[0])
	{
		TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
	}
	else
	{
		TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
	}

}