#include <ctime>
#include <iostream>
#include <fstream>
#include <map>
#include <Core/Engine.h>
#include <Laboratoare/LabList.h>
#include <Tema1/Game.h>

using namespace std;

extern void writeMatrixFiles();
extern std::map<std::string, double> jumpMap;
extern std::vector<Mesh*> myMeshes;

int main(int argc, char **argv)
{
	srand((unsigned int)time(NULL));

	// Create a window property structure
	WindowProperties wp;
	wp.resolution = glm::ivec2(1280, 720);

	// Init the Engine and create a new window with the defined properties
	WindowObject* window = Engine::Init(wp);

	// Create a new 3D world and start running it
	World *world = new Game();
	world->Init();
	world->Run();

	writeMatrixFiles();

	for (Mesh* m : myMeshes)
	{
		//dezaloca toate meshele folosite
		m->ClearData();
		free(m);
	}

	// Signals to the Engine to release the OpenGL context
	Engine::Exit();

	return 0;
}