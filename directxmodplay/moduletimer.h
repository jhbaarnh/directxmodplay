#ifndef MODULETIMER_H
#define MODULETIMER_H

#include "stdafx.h"
#include "DirectXMODPlay.h"

namespace DXModPlay
{
class ModuleTimer
{
	public:
		ModuleTimer(DOUBLE Hertz, VOID (*CallBack)(VOID));
		ModuleTimer(VOID);
		~ModuleTimer(VOID);

		BOOL Init(VOID (*CallBack)(VOID), DOUBLE Hertz);
		BOOL Init(VOID (*CallBack)(VOID));
		BOOL Start(VOID);
		BOOL SetSpeed(DOUBLE Hertz);
		BOOL Stop(VOID);

	protected:
		BOOL DeInit(VOID);
		UINT TimerID;
		BOOL Initialised;

};
}
#endif