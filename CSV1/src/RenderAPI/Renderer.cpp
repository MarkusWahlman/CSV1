#include "Renderer.h"

void Renderer::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::ClearColor(float r, float g, float b, float a) const
{
	glClearColor(r, g, b, a);
}

void Renderer::DrawTriangles(VertexArray& va, IndexBuffer& ib, Shader& shader) const
//
//We could have another DrawTriangle that takes in 3 points and a shader, and creates the index buffer and vertex array.
//
{
	shader.Bind();
	va.Bind();
	ib.Bind();
	
	glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawLines(VertexArray& va, IndexBuffer& ib, Shader& shader) const
{
	shader.Bind();
	va.Bind();
	ib.Bind();

	glDrawElements(GL_LINES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}
