#include "playerfactory.h"
#include "xmplayer.h"

using namespace DXModPlay;

PlayerFactory::PlayerFactory(ModuleTimer *Timer, DirectSoundInterface *DXInterface)
{
	players[MODULE_XM] = new XMPlayer(Timer, DXInterface);
	players[MODULE_MOD] = NULL;
	players[MODULE_S3M] = NULL;
	players[MODULE_IT] = NULL;
}

FormatPlayer *PlayerFactory::getPlayer(ModuleType type)
{
	if (players[type] != NULL)
		return players[type];
	else
		throw "Invalid module type";
}
