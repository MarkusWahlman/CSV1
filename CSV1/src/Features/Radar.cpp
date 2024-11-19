#include "Radar.h"
#include "GL/glew.h"

#include <stdexcept>
#include <vector>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "../res/Shaders/Shaders.h"

static void GLAPIENTRY GLDebugCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    std::cerr <<
        "[OpenGL Message from Radar.cpp]:" << '\n' <<
        "Message: " << message << '\n' <<
        "Type: " << type << '\n' <<
        "Severity: " << severity << '\n' <<
        "Id: " << id << '\n' << '\n';
}

Radar::Radar(CSGOMap map, RadarConfig& RadarConfig)
    :
    m_RadarConfig(RadarConfig),

    m_CurrentMap(map),
    m_MapTexture(nullptr),
    m_MapVB(nullptr),
    m_MapVA(nullptr),
    m_MapIB(nullptr),
    m_MapShader(nullptr),

    m_PlayerMarkVB(nullptr),
    m_PlayerMarkVA(nullptr),
    m_PlayerMarkIB(nullptr),
    m_PlayerMarkShader(nullptr),

    m_Proj({}),
    m_View({}),

    m_MapModel({}),

    m_PlayerMarkModel({}),
    
    m_Window(nullptr)
{

}

Radar::~Radar()
{
    
}

static bool RadarWindowMouseHeld = false;
static void RadarWindowMouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        RadarWindowMouseHeld = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        RadarWindowMouseHeld = false;
    }
}

void Radar::Init()
{
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    glDebugMessageCallback(GLDebugCallback, nullptr);
    glEnable(GL_DEBUG_OUTPUT);

    m_Window = glfwCreateWindow(300, 300, "", nullptr, nullptr);
    if (!m_Window)
    {
        glfwTerminate();
        throw std::runtime_error("couldn't create radar window");
    }
    glfwSetWindowPos(m_Window, 0, 0);

    GLFWwindow* oldWindow = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_Window);

    glfwSetMouseButtonCallback(m_Window, RadarWindowMouseCallback);

    /*Setup background map*/
    float BgTexturePos[] = {
          0.0f,     0.0f,   0.0f, 0.0f,
       1024.0f,     0.0f,   1.0f, 0.0f,
       1024.0f,  1024.0f,   1.0f, 1.0f,
          0.0f,  1024.0f,   0.0f, 1.0f
    };

    unsigned int BgTextureIndicies[] =
    {
        0, 1, 2,
        2, 3, 0
    };

    m_MapVA = new VertexArray();
    m_MapVB = new VertexBuffer(BgTexturePos, sizeof(float) * 4 * 4);

    VertexBufferLayout bgTextureLayout;
    bgTextureLayout.Push<float>(2);
    bgTextureLayout.Push<float>(2);

    m_MapVA->AddBuffer(*m_MapVB, bgTextureLayout);
    m_MapIB = new IndexBuffer(BgTextureIndicies, 6);
    m_MapShader = new Shader(s_TextureShaderVertex, s_TextureShaderFragment);
    m_MapShader->Bind();
    m_MapTexture = new Texture(GetMapTextureLocation(m_CurrentMap));
    m_MapShader->SetUniform1i("u_Texture", 0);

    /*
    The player will be rendered on a square map that's originally 1024x1024
    on our map (0, 0) = bottom left AND (1024, 1024) = top right
    .*/
    m_Proj = glm::ortho(0.0f, 1024.0f, 0.0f, 1024.0f, -1.0f, 1.0f);

    m_View = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    m_MapModel = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 MapMvp = m_Proj * m_View * m_MapModel;
    m_MapShader->SetUniformMat4f("u_MVP", MapMvp);

    float PlayerMarkPositions[]
    {
        -12.0f, -12.0f,
         12.0f, -12.0f,
          0.0f,  19.0f
    };

    unsigned int PlayerMarkIndicies[] =
    {
        0, 1, 2
    };

    m_PlayerMarkShader = new Shader(s_BasicShaderVertex, s_BasicShaderFragment);
    m_PlayerMarkShader->Bind();
    m_PlayerMarkShader->SetUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.0f);

    m_PlayerMarkModel = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 PlayerMvp = m_Proj * m_View * m_PlayerMarkModel;
    m_PlayerMarkShader->SetUniformMat4f("u_MVP", PlayerMvp);

    m_PlayerMarkVA = new VertexArray();
    m_PlayerMarkVB = new VertexBuffer(PlayerMarkPositions, sizeof(float) * 2 * 3);

    VertexBufferLayout PlayerMarkLayout;
    PlayerMarkLayout.Push<float>(2);

    m_PlayerMarkVA->AddBuffer(*m_PlayerMarkVB, PlayerMarkLayout);

    m_PlayerMarkIB = new IndexBuffer(PlayerMarkIndicies, 3);

    glfwMakeContextCurrent(oldWindow);
}

void Radar::Destroy()
{
    GLFWwindow* oldWindow = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_Window);

    glfwDestroyWindow(m_Window);
    m_Window = nullptr;

    delete(m_MapTexture);
    delete(m_MapVB);
    delete(m_MapVA);
    delete(m_MapIB);
    delete(m_MapShader);

    delete(m_PlayerMarkVB);
    delete(m_PlayerMarkVA);
    delete(m_PlayerMarkIB);
    delete(m_PlayerMarkShader);

    glfwMakeContextCurrent(oldWindow);
}

void Radar::OnRender(CSGO::EntityList& entityList, CSGO::ClientState& clientState)
{
    GLFWwindow* oldWindow = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_Window);

    m_Renderer.Clear();

    if (RadarWindowMouseHeld)
    {
        int RadarX, RadarY;
        glfwGetWindowSize(m_Window, &RadarX, &RadarY);

        POINT cursorPos{};
        GetCursorPos(&cursorPos);
        glfwSetWindowPos(m_Window, cursorPos.x - RadarX/2, cursorPos.y - RadarY/2);
    }

    if (m_CurrentMap.GetMapName() != clientState.GetCurrentMapName())
        ChangeMap(clientState.GetCurrentMapName());

    const std::vector<CSGO::Player*>& validPlayers = entityList.GetValidPlayers();
    const CSGO::LocalPlayer& localPlayer = entityList.GetLocalPlayer();

    /*Draw map and calc projection*/
    m_MapTexture->Bind();

    m_MapShader->Bind();
    if (m_RadarConfig.AlwaysCentered)
    {
        glm::vec3 lpRadarOrigin = WorldToRadar(localPlayer.GetOrigin()) * m_RadarConfig.MapScale;
        /*To apply rotation we need to change this so the camera remains in the center*/

        glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3((-lpRadarOrigin.x) + 512, (-lpRadarOrigin.y) + 512, 0));

        /*Apply scaling*/
        m_View = glm::scale(trans, glm::vec3(m_RadarConfig.MapScale, m_RadarConfig.MapScale, 0.0f));

        /*@todo Could make the radar rotateable*/
        //m_View = glm::rotate(scaledTrans, glm::radians(localPlayer.GetViewAnglesY() - 90.0f), glm::vec3(0, 0, 1));
    }
    else
    {
        m_View = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0));
    }
    glm::mat4 MapMvp = m_Proj * m_View * m_MapModel;
    m_MapShader->SetUniformMat4f("u_MVP", MapMvp);
    m_Renderer.DrawTriangles(*m_MapVA, *m_MapIB, *m_MapShader);

    /*Draw players*/
    m_PlayerMarkShader->Bind();

    for (CSGO::Player* player : validPlayers)
    {
        if (player->GetTeam() == localPlayer.GetTeam() && !m_RadarConfig.ShowTeammates && player->GetEntityAddress() != localPlayer.GetEntityAddress())
            continue;

        /*Determine player color on radar*/
        if(player->IsDefusing())
            m_PlayerMarkShader->SetUniform4f("u_Color",
                m_RadarConfig.DefuserColor[0],
                m_RadarConfig.DefuserColor[1],
                m_RadarConfig.DefuserColor[2],
                m_RadarConfig.DefuserColor[3]);

        else if (player->GetEntityAddress() == localPlayer.GetEntityAddress()) {
            m_PlayerMarkShader->SetUniform4f("u_Color", 
                m_RadarConfig.LocalPlayerColor[0], 
                m_RadarConfig.LocalPlayerColor[1],
                m_RadarConfig.LocalPlayerColor[2],
                m_RadarConfig.LocalPlayerColor[3]); 
        }
        else if (player->GetTeam() == localPlayer.GetTeam())
            m_PlayerMarkShader->SetUniform4f("u_Color", 
                m_RadarConfig.TeamPlayerColor[0],
                m_RadarConfig.TeamPlayerColor[1],
                m_RadarConfig.TeamPlayerColor[2],
                m_RadarConfig.TeamPlayerColor[3]);
        else
            m_PlayerMarkShader->SetUniform4f("u_Color", 
                m_RadarConfig.EnemyPlayerColor[0],
                m_RadarConfig.EnemyPlayerColor[1],
                m_RadarConfig.EnemyPlayerColor[2],
                m_RadarConfig.EnemyPlayerColor[3]);

        /*apply rotation and scale and move to right place*/
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), WorldToRadar(player->GetOrigin()));
        glm::mat4 scaledTrans = glm::scale(trans, glm::vec3(m_RadarConfig.PlayerScale, m_RadarConfig.PlayerScale, 0.0f));

        if(player->GetEntityAddress() == localPlayer.GetEntityAddress())    //If we read localPlayers rotation from the entityList it's extermely slow?
            m_PlayerMarkModel = glm::rotate(scaledTrans, glm::radians(clientState.GetViewAngles().y - 90.0f), glm::vec3(0, 0, 1));
        else
            m_PlayerMarkModel = glm::rotate(scaledTrans, glm::radians(player->GetViewAnglesY()-90.0f), glm::vec3(0, 0, 1));

        glm::mat4 PlayerMvp = m_Proj * m_View * m_PlayerMarkModel;
        m_PlayerMarkShader->SetUniformMat4f("u_MVP", PlayerMvp);

        m_Renderer.DrawTriangles(*m_PlayerMarkVA, *m_PlayerMarkIB, *m_PlayerMarkShader);
    }

    glfwSwapBuffers(m_Window);
    glfwPollEvents();

    glfwMakeContextCurrent(oldWindow);
}

bool Radar::HasWindow()
{
    if (m_Window == nullptr)
        return false;
    return true;
}

void Radar::ChangeMap(std::string mapName)
{
    m_CurrentMap = CSGOMap(mapName);

    delete(m_MapTexture);
    m_MapTexture = new Texture(GetMapTextureLocation(m_CurrentMap));
}

std::string Radar::GetMapTextureLocation(CSGOMap map)
{
    std::ostringstream oss;
    oss << "res/textures/"
        << map.GetMapName()
        << "_radar.dds";

    std::ifstream ifs(oss.str());
    if (!ifs)
    {
        std::ostringstream ossCS;
        ossCS
            << g_SECONDPROCPATH
            << "\\csgo\\resource\\overviews\\"
            << map.GetMapName()
            << "_radar.dds";

        if (!CopyFileA(ossCS.str().c_str(), oss.str().c_str(), false))
            std::cout << "Couldn't copy file " << ossCS.str() << " to " << oss.str() << '\n';
            //@todo not found // couldn't copy, it must be a workshop map, parse from that?
    }

    return oss.str();
}

glm::vec3 Radar::WorldToRadar(const glm::vec3 world)
{
    /*Counter - Strike Global Offensive\csgo\resource\overviews*/

    float scale = m_CurrentMap.GetMapScale();
    int originX = (int)m_CurrentMap.GetMapOriginX();
    int originY = (int)m_CurrentMap.GetMapOriginY();

    return { (world.x - originX) / scale,
             1024-(-(world.y - originY) / scale), 0};
    //Y starts from the bottom left, not the top left.*/
}

bool CSGOMap::ParseMapInfo()
{
    std::stringstream oss;
    oss << "res/textures/"
        << m_Name
        << ".txt";

    std::ifstream ifs(oss.str());
    ifs.exceptions(ifs.exceptions() | std::ios_base::badbit);

    if (!ifs)
        /*Try copying from game files*/
    {
        std::ostringstream ossCS;
        ossCS 
            << g_SECONDPROCPATH
            << "\\csgo\\resource\\overviews\\"   
            << m_Name
            << ".txt";

        std::cout << ossCS.str() << '\n';

        if (!CopyFileA(ossCS.str().c_str(), oss.str().c_str(), false))
            std::cout << "Couldn't copy file " << ossCS.str() << " to " << oss.str() << '\n';

        ifs.clear();
        ifs.open(oss.str());
        if (!ifs)
        {
            //@todo IF STILL NOT FOUND, IT MUST BE A WORKSHOP MAP.  just parse from BSP and make sure to close the handle
            return false;
        }  
    };
    
    std::vector<std::string> valuesToFind
    {   /*They have to be in order*/
        "pos_x",
        "pos_y",
        "scale"
    };
    int currentValue = 0;

    for (std::string currentLine; std::getline(ifs, currentLine);)
    {
        std::string::size_type pos = currentLine.find(valuesToFind[currentValue]);
        if(pos == std::string::npos)
            continue;   /*Not found*/

        const std::string digits = "0123456789.+-";

        float value;

        unsigned ipos = currentLine.find_first_of(digits);
        if (ipos != std::string::npos)
        {
            std::stringstream(currentLine.substr(ipos)) >> value;
            if (valuesToFind[currentValue] == "pos_x")
                m_OriginX = value;
            else if (valuesToFind[currentValue] == "pos_y")
                m_OriginY = value;
            else if (valuesToFind[currentValue] == "scale")
                m_Scale = value;
            ++currentValue;
        }
        else return false   /*Couldn't find a number*/;
    }

    return true;
}

CSGOMap::CSGOMap(std::string MapName)
    : 
    m_Name(MapName),
    m_Scale(-1),
    m_OriginX(-1),
    m_OriginY(-1)
{
    ParseMapInfo();
    /*Parse info from m_Name + .txt file*/
}

