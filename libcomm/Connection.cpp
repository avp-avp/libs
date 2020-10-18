#include "stdafx.h"
#include "Connection.h"
#include "../libutils/Buffer.h"

static int g_ConnectionCount=0;

CConnection::CConnection(void)
{
	m_Timeout = 0;
	m_AutoDelete = false;
	m_Connected = false;
	CLog::Default()->Printf(5, "CConnection(%p)", this);
	g_ConnectionCount++;
}

CConnection::~CConnection(void)
{
	g_ConnectionCount--;
	CLog::Default()->Printf(g_ConnectionCount<30?5:3, "~CConnection(%p). Total %d conn", this, g_ConnectionCount);
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

