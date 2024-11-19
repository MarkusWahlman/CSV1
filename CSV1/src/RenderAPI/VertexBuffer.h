#pragma once
class VertexBuffer
{
private: 
	unsigned int m_RendererID;
	unsigned int m_Size;

public:
	VertexBuffer(const void* data, unsigned int size, bool dynamic = false);
	~VertexBuffer();

	void ChangeData(const void* data, int offset = 0, unsigned int size = 0);	/*Offset and size are optional*/

	void Bind()		const;
	void UnBind()	const;
};

