#include "VertexBuffer.h"
#include "GL\glew.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size, bool dynamic)
	: m_Size(size)
{
	glGenBuffers(1, &m_RendererID);
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_RendererID);
}

void VertexBuffer::ChangeData(const void* data, int offset, unsigned int size)
{
	unsigned int realSize = size ? size : m_Size;	/*If we have a size (not 0) then we use that, otherwise use m_Size*/
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferSubData(GL_ARRAY_BUFFER, offset, realSize, data);
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
}

void VertexBuffer::UnBind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}