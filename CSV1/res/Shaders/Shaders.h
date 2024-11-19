/* https://tomeko.net/online_tools/cpp_text_escape.php?lang=en */

inline std::string s_BasicShaderVertex =
	"#version 330 core\n"
	"\n"
	"layout(location = 0) in vec4 position;\n"
	"\n"
	"uniform mat4 u_MVP;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = u_MVP * position;\n"
	"};";

inline std::string s_BasicShaderFragment =
	"#version 330 core\n"
	"\n"
	"layout(location = 0) out vec4 color;\n"
	"\n"
	"uniform vec4 u_Color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tcolor = u_Color;\n"
	"};";

inline std::string s_TextureShaderVertex =
	"#version 330 core\n"
	"\n"
	"layout(location = 0) in vec4 position;\n"
	"layout(location = 1) in vec2 texCoord;\n"
	"\n"
	"out vec2 v_TexCoord;\n"
	"uniform mat4 u_MVP;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = u_MVP * position;\n"
	"\tv_TexCoord = texCoord;\n"
	"};";

inline std::string s_TextureShaderFragment =
	"#version 330 core\n"
	"\n"
	"layout(location = 0) out vec4 color;\n"
	"\n"
	"in vec2 v_TexCoord;\n"
	"\n"
	"uniform sampler2D u_Texture;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tvec4 texColor = texture(u_Texture, v_TexCoord);\n"
	"\tcolor = texColor;\n"
	"};";