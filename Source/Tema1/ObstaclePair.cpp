#include "ObstaclePair.h"
#include "Game.h"
#include <iostream>

int ObstaclePair::m_id = 0;

//o pereche este un obstacol aflat jos si unul aflat sus
ObstaclePair::ObstaclePair(Obstacle& bottomObstacle, Obstacle& topObstacle) : bottomObstacle(bottomObstacle), topObstacle(topObstacle)
{
	id = m_id;
	std::cout << "Creating pair with id " << id << std::endl;
	m_id++;
	addScore = true;
	color.r = 0;
	color.g = 1;
	color.b = 0;
	type = 0;
}

ObstaclePair::~ObstaclePair()
{
}

int ObstaclePair::getType()
{
	return type;
}

void ObstaclePair::setType(int type)
{
	this->type = type;
}

Obstacle& ObstaclePair::getBottomObstacle()
{
	return this->bottomObstacle;
}

Obstacle& ObstaclePair::getTopObstacle()
{
	return this->topObstacle;
}

bool ObstaclePair::isMoving()
{
	return this->moving;
}

void ObstaclePair::setMoveState(bool move)
{
	this->moving = move;
}

void ObstaclePair::moveToRight(float speed, float deltaTimeSeconds, float maxX)
{
	float newPosition = bottomObstacle.getPositionX() - speed * deltaTimeSeconds;

	if (newPosition <= -bottomObstacle.getWidth()) //daca a iesit de pe ecran
	{
		//muta dreptunghiul in dreapta pentru a putea intra iar pe ecran
		newPosition = maxX;
		moving = false;
		addScore = true;
		this->setSliding(false);
		this->setColor({ 0, 1, 0 });
	}

	if (Game::getGameType() == 2 && type == 2)
	{
		if (newPosition <= Game::getResolutionX()/2) //daca a iesit de pe ecran
		{
			//muta dreptunghiul in dreapta pentru a putea intra iar pe ecran
			newPosition = maxX;
			moving = false;
			addScore = true;
			this->setSliding(false);
			this->setColor({ 0, 1, 0 });
		}
	}

	bottomObstacle.setPositionX(newPosition);
	topObstacle.setPositionX(newPosition);
}

void ObstaclePair::setAddScore(bool score)
{
	this->addScore = score;
}

bool ObstaclePair::canAddScore()
{
	return addScore;
}

bool ObstaclePair::isSliding()
{
	return sliding;
}

void ObstaclePair::setSliding(bool slide)
{
	//creeaza animatia de "usi glisante" pt obstacole
	scaleFactor = 0.0f;
	this->sliding = slide;
	scalingType = OBSTACLE_GROWING;
}

float ObstaclePair::calculateSlidingScaleFactor(float deltaTimeSeconds, int type)
{
	float scalingSpeed;

	int score;
	if (type == 1)
	{
		score = Game::playerScore;
	}
	else if (type == 2)
	{
		score = Game::botScore;
	}


	//obstacolele se vor apropia mai repede de la un anumit scor
	if (score <= 20)
	{
		scalingSpeed = 120;
	}
	else
	{
		scalingSpeed = 150;
	}

	if (scalingType == OBSTACLE_GROWING)
	{
		//obstacolele se apropie
		scaleFactor = scaleFactor + scalingSpeed * deltaTimeSeconds;
		if (scaleFactor >= Game::random_int(100, 180))
		{
			scalingType = OBSTACLE_SHRINKING;
		}
	}
	else
	{
		//obstacolele se departeaza
		scaleFactor = scaleFactor - scalingSpeed * deltaTimeSeconds;
		if (scaleFactor <= 1)
		{
			scalingType = OBSTACLE_GROWING;
		}
	}

	return scaleFactor;
}

glm::vec3 ObstaclePair::getColor()
{
	return color;
}

void ObstaclePair::setColor(glm::vec3 color)
{
	this->color.x = color.x;
	this->color.y = color.y;
	this->color.z = color.z;
}
