#include "DirectXMODPlay.h"
#include "moduleplayer.h"
#include "xmplayer.h"
#include "playerfactory.h"

using namespace DXModPlay;

BOOL DirectXMODPlayer::Load(std::istream &ModuleName, ModuleType type)
{
	if (DXInterface != NULL)
	{
		SAFE_DELETE(Player);

		DEBUGPRINT(2, "Allocating player");
		PlayerFactory *factory = new PlayerFactory(Timer, DXInterface);
		Player = factory->getPlayer(type);

		DEBUGPRINT(2, "Reading module");
		return InitModule(ModuleName);
	}
	else
		return FALSE;
}


DirectXMODPlayer::DirectXMODPlayer(VOID)
{
	this->Timer = new ModuleTimer();
	DXInterface = NULL;
	Player = NULL;
}

BOOL DirectXMODPlayer::DownSample(VOID)
{
	for (int instrument = 0; instrument < module->nInstruments; instrument++)
	{
		for (int sample = 0; sample < module->Instruments[instrument].nSamples; sample++)
		{
			if (module->Instruments[instrument].Samples[sample].Length > 0 &&
				(module->Instruments[instrument].Samples[sample].DownSampleFactor > 1 || module->Instruments[instrument].Samples[sample].UpSampleFactor > 1))
			{

				DEBUGPRINT(4, "Resampling instrument %d, sample %d", instrument, sample);
				DEBUGPRINT(4, "Downsample factor: %u, Upsample factor: %u", module->Instruments[instrument].Samples[sample].DownSampleFactor,
							module->Instruments[instrument].Samples[sample].UpSampleFactor);
				UINT d = module->Instruments[instrument].Samples[sample].DownSampleFactor, u = module->Instruments[instrument].Samples[sample].UpSampleFactor;
				BYTE *newSample = new BYTE[module->Instruments[instrument].Samples[sample].Length * d];
				DOUBLE fsin = 1;
				DOUBLE fgG = (DOUBLE)u / d * fsin * 0.0116;
				DOUBLE fgK = (DOUBLE)u / d * fsin * 0.461;
				INT L = (INT)(162 * (DOUBLE)d / u + 0.5);
				DOUBLE gain = 0.8;

				int newLen = rateconv((int *)module->Instruments[instrument].Samples[sample].Data, module->Instruments[instrument].Samples[sample].Length / sizeof(int), (int *)newSample, fsin, fgG, fgK, u, d, L, gain, 0, 1);
				if (newLen == 0)
				{
					DEBUGPRINT(1, "Resampling failed");
					return FALSE;
				}

				module->Instruments[instrument].Samples[sample].Length = newLen;
				delete module->Instruments[instrument].Samples[sample].Data;
				module->Instruments[instrument].Samples[sample].Data = newSample;
			}
		}
	}
	return TRUE;
}
	
BOOL DirectXMODPlayer::InitModule(std::istream &Module)
{
	module = Player->ReadModule(Module);

	if (module == NULL)
		return FALSE;

	DEBUGPRINT(2, "Checking frequencies");

	if (Player->CheckFrequencies())
		if (!DownSample())
			return FALSE;

	DEBUGPRINT(2, "Registering samples");
	UINT nSamples = 0;
	for (int instrument = 0; instrument < module->nInstruments; instrument++)
		for (int sample = 0; sample < module->Instruments[instrument].nSamples; sample++)
			if (module->Instruments[instrument].Samples[sample].Length > 0)
				nSamples++;

	DXInterface->Init(nSamples, module->nChannels);

	for (int instrument = 0; instrument < module->nInstruments; instrument++)
		for (int sample = 0; sample < module->Instruments[instrument].nSamples; sample++)
			if (module->Instruments[instrument].Samples[sample].Length > 0)
				module->Instruments[instrument].Samples[sample].SampleNum =
					DXInterface->AddSample(module->Instruments[instrument].Samples[sample].Data,
										   module->Instruments[instrument].Samples[sample].Length,
										   module->Instruments[instrument].Samples[sample].BitsPerSample);
	
	DEBUGPRINT(2, "Initialising channels");

	module->ChannelInfo = new CHANNELINFO[module->nChannels];
	for (int channel = 0; channel < module->nChannels; channel++)
		memset(&module->ChannelInfo[channel], 0, sizeof module->ChannelInfo[channel]);

	DEBUGPRINT(1, "Init module done");

	return TRUE;
}

DWORD Ticks;
LPMODULE CurrentModule;
FormatPlayer *MODPlayer;
ModuleTimer *MODTimer;
VOID *PlayCallBackData, *TickCallBackData;
ModuleCallBack PlayCallBack, TickCallBack;

VOID ModuleTimerCallBack(VOID)
{
	DEBUGPRINT(6, "Timer callback");
	if (TickCallBack != NULL)
		(*TickCallBack)(TickCallBackData);

	// Need >=, because Tempo can be changed (ie subtracted, in which case we would have an error...)
	if (++Ticks >= CurrentModule->Tempo)
	{
		if (PlayCallBack != NULL)
			(*PlayCallBack)(PlayCallBackData);

		MODPlayer->PlayRow();
		Ticks = 0;
	}
}

DWORD WINAPI PlayModuleThread(LPVOID lpThreadParameter)
{
	Ticks = CurrentModule->Tempo;

	DEBUGPRINT(2, "Initialising timer and player");

	if (!MODPlayer->InitPlay())
	{
		DEBUGPRINT(1, "Could not initialise playing");
		return FALSE;
	}

	if (!MODTimer->Start())
	{
		DEBUGPRINT(1, "Could not start timer");
		return FALSE;
	}

	MSG msg;

    while (GetMessage(&msg, // message structure 
            NULL,           // handle of window to receive the message 
            NULL,           // lowest message to examine 
            NULL))          // highest message to examine 
    { 
        TranslateMessage(&msg); // translates virtual-key codes 
        DispatchMessage(&msg);  // dispatches message to window 
		WaitMessage();
    } 

	return TRUE;
}

BOOL DirectXMODPlayer::Init(HWND hwnd)
{
	DeInit();
	DXInterface = new DirectSoundInterface(hwnd);
	
	PlayCallBack = NULL;
	TickCallBack = NULL;

	return TRUE;
}

VOID DirectXMODPlayer::SetPlayCallBack(ModuleCallBack CallBack, VOID *Data)
{
	PlayCallBack = CallBack;
	PlayCallBackData = Data;
}

VOID DirectXMODPlayer::SetTickCallBack(ModuleCallBack CallBack, VOID *Data)
{
	TickCallBack = CallBack;
	TickCallBackData = Data;
}

BOOL DirectXMODPlayer::Play(VOID)
{
	if (Player == NULL)
		return FALSE;

	DEBUGPRINT(2, "Start playing primary buffer");

	if (!DXInterface->StartPlaying())
	{
		DEBUGPRINT(1, "Could not play primary buffer");
		return FALSE;
	}

	if (!Timer->Init(ModuleTimerCallBack))
	{
		DEBUGPRINT(1, "Could not set timer callback");
		return FALSE;
	}

	CurrentModule = module;
	MODPlayer = Player;
	MODTimer = Timer;
	ThreadHandle = CreateThread(NULL, 1024, PlayModuleThread, NULL, 0, &ThreadID);
	if (ThreadHandle == 0)
	{
		DEBUGPRINT(1, "Could not create player thread");
		return FALSE;
	}

	if (SetThreadPriority(ThreadHandle, THREAD_PRIORITY_TIME_CRITICAL) == 0)
	{
		DEBUGPRINT(1, "Could not set player thread priority");
		return FALSE;
	}

	return TRUE;
}

BOOL DirectXMODPlayer::DeInit(VOID)
{
	Stop();
	SAFE_DELETE(Player);
	SAFE_DELETE(DXInterface);
	
	return TRUE;
}

DirectXMODPlayer::~DirectXMODPlayer(VOID)
{
	DeInit();
	delete Timer;
}

BOOL DirectXMODPlayer::Stop(VOID)
{
	if (DXInterface != NULL)
	{
		Timer->Stop();	
		DXInterface->StopPlaying();

		if (!PostThreadMessage(ThreadID, WM_QUIT, 0, 0))
			return FALSE;

		return TRUE;
	}
	else
		return FALSE;
}

VOID DirectXMODPlayer::SetVolume(UCHAR Volume)
{
	if (Player != NULL)
		Player->SetVolume(Volume);
}

UCHAR DirectXMODPlayer::GetVolume(VOID)
{
	if (Player != NULL)
		return Player->GetVolume();
	else
		return 0;
}

VOID SetPlayCallBack(ModuleCallBack CallBack, VOID *Data);
VOID SetTickCallBack(ModuleCallBack CallBack, VOID *Data);

