#ifndef MODULEPLAYER_H
#define MODULEPLAYER_H

#include "stdafx.h"
#include "DirectXMODPlay.h"
#include "moduletimer.h"
#include "dxinterface.h"
#include "formatplayer.h"

extern "C" int rateconv(int *in, int in_size, int *out, double fsin, double fgG, double fgK, int u, int d, 
						int L, double gain, int stereo, int little_endian);

namespace DXModPlay
{			
	typedef enum 
	{
		MODULE_XM,
		MODULE_MOD,
		MODULE_S3M,
		MODULE_IT
	} ModuleType;
	const nModuleTypes = 4;

	typedef VOID (*ModuleCallBack)(VOID *Data);

	class DIRECTXMODPLAY_API DirectXMODPlayer
	{
		public:
			DirectXMODPlayer();
			~DirectXMODPlayer(VOID);
			BOOL Init(HWND hwnd);
			BOOL Play(VOID);
			BOOL Load(std::istream &Module, ModuleType type);
			BOOL Stop(VOID);
			BOOL Pause(VOID);
			VOID SetVolume(UCHAR Volume);
			UCHAR GetVolume(VOID);

			VOID SetPlayCallBack(ModuleCallBack CallBack, VOID *Data);
			VOID SetTickCallBack(ModuleCallBack CallBack, VOID *Data);

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
