# CSV1

CSV1 is a learning project focused on creating a real-time data analysis and visualization system for a popular first-person shooter game. It explores techniques to extract live game data without modifying or interfering with the game's codebase.

### Features

- **Authentication**  
  The program is only directly accessible to authenticated users. Users authenticate via Firebase to gain access.

- **Radar**  
  ![](images/radar.png)

- **ESP (Extra Sensory Perception)**  
  ![](images/esp.png)

- **Aim assist**  
  ![](images/aim_assist.png)

## Statically Linked Libraries
- [GLFW](https://github.com/glfw/glfw)
- [GLEW](https://github.com/nigels-com/glew)
- [Firebase C++ SDK](https://github.com/firebase/firebase-cpp-sdk)

  **Note:** The Firebase C++ SDK's debug libraries are too large to be hosted on GitHub directly. You can download them from the official release page:

  - [Download Debug Libraries](https://github.com/firebase/firebase-cpp-sdk/releases/tag/v11.8.0)

  ### Installation Instructions:
  1. Download the libraries from the link above.
  2. Extract the contents and navigate to `libs/windows/vs2019/md/x86/Debug`.
  3. Move the files from that folder to the following directory in your project:
     ```
     /CSV1/src/vendor/firebase/libs/windows/VS2019/MD/x86/Debug
     ```

- [GLM](https://github.com/g-truc/glm)
- [ImGui](https://github.com/ocornut/imgui)
- [SOIL](https://github.com/littlstar/soil)
- [STB](https://github.com/nothings/stb)
