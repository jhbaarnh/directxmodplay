#ifndef MODULEPLAYER_H
#define MODULEPLAYER_H

#include "stdafx.h"
#include "DirectXMODPlay.h"
#include "moduletimer.h"
#include "dxinterface.h"
#include "formatplayer.h"

extern "C" int rateconv(WORD *in, int in_size, WORD *out, double fsin, double fgG, double fgK, int u, int d, 
						int L, double gain, int stereo, int little_endian);

namespace DXModPlay
{

class DIRECTXMODPLAY_API DirectXMODPlayer
{
	public:
		DirectXMODPlayer(VOID);
		~DirectXMODPlayer(VOID);
		BOOL Init(HWND hwnd);
		BOOL Play(VOID);
		BOOL Load(std::istream &Module);
		BOOL Stop(VOID);
		BOOL Pause(VOID);
		VOID SetVolume(LONG Volume);
		LONG GetVolume(VOID);

	protected:
		MODULE *module;
		HANDLE ThreadHandle;
		DWORD ThreadID;

		BOOL DeInit();
		BOOL InitModule(std::istream &Module);

		BOOL DownSample(VOID);

		ModuleTimer *Timer;
		DirectSoundInterface *DXInterface;
		FormatPlayer *Player;
};
}
#endif
