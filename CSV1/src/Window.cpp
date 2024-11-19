#include "RenderAPI/Renderer.h"
#include "Process/Process.h"
#include "Process/Struct/CSGO.h"
#include "SimpleAuth/SimpleAuth.h"

#include "Features/Radar.h"
#include "Features/ESP.h"
#include "Features/Aimbot.h"
#include "Features/Bunnyhop.h"

#include "../res/resource.h"

#include "Window.h"
#include <sstream>
#include <stdexcept>
#include <memory>
#include <chrono>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include "imgui/custom/custom.h"

constexpr static float CurrentVersion = 1.0f;

static bool CheatWindowShouldMove = false;
static ImVec2 HoldWindowPosition{};
static int LastButtonPressed;

static void CheatWindowMouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    LastButtonPressed = button;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        HoldWindowPosition = ImGui::GetMousePos();
        if(!ImGui::IsAnyItemHovered())
            CheatWindowShouldMove = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        CheatWindowShouldMove = false;
    }
}

MAYUMI::Window::Window()
{
    if (!glfwInit())
        throw std::runtime_error("couldn't initialize glfw");

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW , GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    m_Window = glfwCreateWindow(500, 500, "", NULL, NULL);
    if (!m_Window)
    {
        glfwTerminate();
        throw std::runtime_error("couldn't create glfw window");
    }

    glfwMakeContextCurrent(m_Window);

    glfwSetMouseButtonCallback(m_Window, CheatWindowMouseCallback);

    ImGui::CreateContext();

    /*Style & Colors*/
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    /*custom values*/
    style.FrameRounding = 2.0f;
    style.CellPadding = ImVec2(4, 1);
    style.ItemInnerSpacing = ImVec2(5, 4);
    style.FramePadding = ImVec2(20, 3);
    style.ItemSpacing = ImVec2(10, 4);
    style.PopupBorderSize = 0.0f;
    style.GrabRounding = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.73f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.49f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.35f, 0.98f, 1.00f);

    /*Font*/
    std::string fontFilePath("C:\\Windows\\Fonts\\arial.ttf");
    float fontSize = 18.f;

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (!io.Fonts->AddFontFromFileTTF(fontFilePath.c_str(), fontSize))
        std::cout << "couldn't load font from " << fontFilePath << '\n';

    /*Disable imgui.ini*/
    io.IniFilename = NULL;

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init();

    glewInit();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


static bool HasTimePassed(int duration)
{
    static std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::steady_clock::now();
    static int timeExecuted = 0;
    if (timeExecuted < duration) {
        std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::steady_clock::now();
        std::chrono::duration<__int64, std::nano> difference = endTime - startTime;
        timeExecuted = (int)std::chrono::duration_cast<std::chrono::seconds>(difference).count();
        return false;
    }
    return true;
}

static bool WaitForGameWindow(Process& proc)
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    /*Don't need to wait, already in required state*/
    if (flag_has_value(proc.State(), Process::STATE::IsChildProcess))
        return true;    

    if (ImGui::Begin("Wait for game", nullptr, flags))
    {
        /*Display Mayumi logo image*/
        static Texture LogoTexture("res//textures//logo.png", false);
        ImGui::ImageCentered((ImTextureID)LogoTexture.GetRendererID(), ImVec2((float)LogoTexture.GetWidth(), (float)LogoTexture.GetHeight()));
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        proc.FindHandles();
        if (flag_has_value(proc.State(), Process::STATE::ProcessFound))
        {
            if (flag_has_value(proc.State(), Process::STATE::HandleFound))
            {
                ImGui::TextCentered("Starting process...");
                if (HasTimePassed(7)) /*Wait 7s so csgo has been fully started*/
                {
                    if (proc.ChangeFirstProcHandleInherit(TRUE) && proc.StartProcessAsFirstProcChild())
                    {
                        /*When we have started the other process as a child, we can quit this one.*/
                        proc.ChangeFirstProcHandleInherit(FALSE);
                        exit(1);
                    }
                    else
                    {
                        ImGui::TextCentered("Error starting the process, try restarting your computer.");
                        ImGui::TextCentered("If the issue persists read the README.");
                        ImGui::TextCentered("Contact us at mayumicheat@gmail.com.");
                    }
                }
            }
            else 
            {
                ImGui::TextCentered("Waiting for required state...");
                ImGui::TextCentered("If the issue persists read the README.");
                ImGui::TextCentered("Contact us at mayumicheat@gmail.com.");
            }
        }
        else
        {
            ImGui::TextCentered("Waiting for csgo...");
        }
    }
    ImGui::End();
    
    return false;
}

__forceinline static bool VerificationWindow(Process& proc, bool& verified, SimpleAuth& auth)
{
    /*In verification window the game should be up already, if the target process is closed this process will close aswell.*/
    if (!proc.SecondProcStillRunning())
        exit(1);

    if (flag_has_value(proc.State(), 
        Process::STATE::HandleFound     | 
        Process::STATE::IsChildProcess  |
        Process::STATE::ModulesFound    |
        Process::STATE::OffsetsFound    |
        Process::STATE::ProcessFound)   &&
        verified)
            return true;

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    static bool processError = false;
    static bool authError = false;

    if (ImGui::Begin("Verification", nullptr, flags))
    {
        /*Display Mayumi logo image*/
        static Texture LogoTexture("res//textures//logo.png", false);
        ImGui::ImageCentered((ImTextureID)LogoTexture.GetRendererID(), ImVec2((float)LogoTexture.GetWidth(), (float)LogoTexture.GetHeight()));
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        static std::string email = "";
        static std::string password = "";

        ImGui::InputText("Email", &email);
        ImGui::InputText("Password", &password, ImGuiInputTextFlags_Password);

        ImGui::NewLine();

        if (ImGui::Button("Login") || ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            if (auth.SignIn(email, password))
            {
                proc.Setup();
                if (flag_has_value(proc.State(),
                    Process::STATE::HandleFound |
                    Process::STATE::IsChildProcess |
                    Process::STATE::ModulesFound |
                    Process::STATE::OffsetsFound |
                    Process::STATE::ProcessFound))
                        verified = true;
                else
                    processError = true;
            }
            else
                authError = true;
        }

        std::stringstream ssCurrent;
        ssCurrent << "Current version: " << std::fixed << std::setprecision(1) << CurrentVersion;
        ImGui::Text(ssCurrent.str().c_str());

        if (auth.GetVersion() != CurrentVersion && auth.GetVersion() != 0.0f)
        {
            std::stringstream ssAvailable;
            ssAvailable << "Version " << std::fixed << std::setprecision(1) << auth.GetVersion() << " is available!";
            ImGui::Text(ssAvailable.str().c_str());
            ImGui::AddUnderLine(ImColor(1.0f, 0.0f, 0.0f, 1.0f));
            if (ImGui::IsItemHovered())
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Hand);
            }
            if (ImGui::IsItemClicked())
            {
                ShellExecute(0, 0, L"https://mayumi.ga/subscription.html", 0, 0, SW_SHOW);
            }
            ImGui::NewLine();
        }

        if (processError)
        {
            ImGui::Text("Error with the process.");
            ImGui::Text("If the issue persists make sure to read the README.");
            ImGui::Text("Contact us at mayumicheat@gmail.com.");
        }

        if (authError)
        {
            ImGui::Text("Authentication error!");
        }

    }
    ImGui::End();
    return false;
}

/*Different cheat tabs*/
class ImGuiCheatTab
{
public:
    ImGuiCheatTab(std::string name, bool open, CSGO::EntityList& entityList, CSGO::ClientState& clientState)
        : m_Name(name), m_Open(open), m_EntityList(entityList), m_ClientState(clientState)
    {}

    virtual void OnImGuiRender() = 0;
    virtual void OnTabStateChange(){};

    bool& IsOpen() { return m_Open; }
    const std::string& GetName() const { return m_Name; }

protected:
    std::string m_Name;
    bool m_Open;
    CSGO::EntityList& m_EntityList;
    CSGO::ClientState& m_ClientState;
};

class VisualTab : public ImGuiCheatTab
{
public:

    VisualTab(std::string name, bool open, CSGO::EntityList& entityList, CSGO::ClientState& clientState)
        : ImGuiCheatTab(name, open, entityList, clientState),
        m_RadarConfig(false, 1.0f, 1.0f),
        m_Radar(CSGOMap(clientState.GetCurrentMapName()), m_RadarConfig),
        m_RadarActive(false),

        m_ESPConfig(),
        m_ESP(m_ESPConfig),
        m_ESPActive(false)
    {
        if (m_RadarActive)
            m_Radar.Init();
    }

    void OnTabStateChange() override
    {
        if (m_RadarActive && m_Radar.HasWindow()) m_Radar.Destroy();
        else if (m_RadarActive) m_Radar.Init();

        if (m_ESPActive && m_ESP.HasWindow()) m_ESP.Destroy();
        else if (m_ESPActive) m_ESP.Init();
    }

    void OnImGuiRender() override
    {
        ImGui::Text("Visuals:");

        if(ImGui::Checkbox("Radar", &m_RadarActive))
        {
            if(m_Radar.HasWindow()) m_Radar.Destroy();
            else m_Radar.Init();
        }
        if (m_RadarActive && m_Open)
        {
            m_Radar.OnRender(m_EntityList, m_ClientState);
            ImGui::Indent();
                ImGui::ColorEdit4("Defuser##Marks", m_RadarConfig.DefuserColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::ColorEdit4("LocalPlayer##Marks", m_RadarConfig.LocalPlayerColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::ColorEdit4("Team##Marks", m_RadarConfig.TeamPlayerColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::ColorEdit4("Enemy##Marks", m_RadarConfig.EnemyPlayerColor, ImGuiColorEditFlags_NoInputs);
                ImGui::Checkbox("Always Centered", &m_RadarConfig.AlwaysCentered);
                ImGui::Checkbox("Show teammates##Marks", &m_RadarConfig.ShowTeammates);
                if (m_RadarConfig.AlwaysCentered)
                {
                    ImGui::SliderFloat("Map Scale", &m_RadarConfig.MapScale, 0.5f, 3.5f);
                }
                ImGui::Text("Marks:");
                ImGui::SliderFloat("Scale", &m_RadarConfig.PlayerScale, 0.0f, 3.0f); 
               
            ImGui::Unindent();
        }

        if (ImGui::Checkbox("ESP", &m_ESPActive))
        {
            if (m_ESP.HasWindow()) m_ESP.Destroy();
            else m_ESP.Init();
        }
        if (m_ESPActive && m_Open)
        {
            m_ESP.OnRender(m_EntityList, m_ClientState);
            ImGui::Indent();
                ImGui::ColorEdit4("Defuser##Boxes", m_ESPConfig.DefuserBoxColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::ColorEdit4("Team##Boxes", m_ESPConfig.TeamPlayerBoxColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::ColorEdit4("Enemy##Boxes", m_ESPConfig.EnemyPlayerBoxColor, ImGuiColorEditFlags_NoInputs);

                ImGui::Checkbox("Show teammates##Boxes", &m_ESPConfig.ShowTeammates);
                ImGui::Checkbox("Boxes", &m_ESPConfig.DrawBoxes);
                ImGui::Checkbox("HP", &m_ESPConfig.DrawHP);
                ImGui::Checkbox("Draw arm bones", &m_ESPConfig.DrawBones);
            ImGui::Unindent();
        }

        ImGui::NewLine();
    }

private:
    Radar m_Radar;
    RadarConfig m_RadarConfig;
    bool m_RadarActive;

    ESP m_ESP;
    bool m_ESPActive;
    ESPConfig m_ESPConfig;
};

class AimbotTab : public ImGuiCheatTab
{
public:
    AimbotTab(std::string name, bool open, CSGO::EntityList& entityList, CSGO::ClientState& clientState)
        : ImGuiCheatTab(name, open, entityList, clientState),
        m_AimbotConfig(),
        m_Aimbot(m_AimbotConfig, entityList, clientState)
    {}

    void OnTabStateChange() override
    {
        if (!m_Open && m_AimbotConfig.AimbotActive)
        {
            m_AimbotConfig.AimbotActive = false;
            m_Aimbot.HandleThreads();
            m_AimbotConfig.AimbotActive = true; /*The thread will start when m_Open is switched*/
        } 
    }

    void OnImGuiRender() override
    {
        ImGui::Text("Aimbot:");

        ImGui::Checkbox("Aim assist", &m_AimbotConfig.AimbotActive);
        m_Aimbot.HandleThreads();
        if (m_AimbotConfig.AimbotActive) 
        {
            ImGui::Indent();
                ImGui::SliderInt("Slowness", &m_AimbotConfig.Speed, 1, 50);
                    ImGui::SameLine();
                    ImGui::HelpMarker("How slow the aim assist will be.");

                ImGui::Checkbox("Stop assist when shooting", &m_AimbotConfig.StopWhenShooting);
                    ImGui::SameLine(); 
                    ImGui::HelpMarker("Stops aim assist when you are shooting.");

                ImGui::SliderInt("Change dir", &m_AimbotConfig.ChangeDirectionIf, 1, 25);    
                    ImGui::SameLine(); 
                    ImGui::HelpMarker("When the target is overaimed this decides how far the crosshair\n"
                                      "will to go from the original target before changing direction.\n"
                                      "Should be kept at 1 or 2 for optimal aim assist experience.");

                ImGui::SliderFloat("Activate dist", &m_AimbotConfig.ActivateDistance, 1.0f, 250.0f);
                    ImGui::SameLine();
                    ImGui::HelpMarker("The minimum distance from a player\n"
                                      "for the aim assist to activate.");

                ImGui::SliderInt("StopClose dist", &m_AimbotConfig.StopAimingDistance, 1, 50);
                    ImGui::SameLine();
                    ImGui::HelpMarker("How close to the desired point should the\n"
                                      "crosshair get before stopping aiming.");

                    ImGui::Checkbox("Aim teammates", &m_AimbotConfig.AimTeammates);
                        ImGui::SameLine();
                        ImGui::HelpMarker("Will enable aimbot on your own teammates.");

                if (ImGui::Button("Bones", ImVec2(-1, 0)))
                    ImGui::OpenPopup("BonesOptions");
                if (ImGui::BeginPopup("BonesOptions", ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::CheckboxFlags("Head", (unsigned int*)&m_AimbotConfig.BoneFlags, (unsigned int)CSGO::BoneFlags::HEAD);
                    ImGui::CheckboxFlags("Neck", (unsigned int*)&m_AimbotConfig.BoneFlags, (unsigned int)CSGO::BoneFlags::NECK);
                    ImGui::CheckboxFlags("Upper spine", (unsigned int*)&m_AimbotConfig.BoneFlags, (unsigned int)CSGO::BoneFlags::UPPER_SPINE);
                    ImGui::CheckboxFlags("Middle spine", (unsigned int*)&m_AimbotConfig.BoneFlags, (unsigned int)CSGO::BoneFlags::MIDDLE_SPINE);
                    ImGui::CheckboxFlags("Lower spine", (unsigned int*)&m_AimbotConfig.BoneFlags, (unsigned int)CSGO::BoneFlags::LOWER_SPINE);
                    ImGui::CheckboxFlags("Hip", (unsigned int*)&m_AimbotConfig.BoneFlags, (unsigned int)CSGO::BoneFlags::HIP);
                    ImGui::CheckboxFlags("Pelvis", (unsigned int*)&m_AimbotConfig.BoneFlags, (unsigned int)CSGO::BoneFlags::PELVIS);
                    ImGui::EndPopup();
                }
                ImGui::InputKeyAndMouse("Key##Aimbot", "Mouse##Aimbot", &m_AimbotConfig.StartKey1, &m_AimbotConfig.StartKey2, LastButtonPressed);
                
            ImGui::Unindent();
        }

        ImGui::NewLine();
    }

private:
    Aimbot m_Aimbot;
    AimbotConfig m_AimbotConfig;
};

class MiscTab : public ImGuiCheatTab
{
public:
    MiscTab(std::string name, bool open, CSGO::EntityList& entityList, CSGO::ClientState& clientState)
        : ImGuiCheatTab(name, open, entityList, clientState),
        m_BunnyhopConfig(),
        m_Bunnyhop(m_BunnyhopConfig, entityList, clientState)
    {}

    void OnTabStateChange() override
    {
        if (!m_Open && m_BunnyhopConfig.BunnyhopActive)
        {
            m_BunnyhopConfig.BunnyhopActive = false;
            m_Bunnyhop.HandleThreads();
            m_BunnyhopConfig.BunnyhopActive = true; /*The thread will start when m_Open is switched*/
        }
    }

    void OnImGuiRender() override
    {
        ImGui::Text("Misc:");

        ImGui::Checkbox("Bunnyhop", &m_BunnyhopConfig.BunnyhopActive);
        m_Bunnyhop.HandleThreads();
        if (m_BunnyhopConfig.BunnyhopActive)
        {
            ImGui::Indent();
                ImGui::SliderInt("Miss chance", &m_BunnyhopConfig.MissChance, 1, 100);
                    ImGui::SameLine();
                    ImGui::HelpMarker("Increasing \"Miss chance\" will increase\n"
                                      "your chance of missing a hop.");
                    ImGui::InputKeyAndMouse("Key##Bunnyhop", "Mouse##Bunnyhop", &m_BunnyhopConfig.StartKey1, &m_BunnyhopConfig.StartKey2, LastButtonPressed);
                        ImGui::SameLine();
                        ImGui::HelpMarker("You can't bind any key that's already used for jumping.\n"
                                          "To use spacebar for bunnyhop, you will need to bind to \' \' and\n"
                                          "run this command to unbind space and bind jump to the \'p\' key:\n"
                                          "\"unbind space; bind p +jump;\" Click the button to copy.\n"
                                          "Remember to bind back using \"bind space +jump\" and \"unbind p\"");
                        ImGui::SameLine();
                        if (ImGui::Button("", ImVec2(20.0f, 20.0f)))
                        {
                            ImGui::SetClipboardText("unbind space; bind p +jump;");
                        }
            ImGui::Unindent();
        }
    }
private:
    Bunnyhop m_Bunnyhop;
    BunnyhopConfig m_BunnyhopConfig;
};

__forceinline static void CheatWindow(Process& proc, GLFWwindow* window)
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    static CSGO::ClientState clientState(proc);
    static CSGO::EntityList entityList(proc, clientState);

    entityList.Update();
    clientState.Update();

    if (ImGui::Begin("Cheat window", nullptr, flags))
    {
        ImGui::Indent(460);
        if (ImGui::Button("", ImVec2(25, 20)))
        {
            glfwIconifyWindow(window);
        }
        ImGui::Unindent(460);

        static std::vector<ImGuiCheatTab*> cheatTabs
        {
            new VisualTab("Visuals", true, entityList, clientState),
            new AimbotTab("Aimbot", false, entityList, clientState),
            new MiscTab("Misc", false, entityList, clientState)
        };
            
        for (unsigned int n = 0; n < cheatTabs.size(); n++)
        {
            if (n > 0) { ImGui::SameLine(); }
            if (ImGui::Checkbox(cheatTabs[n]->GetName().c_str(), &cheatTabs[n]->IsOpen()))
            {
                cheatTabs[n]->OnTabStateChange();
            }
        }
            
        ImGui::NewLine();

        for (unsigned int n = 0; n < cheatTabs.size(); n++)
        {
            if (cheatTabs[n]->IsOpen())
            {
                cheatTabs[n]->OnImGuiRender();
            }
        }
        /*Display Mayumi logo image*/
        static Texture LogoTexture("res//textures//logo.png", false);
        ImGui::ImageCentered((ImTextureID)LogoTexture.GetRendererID(), ImVec2((float)LogoTexture.GetWidth(), (float)LogoTexture.GetHeight()));
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
    }
    ImGui::End();
}

void MAYUMI::Window::Run()
{
    glfwMakeContextCurrent(m_Window);

    std::wstring firstProc = L"svchost.exe";
    std::wstring secondProc = L"cs2.exe";

    Process proc(firstProc, secondProc, 0x00001478, true);
    bool verified = false;

    while (!glfwWindowShouldClose(m_Window))
    {
        /*New frame*/
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        /*Handle window minimization*/
        if (GetAsyncKeyState(VK_INSERT))
        {
            if (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED))
            {
                glfwRestoreWindow(m_Window);
                glfwFocusWindow(m_Window);
            }
            else
            {
                glfwIconifyWindow(m_Window);
            }
            while(GetAsyncKeyState(VK_INSERT))
            { } /*Just wait for the user to let go of INSERT*/
        }

        /*Handle window movement*/
        if (CheatWindowShouldMove)
        {
            int RadarX, RadarY;
            glfwGetWindowSize(m_Window, &RadarX, &RadarY);

            POINT cursorPos{};
            GetCursorPos(&cursorPos);
            glfwSetWindowPos(m_Window, cursorPos.x - (int)HoldWindowPosition.x, cursorPos.y - (int)HoldWindowPosition.y);
        }

        /**/
        static SimpleAuth auth;

        if (verified && !auth.IsSignedIn())
            exit(1);

        if (WaitForGameWindow(proc))
        {
            if (VerificationWindow(proc, verified, auth))   //verify user and process state
            {
                if (!verified)  //more checks to make it annoying to crack
                    return;
                CheatWindow(proc, m_Window);
            }
        }

        /*Render*/
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_Window);
        glfwPollEvents();
        
    }
}

MAYUMI::Window::~Window()
{
    glfwDestroyWindow(m_Window);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
