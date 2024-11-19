#include "Shader.h"
#include "GL/glew.h"
#include <fstream>
#include <sstream>
#include <exception>
#include <iostream>

Shader::Shader(std::string filepath)
{
    ShaderProgramSource source = ParseShader(filepath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::Shader(std::string VertexSource, std::string FragmentSource)
{
    ShaderProgramSource source{ VertexSource, FragmentSource };
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    glDeleteProgram(m_RendererID);
}

void Shader::SetUniform1i(std::string uniformName, int value)
{
    glUniform1i(GetUniformLocation(uniformName), value);
}

void Shader::SetUniform1f(std::string uniformName, float value)
{
    glUniform1f(GetUniformLocation(uniformName), value);
}

void Shader::SetUniform4f(std::string uniformName, float v0, float v1, float v2, float v3)
{
    glUniform4f(GetUniformLocation(uniformName), v0, v1, v2, v3);
}

void Shader::SetUniformMat4f(const std::string& uniformName, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::Bind() const
{
    glUseProgram(m_RendererID);
}

void Shader::UnBind() const
{
    glUseProgram(0);
}

int Shader::GetUniformLocation(std::string uniformName) const
{
    //@todo caching

    int uniformLocation = glGetUniformLocation(m_RendererID, uniformName.c_str());
    if (uniformLocation == -1) 
    {
        std::cout << "[Message: couldn't find uniform " << uniformName << ']' << std::endl;
    }
    return uniformLocation;
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath) const
{
    std::ifstream stream(filepath);
    if (!stream)
        throw std::runtime_error("couldn't open ifstream " + filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType currentType = ShaderType::NONE;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                currentType = ShaderType::VERTEX;

            else if (line.find("fragment") != std::string::npos)
                currentType = ShaderType::FRAGMENT;
        }
        else if (currentType != ShaderType::NONE)
        {
            ss[(int)currentType] << line << '\n';
        }
    }

    return ShaderProgramSource{ ss[0].str(), ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source) const
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int compileStatus;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        throw std::runtime_error("Failed to compile shader!\n" + *message);

        glDeleteShader(id);
        return 0;
    }
    return id;
}

int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) const
{
    unsigned int program = glCreateProgram();

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    //could / should call glDetachShader
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;

}
