#ifndef DIRECTXMODPLAY_H
#define DIRECTXMODPLAY_H

#include "stdafx.h"

namespace DXModPlay
{
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DIRECTXMODPLAY_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DIRECTXMODPLAY_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef DIRECTXMODPLAY_EXPORTS
#define DIRECTXMODPLAY_API __declspec(dllexport)
#else
#define DIRECTXMODPLAY_API __declspec(dllimport)
#endif

#define DEBUGLEVEL 4

#if DEBUGLEVEL == 0
#define NDEBUG
#endif

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }

static inline void DEBUGPRINT(UINT dlev, LPSTR fstr, ...)
{
	va_list list;
	CHAR err[100];

	if(DEBUGLEVEL >= dlev) {
		va_start(list, fstr);
		vfprintf(stderr, fstr, list);
		fprintf(stderr, "\n");
		_vsnprintf(err, sizeof err - sizeof "\n", fstr, list);
		strcat(err, "\n");
		OutputDebugString(err);
		va_end(list);
	}
}

typedef struct 
{
	LPSTR Name;
	UINT NameLength;
	DWORD Length;
	CHAR FineTune;
	UCHAR Volume;
	UCHAR Pan;
	CHAR RelativeNote;
	UCHAR *Data;
	UINT BitsPerSample;
	UINT SampleNum;
	UINT UpSampleFactor;
	UINT DownSampleFactor;
		
	LPVOID ExtraData;
} SAMPLE, *PSAMPLE, NEAR *NPSAMPLE, FAR *LPSAMPLE;

typedef struct 
{
	LPSTR Name;
	UINT NameLength;
	UCHAR Type;
	USHORT nSamples;
	LPSAMPLE Samples;
	UCHAR *NoteSamples;
	UINT nNoteSamples;

	LPVOID ExtraData;
} INSTRUMENT, *PINSTRUMENT, NEAR *NPINSTRUMENT, FAR *LPINSTRUMENT;

typedef struct
{
	UCHAR NoteValue;
	UCHAR Instrument;
	UCHAR Volume;
	UCHAR Effect;
	UCHAR EffectParameter;
} NOTE, *PNOTE, NEAR *NPNOTE, FAR *LPNOTE;

typedef struct 
{
	USHORT nRows;
	LPNOTE *Notes;
} MODPATTERN, *PMODPATTERN, NEAR *NPMODPATTERN, FAR *LPMODPATTERN;

typedef struct
{
	LPSAMPLE lastSample;
	BOOL isPlaying;
} CHANNELINFO, *PCHANNELINFO, NEAR *NPCHANNELINFO, FAR *LPCHANNELINFO;

typedef struct
{
	LPSTR ModuleName;
	UINT ModuleNameLength;
	LPSTR TrackerName;
	UINT TrackerNameLength;
	UCHAR *Order;
	USHORT OrderLength;
	USHORT RestartPosition;
	USHORT nChannels;
	USHORT Tempo;
	USHORT BPM;
	LPMODPATTERN Patterns;
	USHORT nPatterns;
	USHORT nInstruments;
	LPINSTRUMENT Instruments;
	LPCHANNELINFO ChannelInfo;
	LPVOID ExtraData;
} MODULE, *PMODULE, NEAR *NPMODULE, FAR *LPMODULE;

}

#endif