#ifndef FORMATPLAYER_H
#define FORMATPLAYER_H

#include "stdafx.h"
#include "DirectXMODPlay.h"
#include "moduletimer.h"
#include "dxinterface.h"

namespace DXModPlay
{

class FormatPlayer
{
	public:
		FormatPlayer(ModuleTimer *Timer, DirectSoundInterface *DXInterface);

		virtual VOID PlayRow(VOID) = 0;
		virtual LPMODULE ReadModule(std::istream &Input) = 0;
		virtual BOOL InitPlay(VOID) = 0;
		virtual VOID SetVolume(UCHAR Volume) = 0;
		virtual UCHAR GetVolume(VOID) = 0;
		virtual BOOL CheckFrequencies(VOID) = 0;
		
	protected:
		MODULE module;
		ModuleTimer *Timer;
		DirectSoundInterface *DXInterface;
		UCHAR GlobalVolume;
};
}
#endif