#include "DirectXMODPlay.h"
#include "moduletimer.h"

VOID (*ModuleCallBack)(VOID);
BOOL Started;

using namespace DXModPlay;

VOID CALLBACK TimerCallBack(
  HWND hwnd,     // handle of window for timer messages
  UINT uMsg,     // WM_TIMER message
  UINT idEvent,  // timer identifier
  DWORD dwTime   // current system time
)
{
	DEBUGPRINT(6, "Timer tick");

	if (Started)
		(*ModuleCallBack)();

}


ModuleTimer::ModuleTimer(DOUBLE Hertz, VOID (*CallBack)(VOID))
{
	Initialised = FALSE;
	TimerID = 0;

	Init(CallBack, Hertz);
}

ModuleTimer::ModuleTimer(VOID)
{
	Initialised = FALSE;
	TimerID = 0;
}

BOOL ModuleTimer::DeInit(VOID)
{
	if (!Initialised)
		return FALSE;

	return KillTimer(NULL, TimerID);
}

ModuleTimer::~ModuleTimer(VOID)
{
	DeInit();
}

BOOL ModuleTimer::Stop(VOID)
{
	if (!Initialised)
		return FALSE;

	Started = FALSE;
	return TRUE;
}

BOOL ModuleTimer::Init(VOID (*CallBack)(VOID), DOUBLE Hertz)
{
	Init(CallBack);
	return SetSpeed(Hertz);
}

BOOL ModuleTimer::Init(VOID (*CallBack)(VOID))
{
	ModuleCallBack = CallBack;
	Initialised = TRUE;
	Started = FALSE;

	return TRUE;
}

BOOL ModuleTimer::Start(VOID)
{
	DEBUGPRINT(5, "Trying to start timer");

	if (!Initialised)
		return FALSE;

	Started = TRUE;
	return TRUE;
}

BOOL ModuleTimer::SetSpeed(DOUBLE Hertz)
{
	if (!Initialised)
		return FALSE;

	DeInit();

	DEBUGPRINT(5, "Setting Win32 timer with Hertz %f", Hertz);

	// BPM seems not to be "Beats per minute", but rather:  Hertz = 2 * BPM / 5, 
	// ie millisec timeout = 1000 / Hertz = 2500 / BPM
	TimerID = SetTimer(NULL, 1, (UINT)(1000 / Hertz + 0.5), TimerCallBack);
	if (TimerID == 0)
		return FALSE;

	return TRUE;
}
