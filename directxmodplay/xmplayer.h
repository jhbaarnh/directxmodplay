#ifndef XMPLAYER_H
#define XMPLAYER_H

#include "stdafx.h"
#include "DirectXMODPlay.h"
#include "formatplayer.h"

typedef struct
{
	BOOL Amiga;
} XMEXTRA, *PXMEXTRA, NEAR *NPXMEXTRA, FAR *LPXMEXTRA;

typedef struct
{
	LPVOID VolumeEnvelopePoints;
	UCHAR nVolEnvPoints;
	LPVOID PanningEnvelopePoints;
	UCHAR nPanEnvPoints;
	UCHAR VolumeSustainPoint;
	UCHAR VolumeLoopStartPoint;
	UCHAR VolumeLoopEndPoint;
	UCHAR PanningSustainPoint;
	UCHAR PanningLoopStartPoint;
	UCHAR PanningLoopEndPoint;
	UCHAR VolumeType;
	UCHAR PanningType;
	UCHAR VibratoType;
	UCHAR VibratoSweep;
	UCHAR VibratoDepth;
	UCHAR VibratoRate;
	USHORT VolumeFadeOut;
} XMINSTRUMENT, *PXMINSTRUMENT, NEAR *NPXMINSTRUMENT, FAR *LPXMINSTRUMENT;

typedef struct
{
	DWORD LoopStart;
	DWORD LoopLength;
	BOOL Loop;
	UINT LoopType;
	UCHAR Type;
} XMSAMPLE, *PXMSAMPLE, NEAR *NPXMSAMPLE, FAR *LPXMSAMPLE;

// Internal stuff

#define SAMPLE_FORWARD_LOOP 0
#define SAMPLE_PINGPONG_LOOP 1


const WORD AmigaPeriodTable[12 * 8] = 
{
	907, 900, 894, 887, 881, 875, 868, 862, 856, 850, 844, 838, 832, 826, 820, 814, 
	808, 802, 796, 791, 785, 779, 774, 768, 762, 757, 752, 746, 741, 736, 730, 725, 
	720, 715, 709, 704, 699, 694, 689, 684, 678, 675, 670, 665, 660, 655, 651, 646, 
	640, 636, 632, 628, 623, 619, 614, 610, 604, 601, 597, 592, 588, 584, 580, 575, 
	570, 567, 563, 559, 555, 551, 547, 543, 538, 535, 532, 528, 524, 520, 516, 513, 
	508, 505, 502, 498, 494, 491, 487, 484, 480, 477, 474, 470, 467, 463, 460, 457
};


namespace DXModPlay
{
class XMPlayer : public FormatPlayer
{
	public:
		virtual VOID PlayRow(VOID);
		virtual LPMODULE ReadModule(std::istream &Input);
		virtual BOOL InitPlay(VOID);
		virtual VOID SetVolume(LONG Volume);
		virtual LONG GetVolume(VOID);
		virtual BOOL CheckFrequencies(VOID);

		XMPlayer(ModuleTimer *Timer, DirectSoundInterface *DXInterface) : FormatPlayer(Timer, DXInterface) { }
		~XMPlayer(VOID);

	protected:

		DOUBLE LinearPeriod(CHAR Note, CHAR FineTune);
		DOUBLE LinearFrequency(CHAR Note, CHAR FineTune);
		DOUBLE AmigaPeriod(CHAR Note, CHAR FineTune);
		DOUBLE AmigaFrequency(CHAR Note, CHAR FineTune);

		BOOL ReadXM(std::istream &Input, LPMODULE module);
		VOID DeInitPlayer(VOID);
		BOOL PlayInstrument(LPINSTRUMENT instrument, CHAR NoteValue, UCHAR Volume, USHORT channel);
};
}
#endif