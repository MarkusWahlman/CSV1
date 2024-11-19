#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "RenderAPI/VertexBuffer.h"
#include "RenderAPI/VertexBufferLayout.h"
#include "RenderAPI/IndexBuffer.h"
#include "RenderAPI/VertexArray.h"
#include "RenderAPI/Shader.h"
#include "RenderAPI/Texture.h"
#include "RenderAPI/Renderer.h"

#include "Process/Struct/CSGO.h"

struct ESPConfig
{
	ESPConfig()
		:
		ShowTeammates(true),
		DrawBoxes(true),
		DrawHP(true),
		DrawBones(true),
		DefuserBoxColor{ 0.8f, 0.8f, 0.8f, 1.0f },		/*Light Gray*/
		TeamPlayerBoxColor{ 0.0f, 1.0f, 0.0f, 1.0f },	/*Green*/
		EnemyPlayerBoxColor{ 1.0f, 0.0f, 0.0f, 1.0f }	/*Red*/
	{}

	bool ShowTeammates;
	bool DrawBoxes;
	bool DrawHP;
	bool DrawBones;

	float DefuserBoxColor[4];
	float TeamPlayerBoxColor[4];
	float EnemyPlayerBoxColor[4];
};

class ESP
{
public:
	enum class CSGOMap
	{
		Mirage = 0,
		Dust,
		Cache,
		Vertigo
	};

	ESP(ESPConfig& ESPConfig);
	~ESP();

	void Init();
	void Destroy();
	void OnRender(CSGO::EntityList& entityList, CSGO::ClientState& clientState);

	bool HasWindow();

private:
	ESPConfig& m_ESPConfig;

	Renderer m_Renderer;

	glm::mat4 m_PlayerBoxModel;
	VertexBuffer* m_PlayerBoxVB;
	VertexArray* m_PlayerBoxVA;
	IndexBuffer* m_PlayerBoxIB;

	glm::mat4 m_HPBoxModel;
	VertexBuffer* m_HPBoxVB;
	VertexArray* m_HPBoxVA;
	IndexBuffer* m_HPBoxIB;

	std::vector<int> m_BonesToBeDrawn;
	glm::mat4 m_BonesModel;
	VertexBuffer* m_BonesVB;
	VertexArray* m_BonesVA;
	IndexBuffer* m_BonesIB;
	glm::mat4 m_BoneOrthoProj;

	Shader* m_Shader;
	glm::mat4 m_Proj;
	glm::mat4 m_View;

	GLFWwindow* m_Window;
	int m_WindowX;
	int m_WindowY;
};

