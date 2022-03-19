#include "Object2D.h"

#include <Core/Engine.h>

Mesh* Object2D::CreateSquare(std::string name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill)
{
	glm::vec3 corner = leftBottomCorner;

	std::vector<VertexFormat> vertices =
	{
		VertexFormat(corner, color),
		VertexFormat(corner + glm::vec3(length, 0, 0), color),
		VertexFormat(corner + glm::vec3(length, length, 0), color),
		VertexFormat(corner + glm::vec3(0, length, 0), color)
	};

	Mesh* square = new Mesh(name);
	std::vector<unsigned short> indices = { 0, 1, 2, 3 };
	
	if (!fill) {
		square->SetDrawMode(GL_LINE_LOOP);
	}
	else {
		// draw 2 triangles. Add the remaining 2 indices
		indices.push_back(0);
		indices.push_back(2);
	}

	square->InitFromData(vertices, indices);
	return square;
}

Mesh* Object2D::CreateCircle(std::string name)
{
	float radius = 5.0;

	std::vector<VertexFormat> circleVertices;
	for (int i = 0; i <= 360; i++)
	{
		float theta = 2.0f * M_PI * float(i) / float(300);//get the current angle

		float x = radius * glm::cos(theta);//calculate the x component
		float y = radius * glm::sin(theta);//calculate the y component

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