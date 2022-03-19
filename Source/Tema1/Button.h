#pragma once
#include <Core/Engine.h>
#include <iostream>
class Button
{
public:
	glm::vec2 position;
	Mesh* mesh;
	int type;
	bool toggle;
	bool enabled;
	float deltaX;

	Button();
	Button(float x, float y, Mesh* mesh, int type, bool toggle, bool enabled, float deltaX);
	int trackState(float mouseX, float mouseY, int lengthX, int lengthY);
};

