#include "DirectXMODPlay.h"
#include "dxinterface.h"

using namespace DXModPlay;

DirectSoundInterface::DirectSoundInterface(VOID)
{
	Initialised = FALSE;
	isPlaying = FALSE;
}

DirectSoundInterface::DirectSoundInterface(HWND hwnd)
{
	Initialised = FALSE;
	isPlaying = FALSE;

	InitDSound(hwnd);
}

DirectSoundInterface::DirectSoundInterface(HWND hwnd, UINT nSamples, UINT nChannels)
{
	Initialised = FALSE;
	isPlaying = FALSE;
	Init(hwnd, nSamples, nChannels);
}

DirectSoundInterface::~DirectSoundInterface(VOID)
{
	StopPlaying();
	DeInit();
	DeInitDSound();
}

BOOL DirectSoundInterface::DeInit(VOID)
{
	if (!Initialised)
		return FALSE;

	for (UINT sample = 0; sample < nUsedSamples; sample++)
	{
		for (UINT channel = 0; channel < nChannels; channel++)
			Samples[sample][channel]->Release();

		delete Samples[sample];
	}

	delete Samples;

	return TRUE;
}

BOOL DirectSoundInterface::Init(UINT nSamples, UINT nChannels)
{
	if (Initialised)
	{
		StopPlaying();
		DeInit();
	}

	Samples = new LPDIRECTSOUNDBUFFER*[nSamples];
	nTotalSamples = nSamples;
	nUsedSamples = 0;
	this->nChannels = nChannels;

	Initialised = TRUE;

	return TRUE;
}

BOOL DirectSoundInterface::Init(HWND hwnd, UINT nSamples, UINT nChannels)
{
	if (Initialised)
	{	
		StopPlaying();
		DeInit();
		DeInitDSound();
	}

	if (!InitDSound(hwnd))
		return FALSE;

	return Init(nSamples, nChannels);
}

BOOL DirectSoundInterface::DeInitDSound(VOID)
{
	SAFE_RELEASE(lpdsbPrimary);
	SAFE_RELEASE(lpds);

	return TRUE;
}

BOOL DirectSoundInterface::InitDSound(HWND hwnd)
{
	DSBUFFERDESC dsbdesc;

	// Create DirectSound
    if FAILED(DirectSoundCreate(NULL, &lpds, NULL)) 
	{
		DEBUGPRINT(1, "Could not create DirectSound");
        return FALSE;
	}

    // Set co-op level
	if FAILED(lpds->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)) 
	{
		DEBUGPRINT(1, "Could not set DirectSound co-op level");
        return FALSE;
	}

    // Obtain primary buffer
    memset(&dsbdesc, 0, sizeof dsbdesc);
    dsbdesc.dwSize = sizeof dsbdesc;
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if FAILED(lpds->CreateSoundBuffer(&dsbdesc, &lpdsbPrimary, NULL)) 
	{
		DEBUGPRINT(1, "Could not create primary DirectSound buffer");
		return FALSE;
	}

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
	{
		DEBUGPRINT(1, "Could not set primary buffer format");
		return FALSE;
	}

    return TRUE;
}

UINT DirectSoundInterface::AddSample(LPBYTE Data, UINT Length, UINT BitsPerSample)
{ 
	if (!Initialised || nUsedSamples >= nTotalSamples)
		throw "Uninitialised DS";

	UINT SampleNum = nUsedSamples;
	LPDIRECTSOUNDBUFFER *Sample = Samples[SampleNum];
	Sample = new LPDIRECTSOUNDBUFFER[nChannels];
	Samples[SampleNum] = Sample;

    WAVEFORMATEX pcmwf; 
    DSBUFFERDESC dsbdesc; 

    // Set up wave format structure. 
    memset(&pcmwf, 0, sizeof pcmwf); 
    pcmwf.wFormatTag = WAVE_FORMAT_PCM; 
	pcmwf.nChannels = 1;

    pcmwf.nSamplesPerSec = 8363;
//    pcmwf.wBitsPerSample = 8 * (((sample->Type & 16) >> 4) + 1);
	pcmwf.wBitsPerSample = BitsPerSample;
    pcmwf.nBlockAlign = pcmwf.nChannels * pcmwf.wBitsPerSample / 8; 
    pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign; 

	// Sample length in bytes must be DOUBLEd for 16-bit samples. 
	// NO! This is not true!
	int sampleTypeLengthFactor = 1; //pcmwf.wBitsPerSample / 8;

    // Set up DSBUFFERDESC structure. 
    memset(&dsbdesc, 0, sizeof dsbdesc); // Zero it out. 
    dsbdesc.dwSize = sizeof dsbdesc;

    // Need default controls (pan, volume, frequency). 
    dsbdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS ; 

	// Sample length in bytes
	DWORD sampleLen = Length * sampleTypeLengthFactor;

	dsbdesc.dwBufferBytes = sampleLen;
    dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf; 

    // Create buffer. 
    if FAILED(lpds->CreateSoundBuffer(&dsbdesc, &Sample[0], NULL))
		throw "Could not create secondary DirectSound buffer";

	// Fill buffer

	DWORD BufferBytes;
	LPVOID Buffer;

	if FAILED(Sample[0]->Lock(0, 0, &Buffer, &BufferBytes, NULL, NULL, DSBLOCK_ENTIREBUFFER))
		throw "Could not lock DirectSound buffer";

	assert(BufferBytes >= sampleLen);
	memcpy(Buffer, Data, sampleLen);

	if FAILED(Sample[0]->Unlock(Buffer, sampleLen, NULL, 0))
		throw "Could not unlock DirectSound buffer";

	for (UINT channel = 1; channel < nChannels; channel++)
		if FAILED(lpds->DuplicateSoundBuffer(Sample[channel - 1], &Sample[channel]))
			throw "Could not duplicate DirectSound buffer";

	nUsedSamples++;
    return (nUsedSamples - 1); 
}

BOOL DirectSoundInterface::StartPlaying(VOID)
{
	if (isPlaying)
		return FALSE;

	if (FAILED(lpdsbPrimary->Play(0, 0, DSBPLAY_LOOPING)))
		return FALSE;

	isPlaying = TRUE;
	return TRUE;
}

BOOL DirectSoundInterface::StopPlaying(VOID)
{
	if (!isPlaying)
		return FALSE;

	for (UINT sample = 0; sample < nUsedSamples; sample++)
		for (UINT channel = 0; channel < nChannels; channel++)
			if (FAILED(Samples[sample][channel]->Stop()))
				return FALSE;

	if (FAILED(lpdsbPrimary->Stop()))
		return FALSE;

	isPlaying = FALSE;
	return TRUE;
}

BOOL DirectSoundInterface::PlaySample(UINT SampleNum, UINT Channel, BOOL Loop)
{
	if (!isPlaying)
		return FALSE;

	DWORD status;
	if FAILED(Samples[SampleNum][Channel]->GetStatus(&status))
	{
		DEBUGPRINT(2, "Could not get status on Channel %u", Channel);
		return FALSE;
	}

	if (status & DSBSTATUS_PLAYING)
	{
		if FAILED(Samples[SampleNum][Channel]->SetCurrentPosition(0L))
		{
			DEBUGPRINT(2, "Could not set position on Channel %u", Channel);
			return FALSE;
		}
	}
	else
	{
		if (Loop)
		{
			DEBUGPRINT(6, "Playing sample with loop on Channel %u", Channel); 
			if FAILED(Samples[SampleNum][Channel]->Play(0, 0, DSBPLAY_LOOPING))
			{
				DEBUGPRINT(2, "Could not start loop play on Channel %u", Channel);
				return FALSE;
			}
		}
		else
		{
			DEBUGPRINT(6, "Playing sample without loop on Channel %u", Channel); 
			if FAILED(Samples[SampleNum][Channel]->Play(0, 0, 0))
			{
				DEBUGPRINT(2, "Could not start play on Channel %u", Channel);
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL DirectSoundInterface::StopSample(UINT SampleNum, UINT Channel)
{
	if FAILED(Samples[SampleNum][Channel]->Stop())
		return FALSE;
	else
		return TRUE;
}

BOOL DirectSoundInterface::SetPosition(UINT SampleNum, UINT Channel, DWORD Position)
{
	if (FAILED(Samples[SampleNum][Channel]->SetCurrentPosition(Position)))
		return FALSE;
	else
		return TRUE;
}

BOOL DirectSoundInterface::SetPan(UINT SampleNum, UINT Channel, DOUBLE Pan)
{
	// TrackerPan range is 0..255
	// Make pan range DBSPAN_LEFT..DSBPAN_RIGHT
	LONG FinalPan = (LONG)(Pan/256 * abs(DSBPAN_RIGHT - DSBPAN_LEFT) + DSBPAN_LEFT + 0.5);

	if (FAILED(Samples[SampleNum][Channel]->SetPan(FinalPan)))
		return FALSE;
	else
		return TRUE;
}

BOOL DirectSoundInterface::SetVolume(UINT SampleNum, UINT Channel, DOUBLE Volume)
{
	DEBUGPRINT(4, "Setting volume to %f", Volume);
	// TrackerVol range is 0..255
	// Make vol range DBSVOLUME_MIN..DBSVOLUME_MAX
	// DirectSound max volume is "normal" volume, so count downwards from max
	//LONG FinalVol = (LONG)(DSBVOLUME_MAX - (DOUBLE)Volume/256 * abs(DSBVOLUME_MAX - DSBVOLUME_MIN) + 0.5);
	LONG FinalVol = (LONG)(Volume/256 * abs(DSBVOLUME_MAX - DSBVOLUME_MIN) + 0.5 + DSBVOLUME_MIN);
	DEBUGPRINT(4, "Setting DX volume to %ld", FinalVol);

	if (FAILED(Samples[SampleNum][Channel]->SetVolume(FinalVol)))
		return FALSE;
	else
		return TRUE;
}

BOOL DirectSoundInterface::SetFrequency(UINT SampleNum, UINT Channel, DWORD Frequency)
{
	DEBUGPRINT(6, "SampleNum %u, Channel %u", SampleNum, Channel);
	DEBUGPRINT(6, "nUsedSamples: %u, nTotalSamples: %u", nUsedSamples, nTotalSamples);
	DEBUGPRINT(6, "Buffer: %p", Samples);
	DEBUGPRINT(6, "Buffer: %p", Samples[SampleNum]);
	DEBUGPRINT(6, "Buffer: %p", Samples[SampleNum][Channel]);
	if (FAILED(Samples[SampleNum][Channel]->SetFrequency(Frequency)))
		return FALSE;
	else
		return TRUE;
}
