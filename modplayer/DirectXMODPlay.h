#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include <math.h>

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DIRECTXMODPLAY_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DIRECTXMODPLAY_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef DIRECTXMODPLAY_EXPORTS
//#define DIRECTXMODPLAY_API __declspec(dllexport)
#else
//#define DIRECTXMODPLAY_API __declspec(dllimport)
#endif
#define DIRECTXMODPLAY_API 

// This class is exported from the DirectXMODPlay.dll
class DIRECTXMODPLAY_API CDirectXMODPlay {
public:
	CDirectXMODPlay(void);
	// TODO: add your methods here.
};

extern DIRECTXMODPLAY_API int nDirectXMODPlay;

DIRECTXMODPLAY_API int fnDirectXMODPlay(void);
DIRECTXMODPLAY_API BOOL InitPlayer(HWND appInstance, LPCSTR modfile);
DIRECTXMODPLAY_API VOID DeInitPlayer(VOID);
DIRECTXMODPLAY_API BOOL PlayModule();
DIRECTXMODPLAY_API BOOL StopModule();

// Internal stuff

#define SAMPLE_FORWARD_LOOP 0
#define SAMPLE_PINGPONG_LOOP 1

const WORD AmigaPeriodTable[12*8] = {
	907, 900, 894, 887, 881, 875, 868, 862, 856, 850, 844, 838, 832, 826, 820, 814, 
	808, 802, 796, 791, 785, 779, 774, 768, 762, 757, 752, 746, 741, 736, 730, 725, 
	720, 715, 709, 704, 699, 694, 689, 684, 678, 675, 670, 665, 660, 655, 651, 646, 
	640, 636, 632, 628, 623, 619, 614, 610, 604, 601, 597, 592, 588, 584, 580, 575, 
	570, 567, 563, 559, 555, 551, 547, 543, 538, 535, 532, 528, 524, 520, 516, 513, 
	508, 505, 502, 498, 494, 491, 487, 484, 480, 477, 474, 470, 467, 463, 460, 457
};
/*
#define LINEAR_PERIOD(Note, FineTune) (10 * 12 * 16 * 4 - Note * 16 * 4 - FineTune / 2)
#define LINEAR_FREQUENCY(Note, FineTune) (8363 * pow(2,((6 * 12 * 16 * 4 - LINEAR_PERIOD(Note, FineTune)) / (12 * 16 * 4))));

#define AMIGA_PERIOD(Note, FineTune) { \
	int integer; \
	return (AmigaPeriodTable[(Note % 12) * 8 + FineTune / 16] * (1 - modf(FineTune / 16, &integer)) + \
            AmigaPeriodTable[(Note % 12) * 8 + FineTune / 16 + 1] * (modf(FineTune / 16, &integer)) * 16 / pow(2, (Note / 12))); \
	}

#define AMIGA_FREQUENCY(Note, FineTune) (8363 * 1712 / AMIGA_PERIOD(Note, FineTune))
*/
typedef struct 
{
	LPSTR Name;
	UINT NameLength;
	DWORD Length;
	BYTE FineTune;
	BYTE Volume;
	BYTE Pan;
	BYTE Type;
	BYTE RelativeNote;
	DWORD LoopStart;
	DWORD LoopLength;
	BOOL Loop;
	UINT LoopType;
	LPSTR Data;
	LPDIRECTSOUNDBUFFER SoundBuffer;
} SAMPLE, *PSAMPLE, NEAR *NPSAMPLE, FAR *LPSAMPLE;

typedef struct 
{
	LPSTR Name;
	UINT NameLength;
	BYTE Type;
	LPBYTE NoteSamples;
	UINT nNoteSamples;
	LPBYTE VolumeEnvelopePoints;
	BYTE nVolEnvPoints;
	LPBYTE PanningEnvelopePoints;
	BYTE nPanEnvPoints;
	BYTE VolumeSustainPoint;
	BYTE VolumeLoopStartPoint;
	BYTE VolumeLoopEndPoint;
	BYTE PanningSustainPoint;
	BYTE PanningLoopStartPoint;
	BYTE PanningLoopEndPoint;
	BYTE VolumeType;
	BYTE PanningType;
	BYTE VibratoType;
	BYTE VibratoSweep;
	BYTE VibratoDepth;
	BYTE VibratoRate;
	WORD VolumeFadeOut;
	WORD nSamples;
	LPSAMPLE Samples;
} INSTRUMENT, *PINSTRUMENT, NEAR *NPINSTRUMENT, FAR *LPINSTRUMENT;

typedef struct
{
	BYTE NoteValue;
	BYTE Instrument;
	BYTE Volume;
	BYTE Effect;
	BYTE EffectParameter;
} NOTE, *PNOTE, NEAR *NPNOTE, FAR *LPNOTE;

typedef struct 
{
	WORD nRows;
	LPNOTE *Notes;
} MODPATTERN, *PMODPATTERN, NEAR *NPMODPATTERN, FAR *LPMODPATTERN;

typedef struct 
{
	LPSTR ModuleName;
	UINT ModuleNameLength;
	LPSTR TrackerName;
	UINT TrackerNameLength;
	LPBYTE Order;
	WORD OrderLength;
	WORD RestartPosition;
	WORD nChannels;
	WORD Tempo;
	WORD BPM;
	BOOL Amiga;
	LPMODPATTERN Patterns;
	WORD nPatterns;
	WORD nInstruments;
	LPINSTRUMENT Instruments;
} MODULE, *PMODULE, NEAR *NPMODULE, FAR *LPMODULE;

BOOL ReadXM(LPCSTR fileName, LPMODULE module);
BOOL InitDSound(HWND hwnd, GUID *pguid);
BOOL InitSampleSoundBuffers(LPDIRECTSOUND lpDirectSound, LPSAMPLE sample);
VOID DSExit(VOID);
VOID PlayInstrument(LPINSTRUMENT instrument, BYTE NoteValue, BYTE Volume);
DWORD WINAPI PlayModuleThread(VOID);
