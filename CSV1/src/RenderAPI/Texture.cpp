#include "Texture.h"

#include <iostream>

#include <Windows.h>
#include <wingdi.h>

#include "GL/glew.h"
#include "vendor/stb_image/stb_image.h"
#include "SOIL/SOIL.h"


Texture::Texture(const std::string& path, bool invertY)
	: m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), 
	m_Width(0), m_Height(0), m_BPP(0)
{
	/*Load image with SOIL*/
	m_LocalBuffer = SOIL_load_image(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
	glGenTextures(1, &m_RendererID);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);

	/*Image parameters*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	unsigned int texture = SOIL_create_OGL_texture(m_LocalBuffer, m_Width, m_Height, 4, m_RendererID, invertY ? SOIL_FLAG_INVERT_Y : 0);
	glBindTexture(GL_TEXTURE_2D, texture);

	if (m_LocalBuffer)
		SOIL_free_image_data(m_LocalBuffer);
	else
		std::cout << "[Message: couldn't load texture from \"" + path + "\"]\n";
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_RendererID);
}

void Texture::Bind(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void Texture::UnBind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
