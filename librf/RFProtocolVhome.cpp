#include "RFProtocolVhome.h"

// 
static range_type g_timing_pause[7] =
{
	{ 12000, 18000 }, // �����������
	{ 250, 550 }, // ��������
	{ 1200, 1500 }, // �������
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 3500, 3500 },
	{ 250, 550 }, // ��������
	{ 1200, 1500 }, // �������
	{ 0,0 }
};


static const uint16_t g_transmit_data[] =
{
	400, 1200, 3600, 0,  // Pauses
	400, 1200, 12000, 0   // Pulses
};


CRFProtocolVhome::CRFProtocolVhome()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 25, 1, "a")
{
	m_Debug = true;
}


CRFProtocolVhome::~CRFProtocolVhome()
{
	SetTransmitTiming(g_transmit_data);
}


string CRFProtocolVhome::DecodePacket(const string&pkt)
{
	string packet = pkt, res;
	try 
	{
		if (packet.length() == 48)
		{
			if (packet[0] == 'c')
				packet = "B" + packet;
			if (packet[0] == 'b')
				packet = "C" + packet;
		}

		if (packet.length() == 49)
		{
			if (packet[48] == 'B')
				packet += "c";
			if (packet[48] == 'C')
				packet += "b";
		}
		else
			return "";

		for (unsigned int i = 0; i < packet.length() - 1; i += 2)
		{
			if (packet[i]=='[') break;
			string part = packet.substr(i, 2);
			if (part == "Bc")
				res += "0";
			else if (part == "Cb")
				res += "1";
			else
				return "";
		}
	} catch (std::exception ex) {
		throw;
	}
	return res;
}

string CRFProtocolVhome::DecodeData(const string& bits)
{
	if (bits.length()!=25 || bits2long(bits, 0, 4) != 7 || bits[24]!='0')
		return "";

	int addr = bits2long(bits, 4, 16);
	int cmd = bits2long(bits, 20, 4);

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "addr=%04x cmd=%d", addr, cmd);
	return buffer;
}
