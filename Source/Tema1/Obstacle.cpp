#include "Obstacle.h"

int Obstacle::count;

Obstacle::Obstacle(Mesh * mesh)
{
	id = count++;
	width = RECTANGLE_WIDTH;
	height = RECTANGLE_HEIGHT;
	type = BOTTOM_OBSTACLE;
	this->mesh = mesh;
}

Obstacle::~Obstacle()
{

}

float Obstacle::getWidth()
{
	return this->width;
}

float Obstacle::getHeight()
{
	return this->height;
}

void Obstacle::setWidth(float width)
{
	this->width = width;
}

void Obstacle::setHeight(float height)
{
	this->height = height;
}

ObstacleType Obstacle::getObstacleType()
{
	return this->type;
}

void Obstacle::setObstacleType(ObstacleType type)
{
	this->type = type;
}

Mesh* Obstacle::getMesh()
{
	return this->mesh;
}

float Obstacle::getPositionX()
{
	return this->positionX;
}

float Obstacle::getPositionY()
{
	return this->positionY;
}

void Obstacle::setPositionX(float x)
{
	this->positionX = x;
}

void Obstacle::setPositionY(float y)
{
	this->positionY = y;
}

//Stiind dimensiunea initiala a dreptunghiului, determina factorul de scalare ce il aduce la dimensiunea dorita
float Obstacle::getScaleFactorForWidth(float newWidth)
{
	return newWidth / RECTANGLE_WIDTH;
}

//Stiind dimensiunea initiala a dreptunghiului, determina factorul de scalare ce il aduce la dimensiunea dorita
float Obstacle::getScaleFactorForHeight(float newHeight)
{
	return newHeight / RECTANGLE_HEIGHT;
}

//Stiind dimensiunea initiala a dreptunghiului, determina noua dimensiune aplicand un factor de scalare
float Obstacle::getWidthForScaleFactor(float scaleFactor)
{
	return RECTANGLE_WIDTH * scaleFactor;
}

//Stiind dimensiunea initiala a dreptunghiului, determina noua dimensiune aplicand un factor de scalare
float Obstacle::getHeightForScaleFactor(float scaleFactor)
{
	return RECTANGLE_HEIGHT * scaleFactor;
}
