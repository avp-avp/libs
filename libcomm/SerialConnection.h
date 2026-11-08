#pragma once
#include "Connection.h"

class CSupervisor;

/*
 * Serial port connection (Linux/POSIX termios).
 *
 * Connect string: <device>[:<baudrate>[:<format>[:<flow>]]]
 *   device   - /dev/ttyUSB0, /dev/ttyS1, /dev/ttyACM0 ...
 *   baudrate - 1200 ... 4000000, default 115200
 *   format   - <data bits><parity><stop bits>, e.g. 8N1, 7E1, 8O2. Default 8N1
 *   flow     - none (default) | rtscts | xonxoff
 *
 * Examples: "/dev/ttyUSB0:115200", "/dev/ttyS1:9600:8E1", "/dev/ttyACM0:57600:8N1:rtscts"
 *
 * The file descriptor is returned by getSocket(), so the connection can be
 * registered in CSupervisor exactly like CIPConnection.
 */
class LIBCOMM_API CSerialConnection :
	public CConnection
{
protected:
	SOCKET m_hCom;

	string m_Device;
	int m_Baudrate;
	int m_nBits;
	char m_Parity;
	int m_nStop;
	string m_Flow;

	void ParseConnectString(const string &ConnectString);
	void SetupPort();

public:
	CSerialConnection(SOCKET fd=INVALID_SOCKET, string Name="");
	virtual ~CSerialConnection(void);

	virtual void Connect(string ConnectString);
	virtual void Disconnect();
	virtual void Send(const char *Buffer, size_t Size);
	virtual void Recv(char *Buffer, size_t &Size, bool bWaitAll );
	virtual void SetTimeout(int Timeout);

	virtual SOCKET getSocket(){return m_hCom;};
	virtual string GetPeerName();
	virtual void OnSelect(CSupervisor *superviser);

	void Flush();
	int getBaudrate(){return m_Baudrate;};
	const string &getDevice(){return m_Device;};
};
