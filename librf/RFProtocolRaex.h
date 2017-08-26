﻿#pragma once
#include "RFProtocol.h"
class RFLIB_API CRFProtocolRaex :
	public CRFProtocol
{
public:
	CRFProtocolRaex();
	virtual ~CRFProtocolRaex();

	virtual string getName() { return "Raex"; };
	virtual string DecodePacket(const string&);
	virtual string DecodeData(const string&); // Преобразование бит в данные

											  // Кодирование
	virtual string bits2timings(const string &bits);
	virtual string data2bits(const string &data);
};

