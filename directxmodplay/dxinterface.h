#ifndef DXINTERFACE_H
#define DXINTERFACE_H

#include "stdafx.h"
#include "DirectXMODPlay.h"

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

namespace DXModPlay
{
class DirectSoundInterface
{
	public:
		DirectSoundInterface(HWND hwnd, UINT nSamples, UINT nChannels);
		DirectSoundInterface(HWND hwnd);
		DirectSoundInterface(VOID);
		~DirectSoundInterface(VOID);

		BOOL Init(UINT nSamples, UINT nChannels);
		BOOL Init(HWND hwnd, UINT nSamples, UINT nChannels);
		UINT AddSample(LPBYTE Data, UINT Length, UINT BitsPerSample);
		BOOL StartPlaying(VOID);
		BOOL StopPlaying(VOID);
		BOOL PlaySample(UINT SampleNum, UINT Channel, BOOL Loop);
		BOOL StopSample(UINT SampleNum, UINT Channel);
		BOOL SetPosition(UINT SampleNum, UINT Channel, DWORD Position);
		BOOL SetPan(UINT SampleNum, UINT Channel, DOUBLE Pan);
		BOOL SetVolume(UINT SampleNum, UINT Channel, DOUBLE Volume);
		BOOL SetFrequency(UINT SampleNum, UINT Channel, DWORD Frequency);

		static DWORD GetMaxFrequency(VOID)
		{
			return DSBFREQUENCY_MAX;
		}
	protected:
		BOOL Initialised;
		BOOL isPlaying;

		BOOL InitDSound(HWND hwnd);
		BOOL DeInitDSound(VOID);
		BOOL DeInit(VOID);

		LPDIRECTSOUND lpds;
		LPDIRECTSOUNDBUFFER lpdsbPrimary;

		UINT nChannels;
		UINT nTotalSamples, nUsedSamples;
		LPDIRECTSOUNDBUFFER **Samples;
};
}
#endif