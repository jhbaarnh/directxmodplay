#include "DirectXMODPlay.h"
#include "xmplayer.h"

using namespace DXModPlay;

LPMODULE XMPlayer::ReadModule(std::istream &Input)
{
	if (!ReadXM(Input, &module))
		return NULL;
	else
		return &module;
}

BOOL XMPlayer::ReadXM(std::istream &modfile, LPMODULE module)
{
	CHAR buffer[50];
	BYTE test;
	DWORD size;
	USHORT version;

	modfile.read(buffer, 17);
	buffer[17] = '\0';
	if (strcmp(buffer, "Extended Module: "))
		return FALSE;

	module->ModuleName = new CHAR[21];
	modfile.read(module->ModuleName, 20);
	module->ModuleName[20] = '\0';
	module->ModuleNameLength = strlen(module->ModuleName);

	modfile.read((CHAR *)&test, sizeof test);
	if (test != 0x1A)
		return FALSE;

	modfile.read(buffer, 20);
	buffer[20] = '\0';
	modfile.read((char *)&version, sizeof version);
	DEBUGPRINT(4, "Tracker name: %s, version: %x", buffer, version);
	DEBUGPRINT(4, "Module name: %s", module->ModuleName);

	modfile.read((char *)&size, sizeof size);
	modfile.read((char *)&module->OrderLength, sizeof module->OrderLength);
	modfile.read((char *)&module->RestartPosition, sizeof module->RestartPosition);
	modfile.read((char *)&module->nChannels, sizeof module->nChannels);
	modfile.read((char *)&module->nPatterns, sizeof module->nPatterns);
	modfile.read((char *)&module->nInstruments, sizeof module->nInstruments);

	DEBUGPRINT(2, "Number of channels: %u", module->nChannels);
	DEBUGPRINT(2, "Number of patterns: %u", module->nPatterns);
	DEBUGPRINT(2, "Number of instruments: %u", module->nInstruments);

	modfile.read((char *)&version, sizeof version);
	LPXMEXTRA XMData = new XMEXTRA;
	module->ExtraData = XMData;
	if ((version & 1) == 1)
		XMData->Amiga = FALSE;
	else
		XMData->Amiga = TRUE;
	modfile.read((CHAR *)&module->Tempo, sizeof module->Tempo);
	modfile.read((CHAR *)&module->BPM, sizeof module->BPM);
	module->Order = new BYTE[256];
	modfile.read((CHAR *) module->Order, 256);

	module->Patterns = new MODPATTERN[module->nPatterns + 1];

	for (int pattern = 0; pattern < module->nPatterns; pattern++)
	{
		modfile.read((char *)&size, sizeof size);
		modfile.read((char *)&test, sizeof test);
		if (test != 0)
			return FALSE;
	
		USHORT channel;
		modfile.read((char *)&module->Patterns[pattern].nRows, sizeof module->Patterns[pattern].nRows);
		module->Patterns[pattern].Notes = new LPNOTE[module->nChannels];
		for (channel = 0; channel < module->nChannels; channel++)
		{
			module->Patterns[pattern].Notes[channel] = new NOTE[module->Patterns[pattern].nRows];
			memset(module->Patterns[pattern].Notes[channel], 0, module->Patterns[pattern].nRows * sizeof *module->Patterns[pattern].Notes[channel]);
		}

		USHORT patternSize;
		modfile.read((char *)&patternSize, sizeof patternSize);
		UCHAR *patternBuffer = new UCHAR[patternSize];
		modfile.read((CHAR *)patternBuffer, patternSize);

		USHORT row = 0, note = 0;
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
			{
				module->Patterns[pattern].Notes[channel][row].NoteValue = info;
				module->Patterns[pattern].Notes[channel][row].Instrument = patternBuffer[note++];
				module->Patterns[pattern].Notes[channel][row].Volume = patternBuffer[note++];
				module->Patterns[pattern].Notes[channel][row].Effect = patternBuffer[note++];
				module->Patterns[pattern].Notes[channel][row].EffectParameter = patternBuffer[note++];
			}
		}

		delete patternBuffer;
	}

	// standard "empty pattern"
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
		// According do XM docs, Type should always be 0, but this does not seem to be the case
		//if (module->Instruments[instrument].Type != 0)
		//	return FALSE;

		modfile.read((char *)&module->Instruments[instrument].nSamples, sizeof module->Instruments[instrument].nSamples);
		DEBUGPRINT(4, "Instrument %u, name: %s, nof samples: %u", 
			instrument, module->Instruments[instrument].Name,
			module->Instruments[instrument].nSamples);
		
		// Sample header size is always present, even with no samples
		modfile.read((char *)&size, sizeof size);

		if (module->Instruments[instrument].nSamples > 0)
		{
			module->Instruments[instrument].nNoteSamples = 96;
			module->Instruments[instrument].NoteSamples = new UCHAR[module->Instruments[instrument].nNoteSamples];
			modfile.read((CHAR *)module->Instruments[instrument].NoteSamples, module->Instruments[instrument].nNoteSamples);
			LPXMINSTRUMENT XMInstrument = new XMINSTRUMENT;
			module->Instruments[instrument].ExtraData = XMInstrument;
			XMInstrument->VolumeEnvelopePoints = new USHORT[24];
			XMInstrument->PanningEnvelopePoints = new USHORT[24];
			modfile.read((CHAR *)XMInstrument->VolumeEnvelopePoints, 48);
			modfile.read((CHAR *)XMInstrument->PanningEnvelopePoints, 48);

			modfile.read((CHAR *)&XMInstrument->nVolEnvPoints, sizeof XMInstrument->nVolEnvPoints);
			modfile.read((CHAR *)&XMInstrument->nPanEnvPoints, sizeof XMInstrument->nPanEnvPoints);
			
			DEBUGPRINT(4, "Nof volume env points: %u, nof panning env points: %u",
				XMInstrument->nVolEnvPoints, XMInstrument->nPanEnvPoints);

			modfile.read((CHAR *)&XMInstrument->VolumeSustainPoint, sizeof XMInstrument->VolumeSustainPoint);
			modfile.read((CHAR *)&XMInstrument->VolumeLoopStartPoint, sizeof XMInstrument->VolumeLoopStartPoint);
			modfile.read((CHAR *)&XMInstrument->VolumeLoopEndPoint, sizeof XMInstrument->VolumeLoopEndPoint);
			modfile.read((CHAR *)&XMInstrument->PanningSustainPoint, sizeof XMInstrument->PanningSustainPoint);
			modfile.read((CHAR *)&XMInstrument->PanningLoopStartPoint, sizeof XMInstrument->PanningLoopStartPoint);
			modfile.read((CHAR *)&XMInstrument->PanningLoopEndPoint, sizeof XMInstrument->PanningLoopEndPoint);
			modfile.read((CHAR *)&XMInstrument->VolumeType, sizeof XMInstrument->VolumeType);
			modfile.read((CHAR *)&XMInstrument->PanningType, sizeof XMInstrument->PanningType);
			modfile.read((CHAR *)&XMInstrument->VibratoType, sizeof XMInstrument->VibratoType);
			modfile.read((CHAR *)&XMInstrument->VibratoSweep, sizeof XMInstrument->VibratoSweep);
			modfile.read((CHAR *)&XMInstrument->VibratoDepth, sizeof XMInstrument->VibratoDepth);
			modfile.read((CHAR *)&XMInstrument->VibratoRate, sizeof XMInstrument->VibratoRate);
			modfile.read((CHAR *)&XMInstrument->VolumeFadeOut, sizeof XMInstrument->VolumeFadeOut);
			
			DEBUGPRINT(8, "Volume fade out: %u", (UINT) XMInstrument->VolumeFadeOut);

			// The XM doc says size 11, but it seems to be 11 words => 22 bytes
			modfile.read(buffer, 22);

			module->Instruments[instrument].Samples = new SAMPLE[module->Instruments[instrument].nSamples];
			memset(module->Instruments[instrument].Samples, 0, module->Instruments[instrument].nSamples * sizeof *module->Instruments[instrument].Samples);

			int sample;
			for (sample = 0; sample < module->Instruments[instrument].nSamples; sample++)
			{
				LPXMSAMPLE XMSample = new XMSAMPLE;
				module->Instruments[instrument].Samples[sample].ExtraData = XMSample;

				modfile.read((char *)&module->Instruments[instrument].Samples[sample].Length, sizeof module->Instruments[instrument].Samples[sample].Length);
				modfile.read((CHAR *)&XMSample->LoopStart, sizeof XMSample->LoopStart);
				modfile.read((CHAR *)&XMSample->LoopLength, sizeof XMSample->LoopLength);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].Volume, sizeof module->Instruments[instrument].Samples[sample].Volume);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].FineTune, sizeof module->Instruments[instrument].Samples[sample].FineTune);
				modfile.read((CHAR *)&XMSample->Type, sizeof XMSample->Type);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].Pan, sizeof module->Instruments[instrument].Samples[sample].Pan);
				modfile.read((char *)&module->Instruments[instrument].Samples[sample].RelativeNote, sizeof module->Instruments[instrument].Samples[sample].RelativeNote);
				
				modfile.read((char *)&test, sizeof test);
				module->Instruments[instrument].Samples[sample].Name = new CHAR[23];
				modfile.read(module->Instruments[instrument].Samples[sample].Name, 22);
				module->Instruments[instrument].Samples[sample].Name[22] = '\0';
				module->Instruments[instrument].Samples[sample].NameLength = strlen(module->Instruments[instrument].Samples[sample].Name);

				if ((XMSample->Type & 3) == 0)
					XMSample->Loop = FALSE;
				else if ((XMSample->Type & 3) == 1)
				{
					XMSample->Loop = TRUE;
					XMSample->LoopType = SAMPLE_FORWARD_LOOP;
				}
				else if ((XMSample->Type & 3) == 2)
				{
					XMSample->Loop = TRUE;
					XMSample->LoopType = SAMPLE_PINGPONG_LOOP;
				}

			}
			for (sample = 0; sample < module->Instruments[instrument].nSamples; sample++)
			{
				if (module->Instruments[instrument].Samples[sample].Length > 0)
				{
					DEBUGPRINT(4, "Instrument %u, sample %u name: %s", instrument, sample,
						module->Instruments[instrument].Samples[sample].Name);
					DEBUGPRINT(4, "Volume: %u, Pan: %u, FineTune: %d, RelativeNote: %d", 
					(UINT)module->Instruments[instrument].Samples[sample].Volume,
					(UINT)module->Instruments[instrument].Samples[sample].Pan,
					(INT)module->Instruments[instrument].Samples[sample].FineTune,
					(INT)module->Instruments[instrument].Samples[sample].RelativeNote);

					module->Instruments[instrument].Samples[sample].Data = new UCHAR[module->Instruments[instrument].Samples[sample].Length];
					modfile.read((CHAR *)module->Instruments[instrument].Samples[sample].Data, module->Instruments[instrument].Samples[sample].Length);

					// Convert samples from offsets to absolute values
					LPXMSAMPLE XMSample = (LPXMSAMPLE) module->Instruments[instrument].Samples[sample].ExtraData;
					if ((XMSample->Type & 16) > 0)
					{
						DEBUGPRINT(4, "Instrument %d, sample %d is 16-bit", instrument, sample);

						SHORT *curSample = (SHORT *)module->Instruments[instrument].Samples[sample].Data;
						SHORT *lastSample = (SHORT *)(module->Instruments[instrument].Samples[sample].Data + module->Instruments[instrument].Samples[sample].Length * sizeof *module->Instruments[instrument].Samples[sample].Data);
						SHORT oldSample = 0;
						SHORT newSample;
						while (curSample < lastSample)
						{
							newSample = oldSample + *curSample;
							*curSample++ = oldSample = newSample;
						}

						module->Instruments[instrument].Samples[sample].BitsPerSample = 16;
					}
					else
					{
						DEBUGPRINT(4, "Instrument %d, sample %d is 8-bit", instrument, sample);

						// Convert sample to 16-bit
						// Seems like DirectSound does not do this correctly

						UCHAR *sample16 = new UCHAR[module->Instruments[instrument].Samples[sample].Length * 2];
						SHORT *convert = (SHORT *)sample16;
						CHAR *curSample = (CHAR *)module->Instruments[instrument].Samples[sample].Data;
						CHAR *lastSample = (CHAR *)module->Instruments[instrument].Samples[sample].Data + module->Instruments[instrument].Samples[sample].Length * sizeof *module->Instruments[instrument].Samples[sample].Data;
						CHAR oldSample = 0;
						CHAR newSample;

						while (curSample < lastSample)
						{
							newSample = oldSample + *curSample;
							*curSample = oldSample = newSample;

							// Scale values to 16-bit ranges
							*convert++ = (SHORT)*curSample++ * 256;
						}

						delete module->Instruments[instrument].Samples[sample].Data;
						module->Instruments[instrument].Samples[sample].Data = sample16;
						module->Instruments[instrument].Samples[sample].Length *= 2;
		
						// Set sample 16-bit
						XMSample->Type |= 16;
						module->Instruments[instrument].Samples[sample].BitsPerSample = 16;
					}
					
					if (XMSample->LoopType == SAMPLE_PINGPONG_LOOP)
					{
						DWORD sampleLen = module->Instruments[instrument].Samples[sample].Length;
						LPBYTE Buffer = new BYTE[sampleLen * 2];
						memcpy(Buffer, module->Instruments[instrument].Samples[sample].Data, sampleLen);
						if (module->Instruments[instrument].Samples[sample].BitsPerSample == 16)
						{
							for (DWORD i = 0; i < sampleLen / 2; i++)
								((WORD *)Buffer)[sampleLen - i - 1] = 
									((WORD *)module->Instruments[instrument].Samples[sample].Data)[i];
						}
						else
						{
							_strrev((CHAR *)module->Instruments[instrument].Samples[sample].Data);
							memcpy(Buffer + sampleLen, module->Instruments[instrument].Samples[sample].Data, sampleLen);
						}
						delete module->Instruments[instrument].Samples[sample].Data;
						module->Instruments[instrument].Samples[sample].Data = Buffer;
						module->Instruments[instrument].Samples[sample].Length *= 2;
					}

					module->Instruments[instrument].Samples[sample].DownSampleFactor = 1;
					module->Instruments[instrument].Samples[sample].UpSampleFactor = 1;
				}
			}
		}
	}

	return TRUE;
}

VOID XMPlayer::PlayRow(VOID)
{
	static USHORT curPattern = 0;
	static USHORT curRow = 0;

	NOTE curNote;

	for (int channel = 0; channel < module.nChannels; channel++)
	{
		assert(module.Order[curPattern] < module.nPatterns);
		curNote = module.Patterns[module.Order[curPattern]].Notes[channel][curRow];

		if (curNote.NoteValue > 0)
		{
			DEBUGPRINT(6, "Playing note with note value %u, instrument %u", curNote.NoteValue, curNote.Instrument);
			if (curNote.NoteValue == 97)
			{
				if (module.ChannelInfo[channel].isPlaying)
				{
					DEBUGPRINT(6, "Stop note found on channel %u", channel);
					DXInterface->StopSample(module.ChannelInfo[channel].lastSample->SampleNum, channel);
					DXInterface->SetPosition(module.ChannelInfo[channel].lastSample->SampleNum, channel, 0L);

					module.ChannelInfo[channel].isPlaying = FALSE;
				}
			}
			else
			{
				DEBUGPRINT(6, "Nof samples: %u", module.Instruments[curNote.Instrument - 1].nSamples);
				assert(curNote.NoteValue < 97);
				assert(curNote.Instrument > 0);
				assert(curNote.Instrument <= module.nInstruments);
				assert(module.Instruments[curNote.Instrument - 1].nSamples > 0);
				if (!PlayInstrument(&module.Instruments[curNote.Instrument - 1], curNote.NoteValue, curNote.Volume, channel))
						DEBUGPRINT(6, "Could not play instrument %u on channel %u", curNote.Instrument, channel);
			}
		}

		// Implement effects here
		switch (curNote.Effect)
		{
			// Set speed/BPM
			case 0xF:
			{
				if (curNote.EffectParameter < 0x20)
				{
					DEBUGPRINT(2, "Setting tempo to %u", curNote.EffectParameter);
					module.Tempo = curNote.EffectParameter;
					break;
				}
				else
				{
					DEBUGPRINT(2, "Setting BPM to %u", curNote.EffectParameter);
					module.BPM = curNote.EffectParameter;
					Timer->SetSpeed(2 * module.BPM / 5);
					break;
				}
			}
			default:
				break;
		}
			
	}
		
	curRow++;
	if (curRow == module.Patterns[module.Order[curPattern]].nRows) 
	{
		curRow = 0;
		curPattern++;
		if (curPattern == module.OrderLength)
			curPattern = 0;
	}
}


inline DOUBLE XMPlayer::LinearPeriod(CHAR Note, CHAR FineTune) 
{
	return (DOUBLE)(10 * 12 * 16 * 4 - Note * 16 * 4 - FineTune / 2);
}

inline DOUBLE XMPlayer::LinearFrequency(CHAR Note, CHAR FineTune) 
{
	return (8363 * pow((DOUBLE) 2, (6 * 12 * 16 * 4 - LinearPeriod(Note, FineTune)) / (DOUBLE)(12 * 16 * 4)));
}

inline DOUBLE XMPlayer::AmigaPeriod(CHAR Note, CHAR FineTune) 
{ 
	DOUBLE integer; 
	assert((Note % 12) * 8 + (DOUBLE)FineTune / 16 < 95);
	return (AmigaPeriodTable[(Note % 12) * 8 + FineTune / 16] * (1 - modf((DOUBLE) FineTune / 16, &integer)) + 
			AmigaPeriodTable[(Note % 12) * 8 + FineTune / 16 + 1] * (modf((DOUBLE) FineTune / 16, &integer)) * 16 / pow((DOUBLE)2, (Note / 12))); 
}

inline DOUBLE XMPlayer::AmigaFrequency(CHAR Note, CHAR FineTune) 
{
	return (8363 * 1712 / AmigaPeriod(Note, FineTune));
}

LONG XMPlayer::GetVolume(VOID)
{
	return GlobalVolume;
}

VOID XMPlayer::SetVolume(LONG Volume)
{
	GlobalVolume = Volume;
}

BOOL XMPlayer::PlayInstrument(LPINSTRUMENT instrument, CHAR NoteValue, UCHAR Volume, USHORT channel)
{
	DEBUGPRINT(9, "Note sample: %u", instrument->NoteSamples[(UINT)NoteValue - 1]);
	LPSAMPLE NoteSample = &instrument->Samples[instrument->NoteSamples[(UINT)NoteValue - 1]];
	DEBUGPRINT(9, "Relative note: %d, Notevalue: %d, FineTune: %d", NoteSample->RelativeNote, NoteValue, NoteSample->FineTune);

	CHAR RealNote = NoteSample->RelativeNote + NoteValue;
	DOUBLE Frequency;

	if (((LPXMEXTRA)module.ExtraData)->Amiga)
		Frequency = AmigaFrequency(RealNote, NoteSample->FineTune) * NoteSample->UpSampleFactor / NoteSample->DownSampleFactor;
	else
		Frequency = LinearFrequency(RealNote, NoteSample->FineTune) * NoteSample->UpSampleFactor / NoteSample->DownSampleFactor;

	// Volume in volume column should always be adjusted
	Volume += 0x10;

	LONG EnvelopeVolume = 64;
	LONG EnvelopePan = 32;
	LPXMINSTRUMENT XMData = (LPXMINSTRUMENT)instrument->ExtraData;
	DEBUGPRINT(8, "Global volume: %u, Sample vol: %u, Volume: %u, Fadeout: %u", (UINT)GlobalVolume, (UINT)NoteSample->Volume, (UINT)Volume, (UINT)XMData->VolumeFadeOut);
	DOUBLE TrackerVol = ((DOUBLE)EnvelopeVolume / 64) * ((DOUBLE)GlobalVolume / 64) * ((DOUBLE)NoteSample->Volume / 128) * ((DOUBLE)Volume / 64);
	
	// Envelope volume/pan and VolumeFadeOut should only be used when an envelope is in use for the currently playing instrument sample
	//if (XMData->VolumeFadeOut > 0)
	//	TrackerVol *= XMData->VolumeFadeOut / 65536;

	DOUBLE TrackerPan = (DOUBLE)(NoteSample->Pan + (EnvelopePan - 32) * (128 - abs(NoteSample->Pan - 128)) / 32); 

	DEBUGPRINT(6, "Sample num: %u", NoteSample->SampleNum);
	DEBUGPRINT(6, "Setting frequency, panning and volume");

	if (!DXInterface->SetFrequency(NoteSample->SampleNum, channel, (DWORD) (Frequency + 0.5)))
	{
		DEBUGPRINT(2, "Could not set frequency on channel %u", channel);
		return FALSE;
	}

	if (!DXInterface->SetVolume(NoteSample->SampleNum, channel, TrackerVol))
	{
		DEBUGPRINT(2, "Could not set volume on channel %u", channel);
		return FALSE;
	}

	if (!DXInterface->SetPan(NoteSample->SampleNum, channel, TrackerPan))
	{
		DEBUGPRINT(2, "Could not set pan on channel %u", channel);
		return FALSE;
	}

	DEBUGPRINT(6, "Start playing sample");

	if (!DXInterface->PlaySample(NoteSample->SampleNum, channel, ((XMSAMPLE *)NoteSample->ExtraData)->Loop))
	{
		DEBUGPRINT(2, "Could not play sample on channel %u", channel);
		return FALSE;
	}

	module.ChannelInfo[channel].lastSample = NoteSample;
	module.ChannelInfo[channel].isPlaying = TRUE;
	return TRUE;
}

BOOL XMPlayer::CheckFrequencies(VOID)
{
	BOOL resample = FALSE;

	for (int pattern = 0; pattern < module.OrderLength; pattern++)
	{
		assert(module.Order[pattern] < module.nPatterns);
		for (int channel = 0; channel < module.nChannels; channel++)
		{
			for (int row = 0; row < module.Patterns[module.Order[pattern]].nRows; row++)
			{
				NOTE curNote = module.Patterns[module.Order[pattern]].Notes[channel][row];
				if (curNote.Instrument > 0 && curNote.Instrument <= module.nInstruments && 
					curNote.NoteValue > 0 && curNote.NoteValue < 97)
				{
					LPINSTRUMENT instrument = &module.Instruments[curNote.Instrument - 1];
					LPSAMPLE NoteSample = &instrument->Samples[instrument->NoteSamples[curNote.NoteValue - 1]];
					CHAR RealNote = (signed int)NoteSample->RelativeNote + (signed int)curNote.NoteValue;

					if (NoteSample->Length > 0)
					{
						DOUBLE Frequency;
						
						if (((LPXMEXTRA)module.ExtraData)->Amiga)
							Frequency = AmigaFrequency(RealNote, NoteSample->FineTune);
						else
							Frequency = LinearFrequency(RealNote, NoteSample->FineTune);

						DEBUGPRINT(8, "Checking %f against max %lu", Frequency, DirectSoundInterface::GetMaxFrequency()); 
						if (Frequency > DirectSoundInterface::GetMaxFrequency())
						{
							NoteSample->DownSampleFactor = max(NoteSample->DownSampleFactor, 
								(UINT)(Frequency / DirectSoundInterface::GetMaxFrequency()) + 1);
							DEBUGPRINT(8, "Sample %u needs resampling, factor %u", instrument->NoteSamples[curNote.NoteValue - 1], NoteSample->DownSampleFactor);
							resample = TRUE;
						}
					}
				}
			}
		}
	}

	return resample;
}

XMPlayer::~XMPlayer(VOID)
{
	DeInitPlayer();
}

VOID XMPlayer::DeInitPlayer(VOID)
{
	for (int instrument = 0; instrument < module.nInstruments; instrument++)
	{
		for (int sample = 0; sample < module.Instruments[instrument].nSamples; sample++)
		{
			delete module.Instruments[instrument].Samples[sample].Name;
			delete module.Instruments[instrument].Samples[sample].Data;
			delete module.Instruments[instrument].Samples[sample].ExtraData;
		}

		delete module.Instruments[instrument].Name;

		if (module.Instruments[instrument].nSamples > 0)
		{
			delete module.Instruments[instrument].NoteSamples;
			LPXMINSTRUMENT XMInstrument = (LPXMINSTRUMENT) module.Instruments[instrument].ExtraData;
			delete XMInstrument->VolumeEnvelopePoints;
			delete XMInstrument->PanningEnvelopePoints;
			delete XMInstrument;
		}
	}

	for (int pattern = 0; pattern <= module.nPatterns; pattern++)
	{
		for (int channel = 0; channel < module.nChannels; channel++)
			delete module.Patterns[pattern].Notes[channel];
		
		delete module.Patterns[pattern].Notes;
	}

	delete module.ExtraData;
	delete module.ChannelInfo;
	delete module.Patterns;
	delete module.ModuleName;
	delete module.Order;
	delete module.TrackerName;
}

BOOL XMPlayer::InitPlay(VOID)
{
	return Timer->SetSpeed(2 * module.BPM / 5);
}
