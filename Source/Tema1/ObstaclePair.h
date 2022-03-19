#pragma once

#include "OBstacle.h"

enum ScalingType
{
	OBSTACLE_GROWING,
	OBSTACLE_SHRINKING
};

class ObstaclePair
{
public:
	ObstaclePair(Obstacle &bottomObstacle, Obstacle &topObstacle);
	~ObstaclePair();

	bool operator==(const ObstaclePair& other) const
	{
		return id == other.id;
	}

private:
	int id;
	static int m_id;
	Obstacle bottomObstacle;
	Obstacle topObstacle;
	bool moving = false;
	bool addScore = false;
	bool sliding = false;
	float scaleFactor;
	ScalingType scalingType;
	glm::vec3 color;
	int type;

public:
	Obstacle& getBottomObstacle();
	Obstacle& getTopObstacle();
	bool isMoving();
	void setMoveState(bool move);
	void moveToRight(float speed, float deltaTimeSeconds, float maxX);
	void setAddScore(bool score);
	bool canAddScore();
	bool isSliding();
	void setSliding(bool slide);
	float calculateSlidingScaleFactor(float deltaTimeSeconds, int type);
	glm::vec3 getColor();
	void setColor(glm::vec3 color);
	void setType(int type);
	int getType();

	int getIndex()
	{
		return id;
	}
};


