#pragma once
#include <iostream>

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

struct RadarConfig
{
	RadarConfig(bool alwaysCentered, float playerScale, float mapScale)
		: 
		ShowTeammates(true),
		AlwaysCentered(alwaysCentered),
		PlayerScale(playerScale),
		MapScale(mapScale),
		DefuserColor{ 0.8f, 0.8f, 0.8f, 1.0f },		/*Light gray*/
		LocalPlayerColor{0.0f, 0.0f, 1.0f, 1.0f},	/*Blue*/
		TeamPlayerColor{ 0.0f, 1.0f, 0.0f, 1.0f },	/*Green*/
		EnemyPlayerColor{ 1.0f, 0.0f, 0.0f, 1.0f }	/*Red*/
	{}

	bool ShowTeammates;
	bool AlwaysCentered;
	float PlayerScale;
	float DefuserColor[4];
	float LocalPlayerColor[4];
	float TeamPlayerColor[4];
	float EnemyPlayerColor[4];
	
	float MapScale;
};

class CSGOMap
{
public:
	CSGOMap(std::string MapName);

	std::string GetMapName() { return m_Name; }
	float GetMapOriginX() { return m_OriginX; }
	float GetMapOriginY() { return m_OriginY; }
	float GetMapScale() { return m_Scale; }

private:
	bool ParseMapInfo();

	std::string m_Name;

	float m_OriginX;
	float m_OriginY;
	float m_Scale;
	
};

class Radar
{
public:
	Radar(CSGOMap map, RadarConfig& RadarConfig);
   ~Radar();

    void Init();
	void Destroy();
	void OnRender(CSGO::EntityList& entityList, CSGO::ClientState& clientState);

	bool HasWindow();
	void ChangeMap(std::string mapName);
	
private:
	RadarConfig& m_RadarConfig;

	std::string GetMapTextureLocation(CSGOMap map);
	glm::vec3 WorldToRadar(const glm::vec3 world);

	Renderer m_Renderer;

	CSGOMap m_CurrentMap;
	Texture* m_MapTexture;
	VertexBuffer* m_MapVB;
	VertexArray* m_MapVA;
	IndexBuffer* m_MapIB;
	Shader* m_MapShader;

	VertexBuffer* m_PlayerMarkVB;
	VertexArray* m_PlayerMarkVA;
	IndexBuffer* m_PlayerMarkIB;
	Shader* m_PlayerMarkShader;

	glm::mat4 m_Proj;
	glm::mat4 m_View;

	glm::mat4 m_PlayerMarkModel;

	glm::mat4 m_MapModel;

	GLFWwindow* m_Window;
};

