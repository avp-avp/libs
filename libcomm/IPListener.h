#pragma once
#include "IPConnection.h"

class LIBCOMM_API CIPListener :
	public CIPConnection
{
public:
	CIPListener(void);
	virtual ~CIPListener(void);
	void Listen(int Port, string address="");
	virtual void OnSelect(CSupervisor *superviser);

};
