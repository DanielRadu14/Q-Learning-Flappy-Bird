#pragma once

#include <Component/SimpleScene.h>
#include <string>
#include <list>
#include <Core/Engine.h>
#include "Bird.h"
#include "Obstacle.h"
#include "ObstaclePair.h"
#include <map>
#include <windows.h>
#include <mmsystem.h>

class Game : public SimpleScene
{
public:
	Game();
	~Game();

	EngineComponents::Camera *camera;

	std::unordered_map<std::string, Texture2D*> mapTextures;

	void Init() override;

	static int playerScore;
	static int botScore;
	static void increasePlayerScore();
	static void increaseBotScore();

	inline float random_float(float min, float max)
	{
		float scale = rand() / (float)RAND_MAX;
		return min + scale * (max - min);
	}

	static inline int random_int(int min, int max)
	{
		return (rand() % (max - min + 1)) + min;
	}

	static int getGameType()
	{
		return gameType;
	}

	static int getResolutionX()
	{
		return resolutionX;
	}

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;
	void Game::RenderMeshTex(Mesh* mesh, Shader* shader, const glm::mat3& modelMatrix, Texture2D* texture1);
	void Game::printScore(Bird *bird);

private:
	void initializeNewGame();
	void RenderBird();
	void RenderBirdBot();
	void RunBirdPhysics(float deltaTimeSeconds, Bird *bird);
	void DrawObstacles(Bird* bird, float deltaTimeSeconds);
	void RunCollisionPhysics(Bird *bird);
	void CalculateScore(Bird *pasare);
	Mesh* CreateBottomCornerRectangle(std::string name);
	Mesh* CreateTopCornerRectangle(std::string name);

	const int OBSTACLES_COUNT = 6;
	const int HARDER_LEVEL_SCORE = 10;
	const float OBSTACLE_SPEED = 500;

	const float MIN_TIME_INTERVAL = 0.5;
	const float MEDIUM_TIME_INTERVAL = 0.8;
	const float MAX_TIME_INTERVAL = 1.0;

	const float BIRD_VERTICAL_VELOCITY = 12.0f;
	const float GRAVITY = 4 * 9.8;

	const float OBSTACLE_SIZE_X = 50;
	const float OBSTACLE_SIZE_Y = 200;

	float birdStartX = 75;
	float birdStartY = 0;

	static int gameType;

    static int resolutionX;
	int resolutionY;

	Bird *bird;
	Bird* birdBot;
	std::list<ObstaclePair> obstaclesList; //lista obstacolelor
	std::list<ObstaclePair> obstaclesListBot; //lista obstacolelor pentru bot

	glm::mat3 modelMatrix;

	float nextRectangleMoveTime = 0.0f; //timpul dupa care un alt obstacol se va misca
	float gameTime = 0.0f; //timpul curent

	bool playerDied = false;
	bool botDied = false;
};
