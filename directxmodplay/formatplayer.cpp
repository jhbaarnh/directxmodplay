#include "DirectXMODPlay.h"
#include "formatplayer.h"

using namespace DXModPlay;

FormatPlayer::FormatPlayer(ModuleTimer *Timer, DirectSoundInterface *DXInterface)
{
	this->Timer = Timer;
	this->DXInterface = DXInterface;
}
