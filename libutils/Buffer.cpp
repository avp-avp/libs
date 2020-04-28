#include "stdafx.h"
#include "Buffer.h"
#include "Exception.h"
#include "Serializable.h"

CBuffer::CBuffer(void)
{
	m_Buffer = m_BufferPtr = NULL;
	m_Size = m_MaxSize = 0;
}


CBuffer::CBuffer(char *Buffer, size_t Size)
{
	m_Buffer = m_BufferPtr = NULL;
	m_Size = m_MaxSize = 0;
	Alloc(Size);

	if (m_Buffer!=NULL)
		memcpy(m_Buffer, Buffer, Size);
}

CBuffer::~CBuffer(void)
{
	delete m_Buffer;
}

char *CBuffer::getBuffer()
{
	if (m_Buffer != m_BufferPtr && m_Buffer+m_Size != m_BufferPtr)
		throw CHaException(CHaException::ErrSerialize, "CBuffer::Serialize size mismatch");

	return m_Buffer;
}

void CBuffer::Alloc(size_t Size)
{
	if (m_MaxSize>=Size)
	{
		m_Size = Size;
		m_BufferPtr = m_Buffer;
		return;
	}

	if (m_Buffer)
		delete m_Buffer;

	m_MaxSize = m_Size = Size;
	m_BufferPtr = m_Buffer = new char[m_MaxSize];
}

size_t CBuffer::getSizeSize(size_t size)
{
	return sizeof(int32_t);
}

size_t CBuffer::getSizeString(const string &s)
{
	return getSizeSize(s.length()) + s.length();
}


void CBuffer::Serialize(int8_t &data, bool bSerialize)
{
	if (m_BufferPtr-m_Buffer+sizeof(char)>m_Size)
		throw CHaException(CHaException::ErrSerialize, "CBuffer::Serialize size mismatch");

	if (bSerialize)
	{
		*(m_BufferPtr++)=data;
	}
	else
	{
		data = *(m_BufferPtr++);
	}
}

void CBuffer::Serialize(int16_t &data, bool bSerialize)
{
	if (m_BufferPtr-m_Buffer+sizeof(int16_t)>m_Size)
		throw CHaException(CHaException::ErrSerialize, "CBuffer::Serialize size mismatch");

	if (bSerialize)
	{
		*((int16_t*)m_BufferPtr)=htons(data);
	}
	else
	{
		data = ntohs(*((int16_t*)m_BufferPtr));
	}

	m_BufferPtr+=sizeof(int16_t);
}

void CBuffer::Serialize(int32_t &data, bool bSerialize)
{
	if (m_BufferPtr-m_Buffer+sizeof(int32_t)>m_Size)
		throw CHaException(CHaException::ErrSerialize, "CBuffer::Serialize size mismatch");

	if (bSerialize)
	{
		int32_t t =htonl(data);
		memcpy(m_BufferPtr, &t, sizeof(t));
//		*((long*)m_BufferPtr)=t;
	}
	else
	{
		int32_t t;
		memcpy(&t, m_BufferPtr, sizeof(t));
		data = ntohl(t);
	}

	m_BufferPtr+=sizeof(int32_t);
}

void CBuffer::Serialize(string &data, bool bSerialize)
{
	size_t size = data.length();
	SerializeSize(size, bSerialize);

	if (bSerialize)
	{
		SerializeBinary((char*)data.c_str(), size, bSerialize);
	}
	else
	{
		char *buffer = new char[size];
		SerializeBinary(buffer, size, bSerialize);
		data.assign(buffer, size);
		delete []buffer;
	}
}

void CBuffer::SerializeSize(size_t &size, bool bSerialize)
{
	int32_t _size = size;
	Serialize(_size, bSerialize);
	size = _size;
}

void CBuffer::SerializeBinary(void *data, size_t size, bool bSerialize)
{
	if (!size)
		return;

	if (m_BufferPtr-m_Buffer+size>m_Size)
		throw CHaException(CHaException::ErrSerialize, "CBuffer::Serialize size mismatch");

	if (bSerialize)
	{
		memcpy(m_BufferPtr, data, size);
	}
	else
	{
		memcpy(data, m_BufferPtr, size);
	}

	m_BufferPtr+=size;
}

void CBuffer::Serialize(CSerializable &data, bool bSerialize, bool bAlloc)
{
	if (bAlloc)
	{
		Alloc(data.getSize());
	}
	
	data.Serialize(this, bSerialize);

	if (bAlloc)
	{
		if (m_BufferPtr-m_Buffer!=m_Size)
			throw CHaException(CHaException::ErrSerialize, "CBuffer::Serialize size mismatch. Size=%d, Serialized %s", m_Size, m_BufferPtr-m_Buffer);
	}
}
