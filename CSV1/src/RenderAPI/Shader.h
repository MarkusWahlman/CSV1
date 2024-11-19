#pragma once

#include <string>
#include "glm/glm.hpp"

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};


class Shader
{
public:
    Shader(std::string filepath);
    Shader(std::string VertexSource, std::string FragmentSource);
    ~Shader();

    void SetUniform1i(std::string uniformName, int value);
    void SetUniform1f(std::string uniformName, float value);
    void SetUniform4f(std::string uniformName, float v0, float v1, float v2, float v3);
    void SetUniformMat4f(const std::string& uniformName, const glm::mat4& matrix);

    void Bind()      const;
    void UnBind()    const;

private:
    unsigned int m_RendererID;

    int GetUniformLocation(std::string uniformName)                                      const;
    ShaderProgramSource ParseShader(const std::string& filepath)                         const;
    unsigned int CompileShader(unsigned int type, const std::string& source)             const;
    int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) const;
};

