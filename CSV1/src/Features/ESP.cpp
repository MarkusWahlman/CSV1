#include "ESP.h"

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
        "[OpenGL Message from ESP.cpp]:" << '\n' <<
        "Message: " << message << '\n' <<
        "Type: " << type << '\n' <<
        "Severity: " << severity << '\n' <<
        "Id: " << id << '\n' << '\n';
}

ESP::ESP(ESPConfig& ESPConfig)
    : 
    m_ESPConfig(ESPConfig),
    m_Window(nullptr),
    m_WindowX(1920),
    m_WindowY(1080),

    m_PlayerBoxVB(nullptr),
    m_PlayerBoxVA(nullptr),
    m_PlayerBoxIB(nullptr),
    m_Shader(nullptr),
    m_PlayerBoxModel({}),

    m_Proj({}),
    m_View({})
{
}

ESP::~ESP()
{
}

/*Csgo player model size approximately*/
constexpr float m_BoxSizeY = 0.73f;
constexpr float m_BoxSizeX = 0.35f;
constexpr float m_HPBoxSizeX = 0.04f;

void ESP::Init()
{
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    /*  @todo We don't have the latest GLFW version and the overlay won't work without
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);  */

    glDebugMessageCallback(GLDebugCallback, nullptr);
    glEnable(GL_DEBUG_OUTPUT);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    m_WindowX = mode->width;
    m_WindowY = mode->height;

    m_Window = glfwCreateWindow(
        m_WindowX,
        m_WindowY,
        "",
        nullptr,
        nullptr); 

    if (!m_Window)
    {
        glfwTerminate();
        throw std::runtime_error("couldn't create esp window");
    }

    GLFWwindow* oldWindow = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_Window);

    m_Shader = new Shader(s_BasicShaderVertex, s_BasicShaderFragment);
    m_Shader->Bind();
    m_Shader->SetUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.0f);

    m_Proj = glm::mat4(1.0f);
    m_View = glm::mat4(1.0f);

    m_Proj = glm::perspective(glm::radians(90.0f), (float)m_WindowX / (float)m_WindowY, 0.1f, 100.0f);

    /*Player box*/
    float PlayerBoxPositions[]
    {
         -m_BoxSizeX, -m_BoxSizeY, 0.0f,
          m_BoxSizeX, -m_BoxSizeY, 0.0f,
          m_BoxSizeX,  m_BoxSizeY, 0.0f,
         -m_BoxSizeX,  m_BoxSizeY, 0.0f
    };

    unsigned int PlayerBoxIndicies[] =
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0
    };

    m_PlayerBoxModel = glm::mat4(1.0f);

    glm::mat4 PlayerMvp = m_Proj * m_View * m_PlayerBoxModel;
    m_Shader->SetUniformMat4f("u_MVP", PlayerMvp);

    m_PlayerBoxVA = new VertexArray();
    m_PlayerBoxVB = new VertexBuffer(PlayerBoxPositions, sizeof(float) * 3 * 4);

    VertexBufferLayout PlayerBoxLayout;
    PlayerBoxLayout.Push<float>(3);

    m_PlayerBoxVA->AddBuffer(*m_PlayerBoxVB, PlayerBoxLayout);
    m_PlayerBoxIB = new IndexBuffer(PlayerBoxIndicies, 8);

    /*HP Box*/
    float HPBoxPositions[]
    {
         -m_HPBoxSizeX, -m_BoxSizeY, 0.0f, //1
          m_HPBoxSizeX, -m_BoxSizeY, 0.0f, //2
          m_HPBoxSizeX,  m_BoxSizeY, 0.0f, //3
         -m_HPBoxSizeX,  m_BoxSizeY, 0.0f  //4
    };

    unsigned int HPBoxIndicies[] =
    {
         0, 1, 2,
         2, 3, 0
    };

    m_HPBoxModel = glm::mat4(1.0f);

    m_HPBoxVA = new VertexArray();
    m_HPBoxVB = new VertexBuffer(HPBoxPositions, sizeof(float) * 3 * 4, true);

    VertexBufferLayout HPBoxLayout;
    HPBoxLayout.Push<float>(3);

    m_HPBoxVA->AddBuffer(*m_HPBoxVB, HPBoxLayout);
    m_HPBoxIB = new IndexBuffer(HPBoxIndicies, 6);

    /*Bones*/
    m_BonesModel = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

    m_BonesToBeDrawn =
    {
    /*Spine and go to left hand*/
    3,
    4,
    5,
    6,

    /*Go to left arm*/
    10,
    11,

    /*Finger bone meeting point*/
    12,

    11,
    10,
    6,

    };
    std::vector<float> BonePositions(m_BonesToBeDrawn.size() * 2);
    std::vector<int> BoneIndicies
    {
        0, 1
    };

    for (int i = 2; i < (int)m_BonesToBeDrawn.size(); ++i)
    {
        BoneIndicies.push_back(BoneIndicies[BoneIndicies.size() - 1]);
        BoneIndicies.push_back(BoneIndicies[BoneIndicies.size() - 1]+1);
    }


    m_BonesVA = new VertexArray();
    m_BonesVB = new VertexBuffer(BonePositions.data(), sizeof(float) * BonePositions.size(), true);

    VertexBufferLayout BonesLayout;
    BonesLayout.Push<float>(2);

    m_BonesVA->AddBuffer(*m_BonesVB, BonesLayout);

    m_BonesIB = new IndexBuffer(BoneIndicies.data(), BoneIndicies.size());

    glfwMakeContextCurrent(oldWindow);
}

void ESP::Destroy()
{
    GLFWwindow* oldWindow = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_Window);

    glfwDestroyWindow(m_Window);
    m_Window = nullptr;

    delete(m_PlayerBoxVB);
    delete(m_PlayerBoxVA);
    delete(m_PlayerBoxIB);

    delete(m_HPBoxVB);
    delete(m_HPBoxVA);
    delete(m_HPBoxIB);

    delete(m_BonesVB);
    delete(m_BonesVA);
    delete(m_BonesIB);

    delete(m_Shader);

    glfwMakeContextCurrent(oldWindow);
}

void ESP::OnRender(CSGO::EntityList& entityList, CSGO::ClientState& clientState)
{
    m_WindowX = clientState.GetWindowX();
    m_WindowY = clientState.GetWindowY();

    GLFWwindow* oldWindow = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_Window);

    m_Renderer.ClearColor(0.0, 0.0, 0.0, 0.0);
    m_Renderer.Clear();

    glm::mat4 viewMatrix = clientState.GetViewMatrix();

    /*Rendering*/
    const std::vector<CSGO::Player*>& validPlayers = entityList.GetValidPlayers();
    const CSGO::LocalPlayer& localPlayer = entityList.GetLocalPlayer();

    //Check and change FOV
    if (localPlayer.GetFOV() == 0 || localPlayer.GetFOV() == 90)
    {
        m_Proj = glm::perspective(glm::radians(90.0f), (float)m_WindowX / (float)m_WindowY, 0.1f, 100.0f);
    }else
        m_Proj = glm::perspective(glm::radians(float(localPlayer.GetFOV())), (float)m_WindowX / (float)m_WindowY, 0.1f, 100.0f);
    
    m_Shader->Bind();
    for (CSGO::Player* player : validPlayers)
    {
        if (player->GetEntityAddress() == localPlayer.GetEntityAddress()) //Ignore localPlayer
            continue;

        if (player->GetTeam() == localPlayer.GetTeam() && !m_ESPConfig.ShowTeammates)
            continue;

        glm::vec3 PlayerMiddlePos = player->GetOrigin();
        PlayerMiddlePos.z += 64.0626f/*Not read correctly from the game, works like this*/ / 2;

        glm::vec3 ScreenPos = CSGO::WorldToScreen(PlayerMiddlePos, viewMatrix, m_WindowX, m_WindowY);
        if (ScreenPos.x == -1)
            continue;

        glm::vec3 unProjectedScreenPos = glm::unProject(ScreenPos, m_View, m_Proj, glm::vec4(0.0f, 0.0f, m_WindowX, m_WindowY));

        /*Color according to team / Defusing*/
        if (player->IsDefusing())
            m_Shader->SetUniform4f("u_Color",
                m_ESPConfig.DefuserBoxColor[0],
                m_ESPConfig.DefuserBoxColor[1],
                m_ESPConfig.DefuserBoxColor[2],
                m_ESPConfig.DefuserBoxColor[3]);
        else if (player->GetTeam() == localPlayer.GetTeam())
            m_Shader->SetUniform4f("u_Color",
                m_ESPConfig.TeamPlayerBoxColor[0],
                m_ESPConfig.TeamPlayerBoxColor[1],
                m_ESPConfig.TeamPlayerBoxColor[2],
                m_ESPConfig.TeamPlayerBoxColor[3]);
        else
            m_Shader->SetUniform4f("u_Color",
                m_ESPConfig.EnemyPlayerBoxColor[0],
                m_ESPConfig.EnemyPlayerBoxColor[1],
                m_ESPConfig.EnemyPlayerBoxColor[2],
                m_ESPConfig.EnemyPlayerBoxColor[3]);

        if (m_ESPConfig.DrawBoxes)
        {
            m_PlayerBoxModel = glm::translate(glm::mat4(1.0f), unProjectedScreenPos);

            glm::mat4 PlayerBoxMvp = m_Proj * m_View * m_PlayerBoxModel;
            m_Shader->SetUniformMat4f("u_MVP", PlayerBoxMvp);
            m_Renderer.DrawLines(*m_PlayerBoxVA, *m_PlayerBoxIB, *m_Shader);
        }
        
        if (m_ESPConfig.DrawBones)
        {
            bool shouldDraw = true;

            m_BoneOrthoProj = glm::ortho(0.0f, float(m_WindowX), 0.0f, float(m_WindowY), -1.0f, 1.0f);

            const std::array<glm::vec3, CSGO_MAXBONES>& bonesArray = player->GetBonePositions();

            std::vector<float> BoneScreenPositions;
            for (int boneID : m_BonesToBeDrawn)
            {
                glm::vec3 boneScreenPos = CSGO::WorldToScreen(bonesArray[boneID + 1], viewMatrix, m_WindowX, m_WindowY);
                if (boneScreenPos.x < 1)
                {
                    shouldDraw = false;
                    break;
                }

                BoneScreenPositions.push_back(boneScreenPos.x);
                BoneScreenPositions.push_back(boneScreenPos.y);
            }

            if (shouldDraw)
            {
                m_BonesVB->ChangeData(BoneScreenPositions.data());

                /*Using ortho proj*/
                glm::mat4 BoneMvp = m_BoneOrthoProj * m_View * m_BonesModel;
                m_Shader->SetUniformMat4f("u_MVP", BoneMvp);
                m_Renderer.DrawLines(*m_BonesVA, *m_BonesIB, *m_Shader);
            }
        }

        if (m_ESPConfig.DrawHP)
        {
            float HPMultValue = float(player->GetHealth()) / 100.0f;
            m_HPBoxModel = glm::translate(glm::mat4(1.0f), glm::vec3(unProjectedScreenPos.x + m_BoxSizeX + m_HPBoxSizeX, unProjectedScreenPos.y, unProjectedScreenPos.z));
            float HPBoxPositions[]
            {   /*Multiplying the default values according to player HP*/
                  m_HPBoxSizeX,  (m_BoxSizeY*2 * HPMultValue) - m_BoxSizeY, 0.0f,
                 -m_HPBoxSizeX,  (m_BoxSizeY*2 * HPMultValue) - m_BoxSizeY, 0.0f
            };
            m_HPBoxVB->ChangeData(HPBoxPositions, sizeof(float) * 6, sizeof(float) * 6);

            /*Color according to hp*/
            m_Shader->SetUniform4f("u_Color", 1.0f, HPMultValue, 0.0f, 1.0f); 

            glm::mat4 PlayerHPMvp = m_Proj * m_View * m_HPBoxModel;
            m_Shader->SetUniformMat4f("u_MVP", PlayerHPMvp);
            m_Renderer.DrawTriangles(*m_HPBoxVA, *m_HPBoxIB, *m_Shader);
        }
    }
    
    glfwSwapBuffers(m_Window);
    glfwPollEvents();

    glfwMakeContextCurrent(oldWindow);
}

bool ESP::HasWindow()
{
    if (m_Window == nullptr)
        return false;
    return true;
}