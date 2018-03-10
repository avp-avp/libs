#include "RFProtocolSomeButton.h"


// 
static range_type g_timing_pause[7] =
{
	{ 5400, 18000 }, // Разделитель
	{ 140, 600 }, // Короткий
	{ 800, 1200 }, // Длинный
	{ 0,0 }
};

static range_type g_timing_pulse[8] =
{
	{ 5400, 12000 }, // Разделитель
	{ 140, 600 }, // Короткий
	{ 800, 1200 }, // Длинный
	{ 0,0 }
};


CRFProtocolSomeButton::CRFProtocolSomeButton()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 24, 2, "a")
{
}


CRFProtocolSomeButton::~CRFProtocolSomeButton()
{
}
