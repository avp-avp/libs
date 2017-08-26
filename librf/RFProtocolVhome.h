#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolVhome :
	public CRFProtocol
{
public:
	CRFProtocolVhome();
	~CRFProtocolVhome();

	virtual string getName() { return "VHome"; };
	virtual string DecodePacket(const string&);
	virtual string DecodeData(const string& bits);

	// Кодирование
	//virtual string bits2timings(const string &bits);
	//virtual string data2bits(const string &data);
};

