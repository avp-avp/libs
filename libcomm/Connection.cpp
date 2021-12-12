#include "stdafx.h"
#include "Connection.h"
#include "../libutils/Buffer.h"

static int g_ConnectionCount=0;
typedef map<CConnection*, string> TConnMap;
TConnMap g_AllConnections;

CConnection::CConnection(void)
{
	m_Timeout = 0;
	m_AutoDelete = false;
	m_Connected = false;
	CLog::Default()->Printf(5, "CConnection(%p)", this);
	g_ConnectionCount++;
	g_AllConnections[this] = "";
}

CConnection::~CConnection(void)
{
	g_ConnectionCount--;
	CLog::Default()->Printf(g_ConnectionCount<30?5:3, "~CConnection(%p). Total %d conn", this, g_ConnectionCount);

	TConnMap::iterator i = g_AllConnections.find(this);
	if (i!=g_AllConnections.end()) g_AllConnections.erase(i);
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

void CConnection::Dump()
{
	for_each(TConnMap, g_AllConnections, i) {
		CLog::Default()->Printf(1, "Conn %p, %s", i->first, i->first->m_ConnectString.c_str());		
	}
}
