#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

class Renderer
{
public:
	void Clear() const;
	void ClearColor(float r, float g, float b, float a) const;

	void DrawTriangles(VertexArray& va, IndexBuffer& ib, Shader& shader) const;
	void DrawLines(VertexArray& va, IndexBuffer& ib, Shader& shader) const;

private:
};

