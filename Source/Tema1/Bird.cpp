#include <iostream>
#include <Component/SimpleScene.h>
#include <Core/Engine.h>
#include "Bird.h"
#include "Transform2D.h"

Bird::Bird(float x, float y)
{
	setX(x);
	setY(y);
	velocity = 0.0f;
	angle = 0.0f;
	wingsAngle = 200.0f;
}

Bird::~Bird()
{
}

//Genereaza punctele pentru un cerc de raza CIRCLE_RADIUS, format din 360 de puncte
Mesh* Bird::CreateCircle(std::string name)
{
	float radius = CIRCLE_RADIUS;

	std::vector<VertexFormat> circleVertices;
	for (int i = 0; i <= 360; i++)
	{
		float theta = 2.0f * M_PI * float(i) / float(360);

		float x = radius * glm::cos(theta);
		float y = radius * glm::sin(theta);

		circleVertices.push_back(VertexFormat(glm::vec3(x, y, 0), glm::vec3(0, 1, 1)));
	}

	std::vector<unsigned short> circleIndices;
	for (int i = 1; i < circleVertices.size(); i++)
	{
		circleIndices.push_back(i);
		circleIndices.push_back(0);
		circleIndices.push_back(i + 1);
	}

	Mesh* circle = new Mesh(name);
	circle->InitFromData(circleVertices, circleIndices);
	return circle;
}

//Genereaza punctele pentru un triunghi
Mesh* Bird::CreateTriangle(std::string name)
{
	float radius = 5.0;

	std::vector<VertexFormat> triangleVertices =
	{
		VertexFormat(glm::vec3(-1, -1,  0), glm::vec3(0, 1, 1)),
		VertexFormat(glm::vec3(-1, 1,  0), glm::vec3(0, 1, 1)),
		VertexFormat(glm::vec3(1, 0, 0), glm::vec3(0, 1, 1))
	};

	std::vector<unsigned short> triangleIndices =
	{
		0, 1, 2
	};

	Mesh* triangle = new Mesh(name);
	triangle->InitFromData(triangleVertices, triangleIndices);
	return triangle;
}

//Genereaza punctele pentru un triunghi dreptunghic
Mesh* Bird::CreateRightTriangle(std::string name)
{
	std::vector<VertexFormat> triangleVertices =
	{
		VertexFormat(glm::vec3(0, 0,  0), glm::vec3(0, 1, 1)),
		VertexFormat(glm::vec3(0, 1,  0), glm::vec3(0, 1, 1)),
		VertexFormat(glm::vec3(1, 0, 0), glm::vec3(0, 1, 1))
	};

	std::vector<unsigned short> triangleIndices =
	{
		0, 1, 2
	};

	Mesh* triangle = new Mesh(name);
	triangle->InitFromData(triangleVertices, triangleIndices);
	return triangle;
}

//Genereaza punctele pentru un triunghi dreptunghic cu orientarea inversa fata de cel de mai sus
//pentru a evita rotatii suplimentare
Mesh* Bird::CreateRightTriangle2(std::string name)
{
	std::vector<VertexFormat> triangleVertices =
	{
		VertexFormat(glm::vec3(0, 0,  0), glm::vec3(0, 1, 1)),
		VertexFormat(glm::vec3(1, 1,  0), glm::vec3(0, 1, 1)),
		VertexFormat(glm::vec3(1, 0, 0), glm::vec3(0, 1, 1))
	};

	std::vector<unsigned short> triangleIndices =
	{
		0, 1, 2
	};

	Mesh* triangle = new Mesh(name);
	triangle->InitFromData(triangleVertices, triangleIndices);
	return triangle;
}

void Bird::Init()
{
	//initializeaza meshruile necesare pentru a desena pasarea
	body = CreateCircle("body");
	eye = CreateCircle("eye");
	eye1 = CreateCircle("eye1");
	pupil1 = CreateCircle("n1");
	pupil2 = CreateCircle("n2");
	beak1 = CreateTriangle("beak1");
	beak2 = CreateTriangle("beak2");
	wing1 = CreateTriangle("wing1");
	wing2 = CreateTriangle("wing2");
	wing3 = CreateTriangle("wing3");
	eyebrow1 = CreateRightTriangle("eyebrow1");
	eyebrow2 = CreateRightTriangle2("eyebrow2");

}

bool Bird::shouldJump()
{
	return jump;
}

void Bird::setJump(bool jump)
{
	this->jump = jump;
}

void Bird::setX(float x)
{
	//pozitia fiecarei componente a pasarii se calculeaza relativ la centrul acesteia
	bodyX = x;
	eye1X = x + 25;
	eye2X = x + 5;
	pupil1X = x + 30;
	pupil2X = x + 10;
	eyebrow1X = x - 5;
	eyebrow2X = x + 15;
	wing1X = x - 28;
	wing2X = x - 28;
	wing3X = x - 28;
	beak1X = x + 23;
	beak2X = x + 13;
}

void Bird::setY(float y)
{
	//pozitia fiecarei componente a pasarii se calculeaza relativ la centrul acesteia
	bodyY = y;
	eye1Y = y;
	eye2Y = y;
	pupil1Y = y;
	pupil2Y = y;
	eyebrow1Y = y + 5;
	eyebrow2Y = y + 5;
	wing1Y = y;
	wing2Y = y - 10;
	wing3Y = y - 20;
	beak1Y = y - 20;
	beak2Y = y - 30;
}

float Bird::getBodyX()
{
	return bodyX;
}

float Bird::getBodyY()
{
	return bodyY;
}

float Bird::getEye1X()
{
	return eye1X;
}

float Bird::getEye1Y()
{
	return eye1Y;
}

float Bird::getEye2X()
{
	return eye2X;
}

float Bird::getEye2Y()
{
	return eye2Y;
}

float Bird::getPupil1X()
{
	return pupil1X;
}

float Bird::getPupil1Y()
{
	return pupil1Y;
}

float Bird::getPupil2X()
{
	return pupil2X;
}

float Bird::getPupil2Y()
{
	return pupil2Y;
}

float Bird::getEyeBrow1X()
{
	return eyebrow1X;
}

float Bird::getEyeBrow1Y()
{
	return eyebrow1Y;
}

float Bird::getEyeBrow2X()
{
	return eyebrow2X;
}

float Bird::getEyeBrow2Y()
{
	return eyebrow2Y;
}

float Bird::getWing1X()
{
	return wing1X;
}

float Bird::getWing1Y()
{
	return wing1Y;
}

float Bird::getWing2X()
{
	return wing2X;
}

float Bird::getWing2Y()
{
	return wing2Y;
}

float Bird::getWing3X()
{
	return wing3X;
}

float Bird::getWing3Y()
{
	return wing3Y;
}

float Bird::getBeak1X()
{
	return beak1X;
}

float Bird::getBeak1Y()
{
	return beak1Y;
}

float Bird::getBeak2X()
{
	return beak2X;
}

float Bird::getBeak2Y()
{
	return beak2Y;
}

float Bird::getWingsAngle()
{
	return wingsAngle;
}

void Bird::setWingsAngle(float angle)
{
	this->wingsAngle = angle;
}

void Bird::setWingsMovementType(bool type)
{
	this->wingsIncrease = type;
}

float Bird::getWingsMovementType()
{
	return wingsIncrease;
}

float Bird::getVelocity()
{
	return velocity;
}

void Bird::setVelocity(float velocity)
{
	this->velocity = velocity;
}

void Bird::setAngle(float angle)
{
	this->angle = angle;
}

float Bird::getAngle()
{
	return angle;
}
