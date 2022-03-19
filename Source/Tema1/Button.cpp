#include "Button.h"

Button::Button(float x, float y, Mesh* mesh, int type, bool toggle, bool enabled, float deltaX)
{
	position.x = x;
	position.y = y;
	this->mesh = mesh;
	this->type = type;
	this->toggle = toggle;
	this->enabled = enabled;
	this->deltaX = deltaX;
}

int Button::trackState(float mouseX, float mouseY, int lengthX, int lengthY)
{
	bool mouseOnButton = false;
	mouseY = 720 - mouseY;

	if (mouseX >= position.x && mouseX <= position.x + lengthX)
	{
		if (mouseY >= position.y && mouseY <= position.y + lengthY)
		{
			mouseOnButton = true;
		}
	}

	if (mouseOnButton)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
