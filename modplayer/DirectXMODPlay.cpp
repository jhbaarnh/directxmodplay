// DirectXMODPlay.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "DirectXMODPlay.h"
#include <fstream.h>
#include <string.h>
#include <assert.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


// This is an example of an exported variable
DIRECTXMODPLAY_API int nDirectXMODPlay=0;

// This is an example of an exported function.
DIRECTXMODPLAY_API int fnDirectXMODPlay(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see DirectXMODPlay.h for the class definition
CDirectXMODPlay::CDirectXMODPlay()
{ 
	return; 
}

//#define NUMEVENTS 2
 
LPDIRECTSOUND               lpds;
LPDIRECTSOUNDBUFFER         lpdsbPrimary = NULL;
LPDIRECTSOUNDNOTIFY         lpdsNotify = NULL;

//WAVEFORMATEX                *pwfx;
//HMMIO                       hmmio;
//MMCKINFO                    mmckinfoData, mmckinfoParent;
//DSBPOSITIONNOTIFY           rgdsbpn[NUMEVENTS];
//HANDLE                      rghEvent[NUMEVENTS];

MODULE module;

DIRECTXMODPLAY_API BOOL ReadXM(LPCSTR fileName, LPMODULE module)
{
	ifstream modfile(fileName, ios::in | ios::binary | ios::nocreate, filebuf::openprot);
	CHAR buffer[50];
	BYTE test;
	DWORD size;
	WORD version;

	modfile.read(buffer, 17);
	buffer[17] = '\0';
	if (strcmp(buffer, "Extended Module: "))
		return FALSE;

	module->ModuleName = new CHAR[21];
	modfile.read(module->ModuleName, 20);
	module->ModuleName[20] = '\0';
	module->ModuleNameLength = strlen(module->ModuleName);

	modfile >> test;
	if (test != 0x1A)
		return FALSE;

	modfile.read(buffer, 20);
	modfile.read((char *)&version, sizeof version);
	modfile.read((char *)&size, sizeof size);
	modfile.read((char *)&module->OrderLength, sizeof module->OrderLength);
	modfile.read((char *)&module->RestartPosition, sizeof module->RestartPosition);
	modfile.read((char *)&module->nChannels, sizeof module->nChannels);
	modfile.read((char *)&module->nPatterns, sizeof module->nPatterns);
	modfile.read((char *)&module->nInstruments, sizeof module->nInstruments);

	modfile.read((char *)&version, sizeof version);
	modfile.read((char *)&module->Tempo, sizeof module->Tempo);
	modfile.read((char *)&module->BPM, sizeof module->BPM);
	module->Order = new BYTE[256];
	modfile.read(module->Order, 256);

	//module->nPatterns++;
	module->Patterns = new MODPATTERN[module->nPatterns + 1];

	for (int pattern = 0; pattern < module->nPatterns; pattern++)
	{
		modfile.read((char *)&size, sizeof size);
		modfile.read((char *)&test, sizeof test);
		if (test != 0)
			return FALSE;
	
		WORD channel;
		modfile.read((char *)&module->Patterns[pattern].nRows, sizeof module->Patterns[pattern].nRows);
		module->Patterns[pattern].Notes = new LPNOTE[module->nChannels];
		for (channel = 0; channel < module->nChannels; channel++)
		{
			module->Patterns[pattern].Notes[channel] = new NOTE[module->Patterns[pattern].nRows];
			memset(module->Patterns[pattern].Notes[channel], 0, module->Patterns[pattern].nRows * sizeof *module->Patterns[pattern].Notes[channel]);
		}

		WORD patternSize;
		modfile.read((char *)&patternSize, sizeof patternSize);
		LPBYTE patternBuffer = new BYTE[patternSize];
		modfile.read(patternBuffer, patternSize);

		WORD row = 0, note = 0;
		channel = 0;
		while (note < patternSize)
		{
			BYTE info = patternBuffer[note++];
			if (info & 0x80)
			{
				assert(row < module->Patterns[pattern].nRows);
				assert(channel < module->nChannels);
				if (info & 1)
					module->Patterns[pattern].Notes[channel][row].NoteValue = patternBuffer[note++];
				if (info & 2)
					module->Patterns[pattern].Notes[channel][row].Instrument = patternBuffer[note++];
				if (info & 4)
					module->Patterns[pattern].Notes[channel][row].Volume = patternBuffer[note++];
				if (info & 8)
					module->Patterns[pattern].Notes[channel][row].Effect = patternBuffer[note++];
				if (info & 16)
					module->Patterns[pattern].Notes[channel][row].EffectParameter = patternBuffer[note++];

				if (++channel == module->nChannels)
				{
					channel = 0;
					row++;
				}
			} 
			else
				return FALSE;
		}

		delete patternBuffer;
	}

	module->Patterns[module->nPatterns].nRows = 64;
	module->Patterns[module->nPatterns].Notes = new LPNOTE[module->nChannels];
	for (int channel = 0; channel < module->nChannels; channel++)
	{
		module->Patterns[module->nPatterns].Notes[channel] = new NOTE[module->Patterns[module->nPatterns].nRows];
		memset(module->Patterns[module->nPatterns].Notes[channel], 0, module->Patterns[module->nPatterns].nRows * sizeof *module->Patterns[module->nPatterns].Notes[channel]);
	}

	module->Instruments = new INSTRUMENT[module->nInstruments];
	for (int instrument = 0; instrument < module->nInstruments; instrument++)
	{
		modfile.read((char *)&size, sizeof size);
		module->Instruments[instrument].Name = new CHAR[23];
		modfile.read(module->Instruments[instrument].Name, 22);
		module->Instruments[instrument].Name[22] = '\0';
		module->Instruments[instrument].NameLength = strlen(module->Instruments[instrument].Name);
		modfile.read((char *)&module->Instruments[instrument].Type, sizeof module->Instruments[instrument].Type);
		modfile.read((char *)&module->Instruments[instrument].nSamples, sizeof module->Instruments[instrument].nSamples);

		// Sample header size is always present, even with no samples
		modfile.read((char *)&size, sizeof size);

		if (module->Instruments[instrument].nSamples > 0)
		{
			module->Instruments[instrument].nNoteSamples = 96;
			module->Instruments[instrument].NoteSamples = new BYTE[module->Instruments[instrument].nNoteSamples];
			modfile.read(module->Instruments[instrument].NoteSamples, module->Instruments[instrument].nNoteSamples);
			module->Instruments[instrument].VolumeEnvelopePoints = new BYTE[48];
			module->Instruments[instrument].PanningEnvelopePoints = new BYTE[48];
			modfile.read(module->Instruments[instrument].VolumeEnvelopePoints, 48);
			modfile.read(module->Instruments[instrument].PanningEnvelopePoints, 48);

			modfile.read((char *)&module->Instruments[instrument].nVolEnvPoints, sizeof module->Instruments[instrument].nVolEnvPoints);
			modfile.read((char *)&module->Instruments[instrument].nPanEnvPoints, sizeof module->Instruments[instrument].nPanEnvPoints);
			modfile.read((char *)&module->Instruments[instrument].VolumeSustainPoint, sizeof module->Instruments[instrument].VolumeSustainPoint);
			modfile.read((char *)&module->Instruments[instrument].VolumeLoopStartPoint, sizeof module->Instruments[instrument].VolumeLoopStartPoint);
			modfile.read((char *)&module->Instruments[instrument].VolumeLoopEndPoint, sizeof module->Instruments[instrument].VolumeLoopEndPoint);
			modfile.read((char *)&module->Instruments[instrument].PanningSustainPoint, sizeof module->Instruments[instrument].PanningSustainPoint);
			modfile.read((char *)&module->Instruments[instrument].PanningLoopStartPoint, sizeof module->Instruments[instrument].PanningLoopStartPoint);
			modfile.read((char *)&module->Instruments[instrument].PanningLoopEndPoint, sizeof module->Instruments[instrument].PanningLoopEndPoint);
			modfile.read((char *)&module->Instruments[instrument].VolumeType, sizeof module->Instruments[instrument].VolumeType);
			modfile.read((char *)&module->Instruments[instrument].PanningType, sizeof module->Instruments[instrument].PanningType);
			modfile.read((char *)&module->Instruments[instrument].VibratoType, sizeof module->Instruments[instrument].VibratoType);
			modfile.read((char *)&module->Instruments[instrument].VibratoSweep, sizeof module->Instruments[instrument].VibratoSweep);
			modfile.read((char *)&module->Instruments[instrument].VibratoDepth, sizeof module->Instruments[instrument].VibratoDepth);
			modfile.read((char *)&module->Instruments[instrument].VibratoRate, sizeof module->Instruments[instrument].VibratoRate);
			modfile.read((char *)&module->Instruments[instrument].VolumeFadeOut, sizeof module->Instruments[instrument].VolumeFadeOut);
			modfile.read(buffer, 22);

			module->Instruments[instrument].Samples = new SAMPLE[module->Instruments[instrument].nSamples];
			memset(module->Instruments[instrument].Samples, 0, module->Instruments[instrument].nSamples * sizeof *module->Instruments[instrument].Samples);

			int sample;
			for (sample = 0; sample < module->Instruments[instrument].nSamples; sample++)
			{
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].Length, sizeof module->Instruments[instrument].Samples[sample].Length);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].LoopStart, sizeof module->Instruments[instrument].Samples[sample].LoopStart);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].LoopLength, sizeof module->Instruments[instrument].Samples[sample].LoopLength);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].Volume, sizeof module->Instruments[instrument].Samples[sample].Volume);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].FineTune, sizeof module->Instruments[instrument].Samples[sample].FineTune);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].Type, sizeof module->Instruments[instrument].Samples[sample].Type);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].Pan, sizeof module->Instruments[instrument].Samples[sample].Pan);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].RelativeNote, sizeof module->Instruments[instrument].Samples[sample].RelativeNote);
				modfile.read((char *)&test, sizeof test);
				module->Instruments[instrument].Samples[sample].Name = new CHAR[23];
				modfile.read(module->Instruments[instrument].Samples[sample].Name, 22);
				module->Instruments[instrument].Samples[sample].Name[22] = '\0';
				module->Instruments[instrument].Samples[sample].NameLength = strlen(module->Instruments[instrument].Samples[sample].Name);

				if (module->Instruments[instrument].Samples[sample].Type & 3 == 0)
					module->Instruments[instrument].Samples[sample].Loop = FALSE;
				else if (module->Instruments[instrument].Samples[sample].Type & 3 == 1)
				{
					module->Instruments[instrument].Samples[sample].Loop = TRUE;
					module->Instruments[instrument].Samples[sample].Loop = SAMPLE_FORWARD_LOOP;
				}
				else if (module->Instruments[instrument].Samples[sample].Type & 3 == 2)
				{
					module->Instruments[instrument].Samples[sample].Loop = TRUE;
					module->Instruments[instrument].Samples[sample].Loop = SAMPLE_PINGPONG_LOOP;
				}

			}
			for (sample = 0; sample < module->Instruments[instrument].nSamples; sample++)
			{
				if (module->Instruments[instrument].Samples[sample].Length > 0)
				{
					module->Instruments[instrument].Samples[sample].Data = new CHAR[module->Instruments[instrument].Samples[sample].Length];
					modfile.read(module->Instruments[instrument].Samples[sample].Data, module->Instruments[instrument].Samples[sample].Length);
					
					if (module->Instruments[instrument].Samples[sample].Type & 16 == 16)
					{
						SHORT *curSample = (SHORT *)module->Instruments[instrument].Samples[sample].Data;
						SHORT *lastSample = (SHORT *)(module->Instruments[instrument].Samples[sample].Data + module->Instruments[instrument].Samples[sample].Length * sizeof *module->Instruments[instrument].Samples[sample].Data);
						SHORT oldSample = 0;
						SHORT newSample;
						curSample++;
						while (curSample < lastSample)
						{
							newSample = oldSample + *curSample;
							*curSample = newSample;
							oldSample = newSample;
							curSample++;
						}
					}
					else
					{
						CHAR *curSample = module->Instruments[instrument].Samples[sample].Data;
						CHAR *lastSample = module->Instruments[instrument].Samples[sample].Data + module->Instruments[instrument].Samples[sample].Length * sizeof *module->Instruments[instrument].Samples[sample].Data;
						CHAR oldSample = 0;
						CHAR newSample;
						curSample++;
						while (curSample < lastSample)
						{
							newSample = oldSample + *curSample;
							*curSample = newSample;
							oldSample = newSample;
							curSample++;
						}
					}
				}
			}
		}
	}

	return TRUE;
}

DIRECTXMODPLAY_API VOID DeInitPlayer(VOID)
{
	DSExit();

	for (int instrument = 0; instrument < module.nInstruments; instrument++)
	{
		for (int sample = 0; sample < module.Instruments[instrument].nSamples; sample++)
		{
			delete module.Instruments[instrument].Samples[sample].Name;
			delete module.Instruments[instrument].Samples[sample].Data;
		}

		delete module.Instruments[instrument].Name;

		if (module.Instruments[instrument].nSamples > 0)
		{
			delete module.Instruments[instrument].NoteSamples;
			delete module.Instruments[instrument].VolumeEnvelopePoints;
			delete module.Instruments[instrument].PanningEnvelopePoints;
		}
	}

	for (int pattern = 0; pattern <= module.nPatterns; pattern++)
	{
		for (int channel = 0; channel < module.nChannels; channel++)
			delete module.Patterns[pattern].Notes[channel];
		
		delete module.Patterns[pattern].Notes;
	}

	delete module.Patterns;
	delete module.ModuleName;
	delete module.Order;
	//delete module.TrackerName;
}


DIRECTXMODPLAY_API BOOL InitPlayer(HWND hwnd, GUID *pguid, LPCSTR modfile)
{
	if (!InitDSound(hwnd, pguid))
		return FALSE;

	if (!ReadXM(modfile, &module))	
		return FALSE;

	for (int instrument = 0; instrument < module.nInstruments; instrument++)
	{
		for (int sample = 0; sample < module.Instruments[instrument].nSamples; sample++)
		{
			if (!InitSampleSoundBuffers(lpds, &module.Instruments[instrument].Samples[sample]))
				return FALSE;
		}
	}
	
	return TRUE;
}

BOOL InitDSound(HWND hwnd, GUID *pguid)
{
	DSBUFFERDESC dsbdesc;

	// Create DirectSound
    if FAILED(DirectSoundCreate(pguid, &lpds, NULL)) 
        return FALSE;
 
    // Set co-op level
    if FAILED(lpds->SetCooperativeLevel(hwnd, DSSCL_PRIORITY))
        return FALSE;

    // Obtain primary buffer
    memset(&dsbdesc, 0, sizeof dsbdesc);
    dsbdesc.dwSize = sizeof dsbdesc;
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    if FAILED(lpds->CreateSoundBuffer(&dsbdesc, &lpdsbPrimary, NULL))
		return FALSE;

	// Set primary buffer format
    WAVEFORMATEX wfx;
    memset(&wfx, 0, sizeof wfx); 
    wfx.wFormatTag = WAVE_FORMAT_PCM; 
    wfx.nChannels = 2; 
    wfx.nSamplesPerSec = 44100; 
    wfx.wBitsPerSample = 16;     
	wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
 
    if FAILED(lpdsbPrimary->SetFormat(&wfx)) 
		return FALSE;

    return TRUE;
}



BOOL InitSampleSoundBuffers( 
        LPDIRECTSOUND lpDirectSound, 
		LPSAMPLE sample) 
{ 
    WAVEFORMATEX pcmwf; 
    DSBUFFERDESC dsbdesc; 

    // Set up wave format structure. 
    memset(&pcmwf, 0, sizeof pcmwf); 
    pcmwf.wFormatTag = WAVE_FORMAT_PCM; 
    pcmwf.nChannels = 2; 
    pcmwf.nSamplesPerSec = 8000; 
    pcmwf.wBitsPerSample = 8 + 8 * ((sample->Type & 16) >> 4);
    pcmwf.nBlockAlign = pcmwf.nChannels * pcmwf.wBitsPerSample / 8; 
    pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign; 

    // Set up DSBUFFERDESC structure. 
    memset(&dsbdesc, 0, sizeof dsbdesc); // Zero it out. 
    dsbdesc.dwSize = sizeof dsbdesc; 

    // Need default controls (pan, volume, frequency). 
    dsbdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC; 

    dsbdesc.dwBufferBytes = sample->Length; 
    dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf; 

    // Create buffer. 
    if FAILED(lpDirectSound->CreateSoundBuffer(&dsbdesc, &sample->SoundBuffer, NULL))
	{
        sample->SoundBuffer = NULL; 
        return FALSE; 
    } 

    return TRUE; 
}

VOID DSExit(VOID)
{
    if (lpdsNotify)
        lpdsNotify->Release();

	for (int instrument = 0; instrument < module.nInstruments; instrument++)
		for (int sample = 0; sample < module.Instruments[instrument].nSamples; sample++)
			if (module.Instruments[instrument].Samples[sample].SoundBuffer != NULL)
				module.Instruments[instrument].Samples[sample].SoundBuffer->Release();

	if (lpds)
        lpds->Release(); 
}
