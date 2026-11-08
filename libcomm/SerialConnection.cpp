#include "stdafx.h"
#include "SerialConnection.h"
#include "IPSupervisor.h"
#include "../libutils/Exception.h"
#include "../libutils/strutils.h"

#ifndef WIN32
	#include <unistd.h>
	#include <fcntl.h>
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <sys/select.h>
#endif

CSerialConnection::CSerialConnection(SOCKET fd, string Name)
{
	m_hCom = fd;
	m_Connected = (fd != INVALID_SOCKET);
	m_ConnectString = "@" + Name + "{" + itoa(fd) + "}";

	m_Baudrate = 115200;
	m_nBits = 8;
	m_Parity = 'N';
	m_nStop = 1;
	m_Flow = "none";
}

CSerialConnection::~CSerialConnection(void)
{
	CLog::Default()->Printf(5, "Delete %s serial connection %p", (m_hCom == INVALID_SOCKET ? "closed" : "active"), this);

	if (m_hCom != INVALID_SOCKET)
		Disconnect();
}

void CSerialConnection::ParseConnectString(const string &ConnectString)
{
	string_vector params;
	SplitString(ConnectString, ':', params);

	if (params.size() < 1 || params.size() > 4 || params[0].empty())
		throw CHaException(CHaException::ErrConnectStringError, "Bad connect string (%s). Expected <device>[:<baud>[:<8N1>[:<flow>]]]", ConnectString.c_str());

	m_Device = params[0];

	if (params.size() > 1 && !params[1].empty())
	{
		m_Baudrate = atoi(params[1].c_str());
		if (m_Baudrate <= 0)
			throw CHaException(CHaException::ErrConnectStringError, "Bad baudrate in connect string (%s)", ConnectString.c_str());
	}

	if (params.size() > 2 && !params[2].empty())
	{
		const string &fmt = params[2];
		if (fmt.length() != 3)
			throw CHaException(CHaException::ErrConnectStringError, "Bad port format '%s' in connect string (%s)", fmt.c_str(), ConnectString.c_str());

		m_nBits = fmt[0] - '0';
		m_Parity = toupper(fmt[1]);
		m_nStop = fmt[2] - '0';

		if (m_nBits < 5 || m_nBits > 8 || (m_Parity != 'N' && m_Parity != 'E' && m_Parity != 'O') || (m_nStop != 1 && m_nStop != 2))
			throw CHaException(CHaException::ErrConnectStringError, "Bad port format '%s' in connect string (%s)", fmt.c_str(), ConnectString.c_str());
	}

	if (params.size() > 3 && !params[3].empty())
	{
		m_Flow = params[3];
		if (m_Flow != "none" && m_Flow != "rtscts" && m_Flow != "xonxoff")
			throw CHaException(CHaException::ErrConnectStringError, "Bad flow control '%s' in connect string (%s)", m_Flow.c_str(), ConnectString.c_str());
	}
}

#ifdef WIN32

void CSerialConnection::SetupPort()
{
	throw CHaException(CHaException::ErrNotImplemented, "CSerialConnection is not implemented for WIN32");
}

void CSerialConnection::Connect(string ConnectString)
{
	throw CHaException(CHaException::ErrNotImplemented, "CSerialConnection is not implemented for WIN32");
}

void CSerialConnection::Disconnect()
{
	m_hCom = INVALID_SOCKET;
	m_Connected = false;
}

void CSerialConnection::Send(const char *Buffer, size_t Size)
{
	throw CHaException(CHaException::ErrNotImplemented, "CSerialConnection is not implemented for WIN32");
}

void CSerialConnection::Recv(char *Buffer, size_t &Size, bool bWaitAll)
{
	throw CHaException(CHaException::ErrNotImplemented, "CSerialConnection is not implemented for WIN32");
}

void CSerialConnection::Flush()
{
}

#else

static speed_t BaudToSpeed(int baud)
{
	switch (baud)
	{
	case 50: return B50;
	case 75: return B75;
	case 110: return B110;
	case 134: return B134;
	case 150: return B150;
	case 200: return B200;
	case 300: return B300;
	case 600: return B600;
	case 1200: return B1200;
	case 1800: return B1800;
	case 2400: return B2400;
	case 4800: return B4800;
	case 9600: return B9600;
	case 19200: return B19200;
	case 38400: return B38400;
	case 57600: return B57600;
	case 115200: return B115200;
	case 230400: return B230400;
#ifdef B460800
	case 460800: return B460800;
#endif
#ifdef B500000
	case 500000: return B500000;
#endif
#ifdef B576000
	case 576000: return B576000;
#endif
#ifdef B921600
	case 921600: return B921600;
#endif
#ifdef B1000000
	case 1000000: return B1000000;
#endif
#ifdef B1152000
	case 1152000: return B1152000;
#endif
#ifdef B1500000
	case 1500000: return B1500000;
#endif
#ifdef B2000000
	case 2000000: return B2000000;
#endif
#ifdef B2500000
	case 2500000: return B2500000;
#endif
#ifdef B3000000
	case 3000000: return B3000000;
#endif
#ifdef B3500000
	case 3500000: return B3500000;
#endif
#ifdef B4000000
	case 4000000: return B4000000;
#endif
	default:
		throw CHaException(CHaException::ErrConnectStringError, "Unsupported baudrate %d", baud);
	}
}

void CSerialConnection::SetupPort()
{
	termios tio;
	if (tcgetattr(m_hCom, &tio) != 0)
		throw CHaException(CHaException::ErrConnectError, "tcgetattr(%s) failed: %d (%s)", m_Device.c_str(), errno, strerror(errno));

	// Raw mode: no echo, no canonical processing, no signals, no CR/LF translation
	cfmakeraw(&tio);

	tio.c_cflag |= CLOCAL | CREAD;

	tio.c_cflag &= ~CSIZE;
	switch (m_nBits)
	{
	case 5: tio.c_cflag |= CS5; break;
	case 6: tio.c_cflag |= CS6; break;
	case 7: tio.c_cflag |= CS7; break;
	default: tio.c_cflag |= CS8; break;
	}

	switch (m_Parity)
	{
	case 'E':
		tio.c_cflag |= PARENB;
		tio.c_cflag &= ~PARODD;
		tio.c_iflag |= INPCK;
		break;
	case 'O':
		tio.c_cflag |= PARENB | PARODD;
		tio.c_iflag |= INPCK;
		break;
	default:
		tio.c_cflag &= ~PARENB;
		tio.c_iflag &= ~INPCK;
		break;
	}

	if (m_nStop == 2)
		tio.c_cflag |= CSTOPB;
	else
		tio.c_cflag &= ~CSTOPB;

	tio.c_cflag &= ~CRTSCTS;
	tio.c_iflag &= ~(IXON | IXOFF | IXANY);
	if (m_Flow == "rtscts")
		tio.c_cflag |= CRTSCTS;
	else if (m_Flow == "xonxoff")
		tio.c_iflag |= IXON | IXOFF;

	// Blocking read returns as soon as at least 1 byte is available.
	// Timeouts are handled with select() in Recv().
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	speed_t speed = BaudToSpeed(m_Baudrate);
	cfsetispeed(&tio, speed);
	cfsetospeed(&tio, speed);

	if (tcsetattr(m_hCom, TCSANOW, &tio) != 0)
		throw CHaException(CHaException::ErrConnectError, "tcsetattr(%s) failed: %d (%s)", m_Device.c_str(), errno, strerror(errno));

	tcflush(m_hCom, TCIOFLUSH);
}

void CSerialConnection::Connect(string ConnectString)
{
	if (m_hCom != INVALID_SOCKET)
		Disconnect();

	m_ConnectString = ConnectString;
	ParseConnectString(ConnectString);

	// O_NONBLOCK - don't wait for DCD on open
	m_hCom = open(m_Device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
	if (m_hCom == INVALID_SOCKET)
		throw CHaException(CHaException::ErrConnectError, "Cannot open %s: %d (%s)", m_Device.c_str(), errno, strerror(errno));

	try
	{
		// Exclusive access, like a TCP connection
		if (ioctl(m_hCom, TIOCEXCL) != 0)
			throw CHaException(CHaException::ErrConnectError, "Port %s is busy: %d (%s)", m_Device.c_str(), errno, strerror(errno));

		SetupPort();

		// Back to blocking mode, Recv() uses select() for timeouts
		int flags = fcntl(m_hCom, F_GETFL);
		if (flags == -1 || fcntl(m_hCom, F_SETFL, flags & ~O_NONBLOCK) == -1)
			throw CHaException(CHaException::ErrConnectError, "fcntl(%s) failed: %d (%s)", m_Device.c_str(), errno, strerror(errno));
	}
	catch (CHaException &)
	{
		close(m_hCom);
		m_hCom = INVALID_SOCKET;
		throw;
	}

	m_Connected = true;
	CLog::Default()->Printf(5, "Serial %s opened (fd=%d, %d %d%c%d, flow=%s)", m_Device.c_str(), m_hCom, m_Baudrate, m_nBits, m_Parity, m_nStop, m_Flow.c_str());
}

void CSerialConnection::Disconnect()
{
	CLog::Default()->Printf(5, "Disconnect %s serial connection %p/%d", (m_hCom == INVALID_SOCKET ? "closed" : "active"), this, m_hCom);

	if (m_hCom != INVALID_SOCKET)
	{
		ioctl(m_hCom, TIOCNXCL);
		close(m_hCom);
	}

	m_hCom = INVALID_SOCKET;
	m_Connected = false;
}

void CSerialConnection::Send(const char *Buffer, size_t Size)
{
	if (m_hCom == INVALID_SOCKET)
		throw CHaException(CHaException::ErrNotConnected, "Serial port %s is not open", m_ConnectString.c_str());

	size_t sent = 0;
	while (sent < Size)
	{
		ssize_t l = write(m_hCom, Buffer + sent, Size - sent);
		if (l < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
				continue;

			throw CHaException(CHaException::ErrSendError, "Error at write(%s): %d (%s)", m_ConnectString.c_str(), errno, strerror(errno));
		}
		sent += l;
	}
}

void CSerialConnection::Recv(char *Buffer, size_t &Size, bool bWaitAll)
{
	if (m_hCom == INVALID_SOCKET)
		throw CHaException(CHaException::ErrNotConnected, "Serial port %s is not open", m_ConnectString.c_str());

	size_t received = 0;

	do
	{
		if (m_Timeout > 0)
		{
			fd_set fd_recv;
			FD_ZERO(&fd_recv);
			FD_SET(m_hCom, &fd_recv);
			timeval t;
			t.tv_sec = m_Timeout;
			t.tv_usec = 0;

			int sel = select(m_hCom + 1, &fd_recv, NULL, NULL, &t);
			if (sel == SOCKET_ERROR)
			{
				if (errno == EINTR)
					continue;
				throw CHaException(CHaException::ErrRecvError, "Error at select(%s): %d (%s)", m_ConnectString.c_str(), errno, strerror(errno));
			}
			else if (sel == 0)
				break; // timeout - return what we have
		}

		ssize_t l = read(m_hCom, Buffer + received, Size - received);

		if (l < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
				continue;

			int err = errno;
			// EIO - device was unplugged (USB-serial adapters)
			Disconnect();
			throw CHaException(CHaException::ErrRecvError, "Error at read(%s): %d (%s)", m_ConnectString.c_str(), err, strerror(err));
		}

		if (l == 0)
		{
			// With VMIN=1 read() returns 0 only on hangup
			Disconnect();
			break;
		}

		received += l;
	}
	while (bWaitAll && received < Size);

	Size = received;
}

void CSerialConnection::Flush()
{
	if (m_hCom != INVALID_SOCKET)
		tcflush(m_hCom, TCIOFLUSH);
}

#endif

void CSerialConnection::SetTimeout(int Timeout)
{
	CConnection::SetTimeout(Timeout);
}

void CSerialConnection::OnSelect(CSupervisor *superviser)
{
	superviser->OnRecieveEx(this);
}

string CSerialConnection::GetPeerName()
{
	if (m_Device.length())
		return m_Device + ":" + itoa(m_Baudrate);
	else if (m_ConnectString.length())
		return m_ConnectString;
	else
		return "<SERIAL>";
}
