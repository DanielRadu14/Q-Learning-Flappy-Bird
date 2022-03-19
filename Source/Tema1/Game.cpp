#pragma comment(lib, "Winmm.lib")

#include "Game.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <time.h>
#include <Core/Engine.h>
#include "Transform2D.h"
#include <map>
#include "Button.h"

using namespace std;

const float alpha = 0.7;
const float gamma = 0.9;
float scale = 4;
const float deathReward = -1000;
const float aliveReward = 0;

struct state
{
	double upperPipeDistance;
	double lowerPipeDistance;
	double horizontalDistance;
};

struct stateData
{
	state s;
	state s1;
	int action;
};

std::map<std::string, double> nothingMap;
std::map<std::string, double> jumpMap;
std::list<Button> buttons;
std::list<stateData> history;
std::vector<Mesh*> myMeshes;

state s, s1;
int action = 0;
int reward = 0;

bool diedHigh = false;

int Game::playerScore = 0;
int Game::botScore = 0;
int Game::gameType = 0;
int Game::resolutionX;
int difficultyLevel = 0;
float horizontalDistance;
float lowerPipeDistance;
float upperPipeDistance;
bool inSettingsMenu;
bool inDifficultyMenu;
bool trainingMode;
bool playSounds;
bool useVariableDistance;

Button *stopLearningButton;
Button* button_press1;
Button* button_press2;
Button* button_press3;
Button* diff_easy;
Button* diff_medium;
Button* diff_hard;
Button* back;


void emitSound(std::string sound)
{
	if (playSounds)
	{
		wchar_t name[256];
		swprintf(name, sizeof(name), L"%hs", sound.c_str());
		PlaySound(name, NULL, SND_FILENAME | SND_ASYNC);
	}
}

float getFromMap(std::map<std::string, double>& map, int v1, int v2, int v3)
{
	v1 = v1 / scale;
	v2 = v2 / scale;

	if (useVariableDistance)
	{
		v3 = v3 / scale;
	}
	else
	{
		v3 = 0;
	}

	char buf[256];
	sprintf(buf, "%dx%dx%d", v1, v2, v3);

	if (map.count(std::string(buf)) > 0)
	{
		return map[std::string(buf)];
	}

	return 0.0f;
}

void setInMap(std::map<std::string, double>& map, int v1, int v2, int v3, double val)
{
	v1 = v1 / scale;
	v2 = v2 / scale;

	if (useVariableDistance)
	{
		v3 = v3 / scale;
	}
	else
	{
		v3 = 0;
	}

	char buf[256];
	sprintf(buf, "%dx%dx%d", v1, v2, v3);

	map[std::string(buf)] = val;
}


void readMatrixFiles()
{
	FILE* fpJump = nullptr;
	FILE* fpNothing = nullptr;

	jumpMap.clear();
	nothingMap.clear();

	if (useVariableDistance == false)
	{
		std::cout << "reading constant distance : ";
		if (difficultyLevel == 1)
		{
			fpJump = fopen("matrixJumpEasy.txt", "rt");
			fpNothing = fopen("matrixNothingEasy.txt", "rt");
			std::cout << "easy" << endl;
		}
		else if (difficultyLevel == 2)
		{
			fpJump = fopen("matrixJumpMedium.txt", "rt");
			fpNothing = fopen("matrixNothingMedium.txt", "rt");
			std::cout << "medium" << endl;
		}
		else if (difficultyLevel == 3)
		{
			fpJump = fopen("matrixJumpHard.txt", "rt");
			fpNothing = fopen("matrixNothingHard.txt", "rt");
			std::cout << "hard" << endl;
		}
	}
	else
	{
		std::cout << "reading variable distance" << std::endl;
		fpJump = fopen("matrixJumpVariableDistance.txt", "rt");
		fpNothing = fopen("matrixNothingVariableDistance.txt", "rt");
	}

	//incarca datele din fisiere
	if (fpJump && fpNothing)
	{
		char buffer[128];
		char key[64];
		float value;

		while (fgets(buffer, 128, fpJump))
		{
			sscanf(buffer, "%s %f\n", key, &value);
			jumpMap[std::string(key)] = value;
		}

		while (fgets(buffer, 128, fpNothing))
		{
			sscanf(buffer, "%s %f\n", key, &value);
			nothingMap[std::string(key)] = value;
		}

		fclose(fpJump);
		fclose(fpNothing);
	}
}

void writeMatrixFiles()
{
	FILE* fpJump = nullptr;
	FILE* fpNothing = nullptr;

	if (useVariableDistance == 0)
	{
		std::cout << "writing constant distance : ";
		if (difficultyLevel == 1)
		{
			fpJump = fopen("matrixJumpEasy.txt", "wt");
			fpNothing = fopen("matrixNothingEasy.txt", "wt");
			std::cout << "easy" << endl;
		}
		else if (difficultyLevel == 2)
		{
			fpJump = fopen("matrixJumpMedium.txt", "wt");
			fpNothing = fopen("matrixNothingMedium.txt", "wt");
			std::cout << "medium" << endl;
		}
		else if (difficultyLevel == 3)
		{
			fpJump = fopen("matrixJumpHard.txt", "wt");
			fpNothing = fopen("matrixNothingHard.txt", "wt");
			std::cout << "hard" << endl;
		}
	}
	else
	{
		std::cout << "writing variable distance" << std::endl;
		fpJump = fopen("matrixJumpVariableDistance.txt", "wt");
		fpNothing = fopen("matrixNothingVariableDistance.txt", "wt");
	}

	if (fpJump && fpNothing)
	{
		for (auto& x : jumpMap)
		{
			fprintf(fpJump, "%s %f\n", x.first.c_str(), x.second);
		}
		fclose(fpJump);

		for (auto& x : nothingMap)
		{
			fprintf(fpNothing, "%s %f\n", x.first.c_str(), x.second);
		}
		fclose(fpNothing);
	}

}

Game::Game()
{
	bird = nullptr;
	birdBot = nullptr;
}

Game::~Game()
{
	delete bird;
	delete birdBot;
	delete stopLearningButton;
	delete button_press1;
	delete button_press2;
	delete button_press3;
	delete diff_easy;
	delete diff_medium;
	delete diff_hard;
	delete back;
}

//Construieste un dreptunghi centrat in coltul din stanga jos
Mesh* Game::CreateBottomCornerRectangle(std::string name)
{
	vector<glm::vec3> vertices
	{
		glm::vec3(0, 0,  0),
		glm::vec3(0, 2,  0),
		glm::vec3(1, 2, 0),
		glm::vec3(1, 0, 0)
	};

	vector<glm::vec3> normals
	{
		glm::vec3(0, 1, 1),
		glm::vec3(1, 0, 1),
		glm::vec3(1, 0, 0),
		glm::vec3(0, 1, 0)
	};

	vector<glm::vec2> textureCoords
	{
		glm::vec2(0.0f, 0.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(1.0f, 0.0f)
	};

	vector<unsigned short> indices =
	{
		0, 1, 3,
		1, 2, 3
	};

	Mesh* mesh = new Mesh(name);
	mesh->InitFromData(vertices, normals, textureCoords, indices);
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);

	return mesh;
}

Mesh* Game::CreateTopCornerRectangle(std::string name)
{
	vector<glm::vec3> vertices
	{
		glm::vec3(0, -2,  0),
		glm::vec3(0, 0,  0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, -2, 0)
	};

	vector<glm::vec3> normals
	{
		glm::vec3(0, 1, 1),
		glm::vec3(1, 0, 1),
		glm::vec3(1, 0, 0),
		glm::vec3(0, 1, 0)
	};

	vector<glm::vec2> textureCoords
	{
		glm::vec2(0.0f, 0.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(1.0f, 0.0f)
	};

	vector<unsigned short> indices =
	{
		0, 1, 3,
		1, 2, 3
	};

	Mesh* mesh = new Mesh(name);
	mesh->InitFromData(vertices, normals, textureCoords, indices);
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);

	return mesh;
}


void Game::Init()
{
	srand(time(NULL));

	trainingMode = false;
	playSounds = false;
	useVariableDistance = false;

	glm::ivec2 resolution = window->GetResolution();
	camera = GetSceneCamera();
	camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
	camera->SetPosition(glm::vec3(0, 0, 50));
	camera->SetRotation(glm::vec3(0, 0, 0));
	camera->Update();
	GetCameraInput()->SetActive(false);

	resolutionX = resolution.x;
	resolutionY = resolution.y;

	//pasarea trebuie sa stea la jumatatea ferestrei
	birdStartY = static_cast<float>(resolutionY) / 2;

	bird = new Bird(birdStartX, birdStartY);
	if (bird == nullptr)
	{
		return;
	}

	birdBot = new Bird(birdStartX + resolutionX / 2 + 50, birdStartY);
	if (birdBot == nullptr)
	{
		delete bird;
		return;
	}

	bird->Init();
	birdBot->Init();

	AddMeshToList(bird->body);
	AddMeshToList(bird->eye);
	AddMeshToList(bird->eye1);
	AddMeshToList(bird->pupil1);
	AddMeshToList(bird->pupil2);
	AddMeshToList(bird->beak1);
	AddMeshToList(bird->beak2);
	AddMeshToList(bird->wing1);
	AddMeshToList(bird->wing2);
	AddMeshToList(bird->wing3);
	AddMeshToList(bird->eyebrow1);
	AddMeshToList(bird->eyebrow2);

	AddMeshToList(birdBot->body);
	AddMeshToList(birdBot->eye);
	AddMeshToList(birdBot->eye1);
	AddMeshToList(birdBot->pupil1);
	AddMeshToList(birdBot->pupil2);
	AddMeshToList(birdBot->beak1);
	AddMeshToList(birdBot->beak2);
	AddMeshToList(birdBot->wing1);
	AddMeshToList(birdBot->wing2);
	AddMeshToList(birdBot->wing3);
	AddMeshToList(birdBot->eyebrow1);
	AddMeshToList(birdBot->eyebrow2);

	myMeshes.push_back(bird->body);
	myMeshes.push_back(bird->eye);
	myMeshes.push_back(bird->eye1);
	myMeshes.push_back(bird->pupil1);
	myMeshes.push_back(bird->pupil2);
	myMeshes.push_back(bird->beak1);
	myMeshes.push_back(bird->beak2);
	myMeshes.push_back(bird->wing1);
	myMeshes.push_back(bird->wing2);
	myMeshes.push_back(bird->wing3);
	myMeshes.push_back(bird->eyebrow1);
	myMeshes.push_back(bird->eyebrow2);

	myMeshes.push_back(birdBot->body);
	myMeshes.push_back(birdBot->eye);
	myMeshes.push_back(birdBot->eye1);
	myMeshes.push_back(birdBot->pupil1);
	myMeshes.push_back(birdBot->pupil2);
	myMeshes.push_back(birdBot->beak1);
	myMeshes.push_back(birdBot->beak2);
	myMeshes.push_back(birdBot->wing1);
	myMeshes.push_back(birdBot->wing2);
	myMeshes.push_back(birdBot->wing3);
	myMeshes.push_back(birdBot->eyebrow1);
	myMeshes.push_back(birdBot->eyebrow2);

	Mesh *mesh = new Mesh("gamemode");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "gamemode.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);

	mesh = new Mesh("botWon");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "botWon.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);

	mesh = new Mesh("playerWon");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "playerWon.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);

	mesh = new Mesh("draw");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "draw.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);


	mesh = new Mesh("goBack");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "goBack.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);

	//cifrele pentru afisarea scorului
	for (int i = 0; i < 10; i++)
	{
		std::string digitName = std::to_string(i);
		mesh = new Mesh(digitName);
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", digitName + ".fbx");
		meshes[mesh->GetMeshID()] = mesh;
		myMeshes.push_back(mesh);
	}

	mesh = CreateBottomCornerRectangle("bar");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);

	//Construieste dreptunghiurile(obstacolele) si le grupeaza in perechi de cate 2(sus si jos)
	for (int i = 1; i <= OBSTACLES_COUNT; i++)
	{
		std::string rectangleName = "bottom_rect_";
		rectangleName.append(std::to_string(i));

		Obstacle bottomObstacle(CreateBottomCornerRectangle(rectangleName));
		bottomObstacle.setObstacleType(BOTTOM_OBSTACLE);
		AddMeshToList(bottomObstacle.getMesh());

		rectangleName = "top_rect_";
		rectangleName.append(std::to_string(i));

		Obstacle topObstacle(CreateTopCornerRectangle(rectangleName));
		topObstacle.setObstacleType(TOP_OBSTACLE);
		AddMeshToList(topObstacle.getMesh());

		ObstaclePair obstaclePair(bottomObstacle, topObstacle);
		obstaclePair.setType(1);
		obstaclesList.push_back(obstaclePair);
	}

	for (int i = 1; i <= OBSTACLES_COUNT; i++)
	{
		std::string rectangleName = "bottom_rect_bot_";
		rectangleName.append(std::to_string(i));

		Obstacle bottomObstacle(CreateBottomCornerRectangle(rectangleName));
		bottomObstacle.setObstacleType(BOTTOM_OBSTACLE);
		AddMeshToList(bottomObstacle.getMesh());

		rectangleName = "top_rect_bot_";
		rectangleName.append(std::to_string(i));

		Obstacle topObstacle(CreateTopCornerRectangle(rectangleName));
		topObstacle.setObstacleType(TOP_OBSTACLE);
		AddMeshToList(topObstacle.getMesh());

		ObstaclePair obstaclePair(bottomObstacle, topObstacle);
		obstaclePair.setType(2);
		obstaclesListBot.push_back(obstaclePair);
	}

	{
		Shader* shader = new Shader("TextureShader");
		shader->AddShader("Source/Tema1/Shaders/TextureVertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Tema1/Shaders/TextureFragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	const string textureLoc = "Source/Tema1/Textures/";
	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((textureLoc + "pipe_down.png").c_str(), GL_REPEAT);
		mapTextures["pipe_down"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((textureLoc + "pipe_up.png").c_str(), GL_REPEAT);
		mapTextures["pipe_up"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((textureLoc + "skyline.png").c_str(), GL_REPEAT);
		mapTextures["background"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((textureLoc + "ground.png").c_str(), GL_REPEAT);
		mapTextures["ground"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((textureLoc + "brick.jpg").c_str(), GL_REPEAT);
		mapTextures["brick"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((textureLoc + "bar.png").c_str(), GL_REPEAT);
		mapTextures["bar"] = texture;
	}

	mesh = new Mesh("easy");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "easy.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	diff_easy = new Button(resolutionX / 2 - 50, 400, mesh, 0, false, false, 0);

	mesh = new Mesh("medium");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "medium.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	diff_medium = new Button(resolutionX / 2 - 50, 350, mesh, 0, false, false, 0);

	mesh = new Mesh("hard");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "hard.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	diff_hard = new Button(resolutionX / 2 - 50, 300, mesh, 0, false, false, 0);

	mesh = new Mesh("back");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "back.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	back = new Button(resolutionX / 2 - 50, 200, mesh, 0, false, false, 0);

	mesh = new Mesh("singleplayer");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "singleplayer.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	button_press1 = new Button(resolutionX / 4, 350, mesh, 0, false, false, 0);

	mesh = new Mesh("multiplayer");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "multiplayer.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	button_press2 = new Button(resolutionX * 3 / 4 - 100, 350, mesh, 0, false, false, 0);

	mesh = new Mesh("settings");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "settings.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	button_press3 = new Button(resolutionX / 2 - 50, 200, mesh, 0, false, false, 0);

	mesh = new Mesh("option_training_mode");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "option_training_mode.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	Button button(resolutionX / 2 - 100, 500, mesh, 1, true, false, 130);
	buttons.push_back(button);

	mesh = new Mesh("back");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "back.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	Button button2(resolutionX / 2 - 100, 350, mesh, 2, false, false, 0);
	buttons.push_back(button2);

	mesh = new Mesh("sounds");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sounds.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	Button button3(resolutionX / 2 - 100, 450, mesh, 3, true, false, 130);
	buttons.push_back(button3);

	mesh = new Mesh("variable_distance");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "variable_distance.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	Button button4(resolutionX / 2 - 100, 400, mesh, 4, true, false, 180);
	buttons.push_back(button4);

	mesh = new Mesh("stop_learning");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "stop_learning.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
	stopLearningButton = new Button(resolutionX / 4 - 150, resolutionY / 2, mesh, 0, false, false, 0);

	mesh = new Mesh("on");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "on.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);

	mesh = new Mesh("off");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "off.fbx");
	meshes[mesh->GetMeshID()] = mesh;
	myMeshes.push_back(mesh);
}

void Game::RenderMeshTex(Mesh* mesh, Shader* shader, const glm::mat3& modelMatrix, Texture2D* texture1)
{
	if (!mesh || !shader || !shader->program)
		return;

	// render an object using the specified shader and the specified position
	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	glm::mat3 mm = modelMatrix;
	glm::mat4 model = glm::mat4(
		mm[0][0], mm[0][1], mm[0][2], 0.f,
		mm[1][0], mm[1][1], mm[1][2], 0.f,
		0.f, 0.f, mm[2][2], 0.f,
		mm[2][0], mm[2][1], 0.f, 1.f);

	// Bind model matrix
	int loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(model));

	// Bind view matrix
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

	// Bind projection matrix
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

	if (texture1)
	{
		//heightmap texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1->GetTextureID());
		glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);
	}

	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}

void Game::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(102.0 / 255, 153.0 / 255, 255.0 / 255, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

int lastScore = -1;

void Game::printScore(Bird* pasare)
{
	int score = 0;
	int position = 0;
	if (pasare == birdBot)
	{
		score = Game::botScore;
		position = resolutionX * 3 / 4;
	}
	else if (pasare == bird && gameType == 2)
	{
		score = Game::playerScore;
		position = resolutionX / 4;
	}
	else if (pasare == bird && gameType == 1)
	{
		score = Game::playerScore;
		position = resolutionX / 2;
	}

	int offset = 0;

	if (score == 0)
	{
		modelMatrix = glm::mat3(1);
		modelMatrix *= Transform2D::Translate(position + offset, resolutionY * 4 / 5);
		modelMatrix *= Transform2D::Scale(0.8, 0.8);
		RenderMesh2D(meshes[std::to_string(0)], modelMatrix, glm::vec3(1, 1, 1));
	}
	else
	{
		while (score)
		{
			int cifra = score % 10;
			std::string s = std::to_string(cifra);

			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(position + offset, resolutionY * 4 / 5);
			modelMatrix *= Transform2D::Scale(0.8, 0.8);
			RenderMesh2D(meshes[s], modelMatrix, glm::vec3(1, 1, 1));

			offset -= 30;
			score = score / 10;
		}
	}
}

void Game::Update(float deltaTimeSeconds)
{

	if (gameType == 1)
	{
		//ceas pentru a putea executa evenimente in functie de timpul scurs
		gameTime = gameTime + 1.0f * deltaTimeSeconds;

		RenderBird(); //deseneaza pasarea
		RunBirdPhysics(deltaTimeSeconds, bird); //salt
		DrawObstacles(bird, deltaTimeSeconds); //deseneaza obstacolele
		RunCollisionPhysics(bird); //detectare coliziuni cu obstacole
		CalculateScore(bird); //calculeaza scorul curent
		printScore(bird);
	}
	else if (gameType == 2)
	{
		//ceas pentru a putea executa evenimente in functie de timpul scurs
		gameTime = gameTime + 1.0f * deltaTimeSeconds;

		if (trainingMode)
		{
			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(stopLearningButton->position.x, stopLearningButton->position.y);
			modelMatrix *= Transform2D::Scale(0.5, 0.5);
			RenderMesh2D(stopLearningButton->mesh, modelMatrix, glm::vec3(0, 0, 0));
		}
		else
		{
			if (playerDied)
			{
				printScore(bird);
			}
		}

		if (botDied)
		{
			printScore(birdBot);
		}

		if (playerDied && botDied)
		{
			if (playerScore > botScore)
			{
				modelMatrix = glm::mat3(1);
				modelMatrix *= Transform2D::Translate(470, 400);
				modelMatrix *= Transform2D::Scale(1, 1);
				RenderMesh2D(meshes["playerWon"], modelMatrix, glm::vec3(0, 0, 0));
			}
			else if (playerScore < botScore)
			{
				modelMatrix = glm::mat3(1);
				modelMatrix *= Transform2D::Translate(500, 400);
				modelMatrix *= Transform2D::Scale(1, 1);
				RenderMesh2D(meshes["botWon"], modelMatrix, glm::vec3(0, 0, 0));
			}
			else
			{
				modelMatrix = glm::mat3(1);
				modelMatrix *= Transform2D::Translate(570, 400);
				modelMatrix *= Transform2D::Scale(1, 1);
				RenderMesh2D(meshes["draw"], modelMatrix, glm::vec3(0, 0, 0));
			}

			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(65, 300);
			modelMatrix *= Transform2D::Scale(1, 1);
			RenderMesh2D(meshes["goBack"], modelMatrix, glm::vec3(0, 0, 0));

			goto drawBackground;
		}

		modelMatrix = glm::mat3(1);
		modelMatrix *= Transform2D::Translate(resolutionX / 2, 0);
		modelMatrix *= Transform2D::Scale(50, 600);
		RenderMeshTex(meshes["bar"], shaders["TextureShader"], modelMatrix, mapTextures["bar"]);

		if (!playerDied)
		{
			if (!trainingMode)
			{
				RenderBird(); //deseneaza pasarea
				RunBirdPhysics(deltaTimeSeconds, bird); //salt
				DrawObstacles(bird, deltaTimeSeconds); //deseneaza obstacolele
				RunCollisionPhysics(bird); //detectare coliziuni cu obstacole
				CalculateScore(bird); //calculeaza scorul curent
				printScore(bird);
			}
		}

		if (!botDied)
		{
			RenderBirdBot();

			s1.horizontalDistance = horizontalDistance;
			s1.upperPipeDistance = upperPipeDistance;
			s1.lowerPipeDistance = lowerPipeDistance;

			stateData currentData;
			currentData.s = s;
			currentData.s1 = s1;
			currentData.action = action;

			//adauga starea curenta in lista de stari
			history.push_front(currentData);

			s = s1;

			double jumpReward = getFromMap(jumpMap, s1.lowerPipeDistance, s1.horizontalDistance, s1.upperPipeDistance);
			double nothingReward = getFromMap(nothingMap, s1.lowerPipeDistance, s1.horizontalDistance, s1.upperPipeDistance);
			action = jumpReward > nothingReward ? 1 : 0;

			if (action == 1)
			{
				birdBot->setJump(true);
			}

			RunCollisionPhysics(birdBot); //detectare coliziuni cu obstacole

			//verifica rezultatul actiunii
			if (botDied && trainingMode)
			{
				int stateCount = 1;
				for (stateData data : history)
				{
					if (stateCount == 1 || stateCount == 2)
					{
						//in caz in care botul moare, penalizam ultimele 2 stari
						reward = deathReward;
					}
					else if (diedHigh)
					{
						//daca a murit prin coliziune cu conducta de sus inseamna ca am avut o serie
						//incorecta de jumpuri, introducem o penalizare suplimentara
						reward = deathReward;
						diedHigh = false;
					}
					else
					{
						//in urma tranzitiei din s in s1 pasarea a ramas in viata
						reward = aliveReward;
					}

					state st = data.s; //starea s
					state resSt = data.s1; //starea rezultata in urma actiunii act, s1
					int act = data.action;

					//actualizeaza Q(s, act) in functie de s1
					jumpReward = getFromMap(jumpMap, resSt.lowerPipeDistance, resSt.horizontalDistance, resSt.upperPipeDistance);
					nothingReward = getFromMap(nothingMap, resSt.lowerPipeDistance, resSt.horizontalDistance, resSt.upperPipeDistance);
					double optimalActionValue = gamma * glm::max(jumpReward, nothingReward);

					if (act == 0)
					{
						double actionValue = getFromMap(nothingMap, st.lowerPipeDistance, st.horizontalDistance, st.upperPipeDistance);
						double finalVal = (1 - alpha) * actionValue + alpha * (reward + optimalActionValue);
						setInMap(nothingMap, st.lowerPipeDistance, st.horizontalDistance, st.upperPipeDistance, finalVal);

					}
					else
					{
						double actionValue = getFromMap(jumpMap, st.lowerPipeDistance, st.horizontalDistance, st.upperPipeDistance);
						double finalVal = (1 - alpha) * actionValue + alpha * (reward + optimalActionValue);
						setInMap(jumpMap, st.lowerPipeDistance, st.horizontalDistance, st.upperPipeDistance, finalVal);

					}

					stateCount = stateCount + 1;
				}


				history.clear();
				gameType = 2;
				initializeNewGame();
			}
			else
			{
				CalculateScore(birdBot); //calculeaza scorul curent
				printScore(birdBot); //afiseaza scorul curent
				RunBirdPhysics(deltaTimeSeconds, birdBot); //salt bot
				DrawObstacles(birdBot, deltaTimeSeconds); //deseneaza obstacolele
			}
		}
	}

	if (gameType == 0)
	{
		if (inSettingsMenu)
		{
			for (Button button : buttons)
			{
				modelMatrix = glm::mat3(1);
				modelMatrix *= Transform2D::Translate(button.position.x, button.position.y);
				modelMatrix *= Transform2D::Scale(0.5, 0.5);
				RenderMesh2D(button.mesh, modelMatrix, glm::vec3(0, 0, 0));

				if (button.toggle)
				{
					modelMatrix = glm::mat3(1);
					modelMatrix *= Transform2D::Translate(resolutionX / 2 + button.deltaX, button.position.y);
					modelMatrix *= Transform2D::Scale(0.5, 0.5);

					if(button.enabled)
					{
						RenderMesh2D(meshes["on"], modelMatrix, glm::vec3(0, 0, 0));
					}
					else
					{
						RenderMesh2D(meshes["off"], modelMatrix, glm::vec3(0, 0, 0));
					}
				}
			}
		}
		else if(inDifficultyMenu)
		{
			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(diff_easy->position.x, diff_easy->position.y);
			modelMatrix *= Transform2D::Scale(0.5, 0.5);
			RenderMesh2D(diff_easy->mesh, modelMatrix, glm::vec3(0, 0, 0));

			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(diff_medium->position.x, diff_medium->position.y);
			modelMatrix *= Transform2D::Scale(0.5, 0.5);
			RenderMesh2D(diff_medium->mesh, modelMatrix, glm::vec3(0, 0, 0));

			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(diff_hard->position.x, diff_hard->position.y);
			modelMatrix *= Transform2D::Scale(0.5, 0.5);
			RenderMesh2D(diff_hard->mesh, modelMatrix, glm::vec3(0, 0, 0));

			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(back->position.x, back->position.y);
			modelMatrix *= Transform2D::Scale(0.5, 0.5);
			RenderMesh2D(back->mesh, modelMatrix, glm::vec3(0, 0, 0));
		}
		else
		{
			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(button_press1->position.x, button_press1->position.y);
			modelMatrix *= Transform2D::Scale(0.5, 0.5);
			RenderMesh2D(button_press1->mesh, modelMatrix, glm::vec3(0, 0, 0));

			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(button_press2->position.x, button_press2->position.y);
			modelMatrix *= Transform2D::Scale(0.5, 0.5);
			RenderMesh2D(button_press2->mesh, modelMatrix, glm::vec3(0, 0, 0));

			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(button_press3->position.x, button_press3->position.y);
			modelMatrix *= Transform2D::Scale(0.5, 0.5);
			RenderMesh2D(button_press3->mesh, modelMatrix, glm::vec3(0, 0, 0));

			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(450, 600);
			modelMatrix *= Transform2D::Scale(0.5, 0.5);
			RenderMesh2D(meshes["gamemode"], modelMatrix, glm::vec3(0, 0, 0));
		}
	}

drawBackground:
	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(1280 - resolutionX / 2 + 1, 820);
	modelMatrix *= Transform2D::Rotate(-M_PI);
	modelMatrix *= Transform2D::Scale(resolutionX / 2, resolutionY / 2);
	RenderMeshTex(meshes["bar"], shaders["TextureShader"], modelMatrix, mapTextures["background"]);

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(1280 - resolutionX / 2 + 1, 100);
	modelMatrix *= Transform2D::Rotate(-M_PI);
	modelMatrix *= Transform2D::Scale(resolutionX / 2, 100);
	RenderMeshTex(meshes["bar"], shaders["TextureShader"], modelMatrix, mapTextures["ground"]);

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(1280, 820);
	modelMatrix *= Transform2D::Rotate(-M_PI);
	modelMatrix *= Transform2D::Scale(resolutionX / 2, resolutionY / 2);
	RenderMeshTex(meshes["bar"], shaders["TextureShader"], modelMatrix, mapTextures["background"]);

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(1280, 100);
	modelMatrix *= Transform2D::Rotate(-M_PI);
	modelMatrix *= Transform2D::Scale(resolutionX / 2, 100);
	RenderMeshTex(meshes["bar"], shaders["TextureShader"], modelMatrix, mapTextures["ground"]);
}

void Game::FrameEnd()
{

}

void Game::OnInputUpdate(float deltaTime, int mods)
{
}

void Game::OnKeyPress(int key, int mods)
{
	if (!inSettingsMenu)
	{
		if (key == GLFW_KEY_SPACE && gameType != 0) //pasarea sare folosind tasta space
		{
			emitSound("Source/Tema1/Sounds/sfx_wing.wav");
			bird->setJump(true);
		}
	}

	if (playerDied && key == GLFW_KEY_ENTER)
	{
		gameType = 0;
	}
}

void Game::OnKeyRelease(int key, int mods)
{
}

void Game::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
}

void Game::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	glm::vec2 resolution = window->GetResolution();;
	float factorX = resolution.x / resolutionX;
	float factorY = resolution.y / resolutionY;
	mouseX = (float)mouseX / factorX;
	mouseY = (float)mouseY / factorY;
	
	if (inSettingsMenu)
	{
		for (Button& button : buttons)
		{
			if (button.type == 1 && button.trackState(mouseX, mouseY, 200, 20))
			{
				trainingMode = !trainingMode;
				button.enabled = !button.enabled;
				break;
			}
			else if (button.type == 2 && button.trackState(mouseX, mouseY, 200, 20))
			{
				inSettingsMenu = false;
				break;
			}
			else if (button.type == 3 && button.trackState(mouseX, mouseY, 200, 20))
			{
				playSounds = !playSounds;
				button.enabled = !button.enabled;
				break;
			}
			else if (button.type == 4 && button.trackState(mouseX, mouseY, 200, 20))
			{
				writeMatrixFiles();

				useVariableDistance = !useVariableDistance;
				if (useVariableDistance)
				{
					scale = 10;
				}
				else
				{
					scale = 4;
				}

				readMatrixFiles();
				std::cout << jumpMap.size() << " " << nothingMap.size() << std::endl;

				button.enabled = !button.enabled;
				break;
			}
		}
	}
	else if (inDifficultyMenu)
	{
		if (back->trackState(mouseX, mouseY, 200, 20))
		{
			gameType = 0;
			inDifficultyMenu = false;
		}
		else if (diff_easy->trackState(mouseX, mouseY, 200, 20))
		{
			if (gameType == 0)
			{
				gameType = 2;
				difficultyLevel = 1;
				inDifficultyMenu = false;
				readMatrixFiles();
				initializeNewGame();
			}
		}
		else if (diff_medium->trackState(mouseX, mouseY, 200, 20))
		{
			if (gameType == 0)
			{
				gameType = 2;
				difficultyLevel = 2;
				inDifficultyMenu = false;
				readMatrixFiles();
				initializeNewGame();
			}
		}
		else if (diff_hard->trackState(mouseX, mouseY, 200, 20))
		{
			if (gameType == 0)
			{
				gameType = 2;
				difficultyLevel = 3;
				inDifficultyMenu = false;
				readMatrixFiles();
				initializeNewGame();
			}
		}
	}
	else
	{
		if (trainingMode && stopLearningButton->trackState(mouseX, mouseY, 200, 20))
		{
			gameType = 0;
			writeMatrixFiles();
		}
		else if (button_press1->trackState(mouseX, mouseY, 200, 20))
		{
			if (gameType == 0)
			{
				gameType = 1;
				initializeNewGame();
			}
		}
		else if (button_press2->trackState(mouseX, mouseY, 200, 20))
		{
			if (gameType == 0)
			{
				if (!useVariableDistance)
				{
					inDifficultyMenu = true;
				}
				else
				{
					gameType = 2;
					difficultyLevel = 0;
					readMatrixFiles();
					initializeNewGame();
				}
			}
		}
		else if (button_press3->trackState(mouseX, mouseY, 200, 20))
		{
			if (gameType == 0)
			{
				inSettingsMenu = true;
			}
		}
	}
}

void Game::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Game::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Game::OnWindowResize(int width, int height)
{
}


void Game::initializeNewGame()
{
	horizontalDistance = 999;
	lowerPipeDistance = 0;
	upperPipeDistance = 0;

	diedHigh = false;

	action = 0;
	reward = aliveReward;

	s.horizontalDistance = 0;
	s.lowerPipeDistance = 0;
	s.upperPipeDistance = 0;

	s1.horizontalDistance = 0;
	s1.lowerPipeDistance = 0;
	s1.upperPipeDistance = 0;

	history.clear();

	//seteaza pozitiile de inceput ale obstacolelor(in coltul de sus/jos dreapta)
	for (auto& obstaclePair : obstaclesList)
	{
		if (gameType == 1)
		{
			obstaclePair.getBottomObstacle().setPositionX(resolutionX);
			obstaclePair.getBottomObstacle().setPositionY(0);

			obstaclePair.getTopObstacle().setPositionX(resolutionX);
			obstaclePair.getTopObstacle().setPositionY(resolutionY);
		}
		else if (gameType == 2)
		{
			obstaclePair.getBottomObstacle().setPositionX(resolutionX / 2);
			obstaclePair.getBottomObstacle().setPositionY(0);

			obstaclePair.getTopObstacle().setPositionX(resolutionX / 2);
			obstaclePair.getTopObstacle().setPositionY(resolutionY);
		}

		//initial obstacolele nu se misca si nu isi schimba dimensiunile
		obstaclePair.setMoveState(false);
		obstaclePair.setSliding(false);
	}

	for (auto& obstaclePair : obstaclesListBot)
	{
		if (gameType == 2)
		{
			obstaclePair.getBottomObstacle().setPositionX(resolutionX);
			obstaclePair.getBottomObstacle().setPositionY(0);

			obstaclePair.getTopObstacle().setPositionX(resolutionX);
			obstaclePair.getTopObstacle().setPositionY(resolutionY);
		}
		obstaclePair.setMoveState(false);
		obstaclePair.setSliding(false);
	}

	bird->setAngle(0.0f);
	bird->setVelocity(0.0f);
	bird->setY(birdStartY);

	birdBot->setAngle(0.0f);
	birdBot->setVelocity(0.0f);
	birdBot->setY(birdStartY);

	playerScore = 0;
	botScore = 0;

	gameTime = 0.0f;

	bird->nextRectangleMoveTime = 0.0f;
	bird->anteriorObstacleHeight1 = -1;
	bird->anteriorObstacleHeight2 = -1;
	bird->anteriorTimeInterval = 0;

	birdBot->nextRectangleMoveTime = 0.0f;
	birdBot->anteriorObstacleHeight1 = -1;
	birdBot->anteriorObstacleHeight2 = -1;
	birdBot->anteriorTimeInterval = 0;

	playerDied = 0;
	botDied = 0;

}

void Game::increasePlayerScore()
{
	playerScore++;
}
void Game::increaseBotScore()
{
	botScore++;
}

void Game::RenderBird()
{
	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getEyeBrow1X(), bird->getEyeBrow1Y());
	modelMatrix *= Transform2D::Scale(20, 15);
	RenderMesh2D(bird->eyebrow1, modelMatrix, glm::vec3(77.0 / 255, 0, 0));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getEyeBrow2X(), bird->getEyeBrow2Y());
	modelMatrix *= Transform2D::Scale(20, 15);
	RenderMesh2D(bird->eyebrow2, modelMatrix, glm::vec3(77.0 / 255, 0, 0));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getBeak1X(), bird->getBeak1Y());
	modelMatrix *= Transform2D::Scale(20, 10);
	modelMatrix *= Transform2D::Rotate(-M_PI / 6);
	RenderMesh2D(bird->beak1, modelMatrix, glm::vec3(255.0 / 255, 140.0 / 255, 26.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getBeak2X(), bird->getBeak2Y());
	modelMatrix *= Transform2D::Scale(18, 5);
	modelMatrix *= Transform2D::Rotate(-RADIANS(93));
	RenderMesh2D(bird->beak2, modelMatrix, glm::vec3(255.0 / 255, 140.0 / 255, 26.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getPupil1X(), bird->getPupil1Y());
	modelMatrix *= Transform2D::Scale(0.5, 0.5);
	RenderMesh2D(bird->pupil1, modelMatrix, glm::vec3(0, 0, 0));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getPupil2X(), bird->getPupil2Y());
	modelMatrix *= Transform2D::Scale(0.5, 0.5);
	RenderMesh2D(bird->pupil2, modelMatrix, glm::vec3(0, 0, 0));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getEye1X(), bird->getEye1Y());
	modelMatrix *= Transform2D::Scale(2, 2);
	RenderMesh2D(bird->eye, modelMatrix, glm::vec3(1, 1, 1));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getEye2X(), bird->getEye2Y());
	modelMatrix *= Transform2D::Scale(2, 2);
	RenderMesh2D(bird->eye1, modelMatrix, glm::vec3(1, 1, 1));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Scale(10, 10);
	RenderMesh2D(bird->body, modelMatrix, glm::vec3(128.0 / 255, 0 / 255, 0.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getWing1X(), bird->getWing1Y());
	modelMatrix *= Transform2D::Scale(30, 20);
	modelMatrix *= Transform2D::Rotate(-RADIANS(bird->getWingsAngle()));
	RenderMesh2D(bird->wing1, modelMatrix, glm::vec3(128.0 / 255, 0 / 255, 0.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getWing2X(), bird->getWing2Y());
	modelMatrix *= Transform2D::Scale(30, 20);
	modelMatrix *= Transform2D::Rotate(-RADIANS(bird->getWingsAngle()));
	RenderMesh2D(bird->wing2, modelMatrix, glm::vec3(128.0 / 255, 0 / 255, 0.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getWing3X(), bird->getWing3Y());
	modelMatrix *= Transform2D::Scale(30, 20);
	modelMatrix *= Transform2D::Rotate(-RADIANS(bird->getWingsAngle()));
	RenderMesh2D(bird->wing3, modelMatrix, glm::vec3(128.0 / 255, 0 / 255, 0.0 / 255));
}

void Game::RenderBirdBot()
{
	Bird* bird = birdBot;
	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getEyeBrow1X(), bird->getEyeBrow1Y());
	modelMatrix *= Transform2D::Scale(20, 15);
	RenderMesh2D(bird->eyebrow1, modelMatrix, glm::vec3(77.0 / 255, 0, 0));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getEyeBrow2X(), bird->getEyeBrow2Y());
	modelMatrix *= Transform2D::Scale(20, 15);
	RenderMesh2D(bird->eyebrow2, modelMatrix, glm::vec3(77.0 / 255, 0, 0));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getBeak1X(), bird->getBeak1Y());
	modelMatrix *= Transform2D::Scale(20, 10);
	modelMatrix *= Transform2D::Rotate(-M_PI / 6);
	RenderMesh2D(bird->beak1, modelMatrix, glm::vec3(255.0 / 255, 140.0 / 255, 26.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getBeak2X(), bird->getBeak2Y());
	modelMatrix *= Transform2D::Scale(18, 5);
	modelMatrix *= Transform2D::Rotate(-RADIANS(93));
	RenderMesh2D(bird->beak2, modelMatrix, glm::vec3(255.0 / 255, 140.0 / 255, 26.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getPupil1X(), bird->getPupil1Y());
	modelMatrix *= Transform2D::Scale(0.5, 0.5);
	RenderMesh2D(bird->pupil1, modelMatrix, glm::vec3(0, 0, 0));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getPupil2X(), bird->getPupil2Y());
	modelMatrix *= Transform2D::Scale(0.5, 0.5);
	RenderMesh2D(bird->pupil2, modelMatrix, glm::vec3(0, 0, 0));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getEye1X(), bird->getEye1Y());
	modelMatrix *= Transform2D::Scale(2, 2);
	RenderMesh2D(bird->eye, modelMatrix, glm::vec3(1, 1, 1));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getEye2X(), bird->getEye2Y());
	modelMatrix *= Transform2D::Scale(2, 2);
	RenderMesh2D(bird->eye1, modelMatrix, glm::vec3(1, 1, 1));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Scale(10, 10);
	RenderMesh2D(bird->body, modelMatrix, glm::vec3(128.0 / 255, 0 / 255, 0.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getWing1X(), bird->getWing1Y());
	modelMatrix *= Transform2D::Scale(30, 20);
	modelMatrix *= Transform2D::Rotate(-RADIANS(bird->getWingsAngle()));
	RenderMesh2D(bird->wing1, modelMatrix, glm::vec3(128.0 / 255, 0 / 255, 0.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getWing2X(), bird->getWing2Y());
	modelMatrix *= Transform2D::Scale(30, 20);
	modelMatrix *= Transform2D::Rotate(-RADIANS(bird->getWingsAngle()));
	RenderMesh2D(bird->wing2, modelMatrix, glm::vec3(128.0 / 255, 0 / 255, 0.0 / 255));

	modelMatrix = glm::mat3(1);
	modelMatrix *= Transform2D::Translate(bird->getBodyX(), bird->getBodyY());
	modelMatrix *= Transform2D::Rotate(bird->getAngle());
	modelMatrix *= Transform2D::Translate(-bird->getBodyX(), -bird->getBodyY());
	modelMatrix *= Transform2D::Translate(bird->getWing3X(), bird->getWing3Y());
	modelMatrix *= Transform2D::Scale(30, 20);
	modelMatrix *= Transform2D::Rotate(-RADIANS(bird->getWingsAngle()));
	RenderMesh2D(bird->wing3, modelMatrix, glm::vec3(128.0 / 255, 0 / 255, 0.0 / 255));
}

void Game::RunBirdPhysics(float deltaTimeSeconds, Bird* bird)
{
	if (bird->getVelocity() < 0)
	{
		//inclina pasarea in jos
		bird->setAngle(bird->DESCENDING_ANGLE);
	}
	else if (bird->getVelocity() > 0)
	{
		//inclina pasarea in sus
		bird->setAngle(bird->ASCENDING_ANGLE);
	}

	//determina modul de miscare al aripilor(rotatii int intervalul 150 - 205)
	if (bird->getWingsMovementType())
	{
		if (bird->getWingsAngle() <= 205)
		{
			bird->setWingsAngle(bird->getWingsAngle() + 100.0 * deltaTimeSeconds);
		}
		else
		{
			bird->setWingsMovementType(false);
		}
	}
	else
	{
		if (bird->getWingsAngle() >= 150)
		{
			bird->setWingsAngle(bird->getWingsAngle() - 100.0 * deltaTimeSeconds);
		}
		else
		{
			bird->setWingsMovementType(true);
		}
	}

	if (bird->shouldJump())
	{
		//adauga viteza pe Oy
		bird->setVelocity(BIRD_VERTICAL_VELOCITY);
		bird->setJump(false);
	}

	//aplica gravitatie
	bird->setVelocity(bird->getVelocity() - GRAVITY * deltaTimeSeconds);

	//calculeaza noua pozitie
	float newBirdPos = bird->getBodyY() + bird->getVelocity();

	if (newBirdPos <= 0)
	{
		//previne iesirea din ecran
		newBirdPos = 0;
	}

	if (newBirdPos >= resolutionY)
	{
		//previne iesirea din ecran
		newBirdPos = static_cast<float>(resolutionY);
	}

	bird->setY(newBirdPos);
}


void Game::DrawObstacles(Bird* birdPtr, float deltaTimeSeconds)
{
	//calculeaza intervalul de timp dupa care va porni urmatorul dreptunghi
	auto calculateTimeInterval = [this]()
	{
		if (useVariableDistance)
			return 0.95f;
		else
			return MEDIUM_TIME_INTERVAL;
	};

	//calculeaza dinamic inaltimea dreptunghiurilor
	auto setObstacleHeightVariable = [this, birdPtr](ObstaclePair& obstaclePair)
	{
		Obstacle& bottomObstacle = obstaclePair.getBottomObstacle();
		Obstacle& topObstacle = obstaclePair.getTopObstacle();

		int height1 = random_int(OBSTACLE_SIZE_Y - 150, OBSTACLE_SIZE_Y + 300);
		int height2 = resolutionY - height1 - random_int(200, 280);

		if (height1 < 50)
		{
			height1 = 50;
		}

		if (height2 < 50)
		{
			height2 = 50;
		}

		if (abs(height2 - birdPtr->anteriorObstacleHeight2) >= OBSTACLE_SIZE_Y && birdPtr->anteriorTimeInterval <= MEDIUM_TIME_INTERVAL + 0.2)
		{
			//previne cazuri imposibile(de exemplu cand pasarea este foarte sus si urmatorul obstacol are deschiderea in partea de jos a ecranului
			if (birdPtr->anteriorObstacleHeight2 < birdPtr->anteriorObstacleHeight1)
			{
				height2 = 150;
				height1 = 300;
			}
			else
			{
				height2 = 300;
				height1 = 150;
			}
		}

		bottomObstacle.setHeight(height1);
		bottomObstacle.setWidth(OBSTACLE_SIZE_X);
		topObstacle.setHeight(height2);
		topObstacle.setWidth(OBSTACLE_SIZE_X);

		birdPtr->anteriorObstacleHeight1 = height1;
		birdPtr->anteriorObstacleHeight2 = height2;
	};

	auto setObstacleHeight = [this, birdPtr](ObstaclePair& obstaclePair)
	{
		Obstacle& bottomObstacle = obstaclePair.getBottomObstacle();
		Obstacle& topObstacle = obstaclePair.getTopObstacle();

		int height1 = random_int(OBSTACLE_SIZE_Y - 150, OBSTACLE_SIZE_Y + 180);
		int height2 = resolutionY - height1 - 290;

		if (abs(height2 - birdPtr->anteriorObstacleHeight2) >= OBSTACLE_SIZE_Y && birdPtr->anteriorTimeInterval <= MEDIUM_TIME_INTERVAL + 0.2)
		{
			//previne cazuri imposibile(de exemplu cand pasarea este foarte sus si urmatorul obstacol are deschiderea in partea de jos a ecranului
			if (birdPtr->anteriorObstacleHeight2 < birdPtr->anteriorObstacleHeight1)
			{
				height2 = 130;
				height1 = 300;
			}
			else
			{
				height2 = 300;
				height1 = 130;
			}
		}

		bottomObstacle.setHeight(height1);
		bottomObstacle.setWidth(OBSTACLE_SIZE_X);
		topObstacle.setHeight(height2);
		topObstacle.setWidth(OBSTACLE_SIZE_X);

		birdPtr->anteriorObstacleHeight1 = height1;
		birdPtr->anteriorObstacleHeight2 = height2;
	};


	//incepe miscarea urmatorului obstacol
	auto prepareObstacleForMoving = [this, calculateTimeInterval, setObstacleHeight, setObstacleHeightVariable, birdPtr](ObstaclePair& obstaclePair)
	{
		float randomInterval = calculateTimeInterval();

		if (useVariableDistance)
		{
			setObstacleHeightVariable(obstaclePair);
		}
		else
		{
			setObstacleHeight(obstaclePair);
		}

		obstaclePair.setMoveState(true);

		birdPtr->nextRectangleMoveTime = gameTime + randomInterval;
		birdPtr->anteriorTimeInterval = randomInterval;
	};


	if (birdPtr == birdBot)
	{
		horizontalDistance = 999.9;
	}

	//actualizeaza pozitia obstacolului si calculeaza distanta orizontala pana la primul obstacol
	auto moveObstacle = [this, deltaTimeSeconds, prepareObstacleForMoving, birdPtr](ObstaclePair& obstaclePair)
	{
		if (birdPtr->nextRectangleMoveTime <= gameTime)
		{
			if (obstaclePair.isMoving() == false)
			{
				prepareObstacleForMoving(obstaclePair);
			}
		}

		if (obstaclePair.isMoving())
		{
			if (gameType == 1)
			{
				obstaclePair.moveToRight(OBSTACLE_SPEED, deltaTimeSeconds, resolutionX);
			}
			else if (gameType == 2)
			{
				if (birdPtr == bird)
				{
					obstaclePair.moveToRight(OBSTACLE_SPEED, deltaTimeSeconds, resolutionX / 2);
				}
				else
				{
					obstaclePair.moveToRight(OBSTACLE_SPEED, deltaTimeSeconds, resolutionX);
				}
			}

			if (birdPtr == birdBot)
			{
				float x = obstaclePair.getBottomObstacle().getPositionX() + obstaclePair.getBottomObstacle().getWidth();
				if (x >= birdBot->getBodyX() - 50)
				{
					float distance = x - (birdBot->getBodyX() - 50);
					if (distance < horizontalDistance)
					{
						horizontalDistance = distance;
					}
				}
			}
		}
	};

	if (birdPtr == bird)
	{
		std::for_each(obstaclesList.begin(), obstaclesList.end(), moveObstacle);
	}
	if(birdPtr == birdBot)
	{
		std::for_each(obstaclesListBot.begin(), obstaclesListBot.end(), moveObstacle);

		if (trainingMode)
		{
			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(birdBot->getBodyX() - 50, birdBot->getBodyY() - 50);
			modelMatrix *= Transform2D::Scale(horizontalDistance, 2);
			RenderMesh2D(meshes["bar"], modelMatrix, glm::vec3(1, 0, 0));
		}

		for (ObstaclePair obs : obstaclesListBot)
		{
			float x = obs.getBottomObstacle().getPositionX() + obs.getBottomObstacle().getWidth();
			float distance = x - (birdBot->getBodyX() - 50);
			if (distance == horizontalDistance)
			{
				//calculeaza distantele pe verticala pana la cele 2 conducte
				lowerPipeDistance = birdBot->getBodyY() - 50 - obs.getBottomObstacle().getPositionY() - obs.getBottomObstacle().getHeight();
				upperPipeDistance = resolutionY - obs.getTopObstacle().getHeight() - birdBot->getBodyY() + 50;
				break;
			}
		}

		if (trainingMode)
		{
			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(birdBot->getBodyX() - 50, birdBot->getBodyY() - 50);
			modelMatrix *= Transform2D::Scale(2, -lowerPipeDistance / 2);
			RenderMesh2D(meshes["bar"], modelMatrix, glm::vec3(0, 1, 0));

			modelMatrix = glm::mat3(1);
			modelMatrix *= Transform2D::Translate(birdBot->getBodyX() - 50, birdBot->getBodyY() - 50);
			modelMatrix *= Transform2D::Scale(2, upperPipeDistance / 2);
			RenderMesh2D(meshes["bar"], modelMatrix, glm::vec3(0, 0, 1));
		}
	}

	auto drawObstacle = [this, deltaTimeSeconds](ObstaclePair& obstaclePair)
	{
		Obstacle& bottomObstacle = obstaclePair.getBottomObstacle();
		Obstacle& topObstacle = obstaclePair.getTopObstacle();

		glm::mat3 rectM = glm::mat3(1);

		float heightScaleFactor1;
		float heightScaleFactor2;

		if (obstaclePair.isSliding())
		{
			//obstacolele se apropie si se departeaza
			float scaleFactor = obstaclePair.calculateSlidingScaleFactor(deltaTimeSeconds, 2);
			heightScaleFactor1 = scaleFactor;
			heightScaleFactor2 = scaleFactor;

			obstaclePair.getBottomObstacle().setHeight(obstaclePair.getBottomObstacle().getHeightForScaleFactor(scaleFactor));
			obstaclePair.getTopObstacle().setHeight(obstaclePair.getBottomObstacle().getHeightForScaleFactor(scaleFactor));
		}
		else
		{
			//obstacolele raman la dimensiunile setate
			heightScaleFactor1 = bottomObstacle.getScaleFactorForHeight(bottomObstacle.getHeight());
			heightScaleFactor2 = topObstacle.getScaleFactorForHeight(topObstacle.getHeight());
		}

		rectM *= Transform2D::Translate(bottomObstacle.getPositionX(), bottomObstacle.getPositionY());
		rectM *= Transform2D::Scale(bottomObstacle.getScaleFactorForWidth(bottomObstacle.getWidth()), heightScaleFactor1);
		RenderMeshTex(bottomObstacle.getMesh(), shaders["TextureShader"], rectM, mapTextures["pipe_down"]);

		rectM = glm::mat3(1);
		rectM *= Transform2D::Translate(topObstacle.getPositionX(), topObstacle.getPositionY());
		rectM *= Transform2D::Scale(topObstacle.getScaleFactorForWidth(topObstacle.getWidth()), heightScaleFactor2);
		RenderMeshTex(topObstacle.getMesh(), shaders["TextureShader"], rectM, mapTextures["pipe_up"]);
	};

	if (birdPtr == bird)
	{
		std::for_each(obstaclesList.begin(), obstaclesList.end(), drawObstacle);
	}
	else
	{
		std::for_each(obstaclesListBot.begin(), obstaclesListBot.end(), drawObstacle);
	}
}

//metoda folosita este cea din tutorialele puse in enuntul temei
void Game::RunCollisionPhysics(Bird* bird)
{
	//verifica daca exista coliziune cu un anumit obstacol
	auto checkCollisionWitObject = [this, bird](Obstacle& obstacle)
	{
		float radius = 5 * 10;

		glm::vec2 center(bird->getBodyX(), bird->getBodyY()); //centrul pasarii
		glm::vec2 aabb_half_extents(obstacle.getWidth() / 2, obstacle.getHeight() / 2); //jumatatile dimensiunilor dreptunghiului
		glm::vec2 aabb_center;

		//calculeaza centrul obstacolelor in functie de tipul lor
		if (obstacle.getObstacleType() == TOP_OBSTACLE)
		{
			aabb_center.x = obstacle.getPositionX() + aabb_half_extents.x;
			aabb_center.y = obstacle.getPositionY() - aabb_half_extents.y;
		}
		else
		{
			aabb_center.x = obstacle.getPositionX() + aabb_half_extents.x;
			aabb_center.y = obstacle.getPositionY() + aabb_half_extents.y;
		}

		glm::vec2 difference = center - aabb_center;
		glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

		glm::vec2 closest = aabb_center + clamped;
		difference = closest - center;

		float length = glm::length(difference); //obtine dimensiunea vectorului diferenta
		if (length < radius)
		{
			return true;
		}

		return false;
	};

	//verifica daca exista coliziune cu cel putin un obstacol
	auto checkCollision = [this, bird, checkCollisionWitObject](ObstaclePair& obstaclePair, int type)
	{
		Obstacle& bottomObstacle = obstaclePair.getBottomObstacle();
		Obstacle& topObstacle = obstaclePair.getTopObstacle();

		if (checkCollisionWitObject(bottomObstacle) || checkCollisionWitObject(topObstacle))
		{
			return true;
		}

		if (bird->getBodyY() <= 0.0)
		{
			if (type == 1 && bird->getBodyX() < resolutionX / 2)
			{
				return true;
			}

			if (type == 2 && bird->getBodyX() > resolutionX / 2)
			{
				return true;
			}
		}
		return false;
	};

	for (ObstaclePair& obstaclePair : obstaclesList)
	{
		if (checkCollision(obstaclePair, 1))
		{
			playerDied = true;
			emitSound("Source/Tema1/Sounds/sfx_hit.wav");
			break;
		}
	}

	if (gameType == 1 && playerDied)
	{
		gameType = 0;
		return;
	}

	if (gameType == 2)
	{
		for (ObstaclePair& obstaclePair : obstaclesListBot)
		{
			if (checkCollision(obstaclePair, 2))
			{
				botDied = true;
				std::cout << botScore << std::endl;
				if (botScore == 0) initializeNewGame();//restart joc ptr scor 0
				emitSound("Source/Tema1/Sounds/sfx_hit.wav");
				break;
			}
		}
	}
}

void Game::CalculateScore(Bird* pasare)
{
	//calculeaza scorul curent
	auto calculateScore = [this, pasare](ObstaclePair& obstaclePair)
	{
		if (gameType == 0)
		{
			return;
		}

		//daca nu s-a adaugat deja scor pentru acest obstacol
		if (obstaclePair.canAddScore())
		{
			//daca obstcolule ste inca pe ecran
			int x = obstaclePair.getBottomObstacle().getPositionX();
			if (!(x <= -obstaclePair.getBottomObstacle().getWidth()))
			{
				//daca pasarea a trecut de el
				if (x < pasare->getBodyX())
				{
					//adauga +1 scor
					if (pasare == bird)
					{
						Game::increasePlayerScore();
						emitSound("Source/Tema1/Sounds/sfx_point.wav");
					}
					else if (pasare == birdBot)
					{
						Game::increaseBotScore();

						if (playerDied)
						{
							emitSound("Source/Tema1/Sounds/sfx_point.wav");
						}
					}
					obstaclePair.setAddScore(false);
				}
			}
		}
	};

	if (pasare == bird)
	{
		std::for_each(obstaclesList.begin(), obstaclesList.end(), calculateScore);
	}
	else if (pasare == birdBot)
	{
		std::for_each(obstaclesListBot.begin(), obstaclesListBot.end(), calculateScore);
	}
}
