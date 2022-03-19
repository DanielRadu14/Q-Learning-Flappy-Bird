#pragma once

#include <Core/GPU/Mesh.h>

enum ObstacleType 
{
	BOTTOM_OBSTACLE,
	TOP_OBSTACLE,
};

class Obstacle
{
public:
	Obstacle(Mesh * mesh);
	~Obstacle();


public:
	static int count;

	float getWidth();
	float getHeight();
	void setWidth(float width);
	void setHeight(float height);
	ObstacleType getObstacleType();
	void setObstacleType(ObstacleType type);
	Mesh* getMesh();
	float getPositionX();
	float getPositionY();
	void setPositionX(float x);
	void setPositionY(float y);
	int id = 0;

	float getScaleFactorForWidth(float newWidth);
	float getScaleFactorForHeight(float newHeight);

	float getWidthForScaleFactor(float scaleFactor);
	float getHeightForScaleFactor(float scaleFactor);

private:
	//dimensiunile de baza ale dreptunghiului
	const float RECTANGLE_WIDTH = 1.0f;
	const float RECTANGLE_HEIGHT = 2.0f;
	float width;
	float height;
	float positionX;
	float positionY;
	float scaleFactor;
	Mesh* mesh;

	ObstacleType type;


};

