#pragma once
#include "RFProtocol.h"
class CRFProtocolSomeButton :
	public CRFProtocol
{
public:
	CRFProtocolSomeButton();
	virtual ~CRFProtocolSomeButton();

	virtual string getName() { return "Button"; };
/*	virtual string DecodePacket(const string&);
	virtual string DecodeData(const string&);


	virtual string bits2timings(const string &bits);
	virtual string data2bits(const string &data);*/
};

