#include "stdafx.h"
#include "Connection.h"
#include "../libutils/Buffer.h"

CConnection::CConnection(void)
{
	m_Timeout = 0;
	m_AutoDelete = false;
	m_Connected = false;
	CLog::Default()->Printf(5, "CConnection(%p)", this);
}

CConnection::~CConnection(void)
{
	CLog::Default()->Printf(5, "~CConnection(%p)", this);
}

void CConnection::SetTimeout(int Timeout)
{
	m_Timeout = Timeout;
}

void CConnection::Reconnect()
{
	Disconnect();
	Connect(m_ConnectString);
}

void CConnection::SendBuffer(CBuffer &buffer)
{
	Send(buffer.getBuffer(), buffer.getSize());
}

