#pragma once

#include <Core/GPU/Mesh.h>
#include <iostream>

class Bird
{
public:
	Bird::Bird(float x, float y);
	~Bird();

private:
	Mesh* CreateCircle(std::string name);
	Mesh* CreateTriangle(std::string name);
	Mesh* CreateRightTriangle(std::string name);
	Mesh* CreateRightTriangle2(std::string name);

public:
	void Init();
    bool shouldJump();
	void setJump(bool jump);

	void setX(float x);
	void setY(float y);

	float getBodyX();
	float getBodyY();
	float getEye1X();
	float getEye1Y();
	float getEye2X();
	float getEye2Y();
	float getPupil1X();
	float getPupil1Y();
	float getPupil2X();
	float getPupil2Y();
	float getEyeBrow1X();
	float getEyeBrow1Y();
	float getEyeBrow2X();
	float getEyeBrow2Y();
	float getWing1X();
	float getWing1Y();
	float getWing2X();
	float getWing2Y();
	float getWing3X();
	float getWing3Y();
	float getBeak1X();
	float getBeak1Y();
	float getBeak2X();
	float getBeak2Y();
	float getWingsAngle();
	void setWingsAngle(float angle);
	float getWingsMovementType();
	void setWingsMovementType(bool type);

	float getVelocity();
	void setVelocity(float velocity);

	void setAngle(float angle);
	float getAngle();

	//cu cat se va inclina pasarea cand are viteza pozitiva pe Oy versus cand are viteza negativa(coboara)
	const float ASCENDING_ANGLE = M_PI / 6;
	const float DESCENDING_ANGLE = -M_PI / 6;

	float nextRectangleMoveTime = 0.0f;
	float anteriorObstacleHeight1 = -1;
	float anteriorObstacleHeight2 = -1;
	float anteriorTimeInterval = 0;

private:
	const float CIRCLE_RADIUS = 5;

	float bodyX, bodyY;
	float eye1X, eye1Y;
	float eye2X, eye2Y;
	float pupil1X, pupil1Y;
	float pupil2X, pupil2Y;
	float eyebrow1X, eyebrow1Y;
	float eyebrow2X, eyebrow2Y;
	float wing1X, wing1Y;
	float wing2X, wing2Y;
	float wing3X, wing3Y;
	float beak1X, beak1Y;
	float beak2X, beak2Y;

	float velocity;
	float angle;
	float wingsAngle;
	float wingsIncrease;

	bool jump;

public:
	Mesh* body;
	Mesh* eye;
	Mesh* eye1;
	Mesh* pupil1;
	Mesh* pupil2;
	Mesh* beak1;
	Mesh* beak2;
	Mesh* wing1;
	Mesh* wing2;
	Mesh* wing3;
	Mesh* eyebrow1;
	Mesh* eyebrow2;

private:
};

