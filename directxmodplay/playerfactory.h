#include "moduleplayer.h"
#include "dxinterface.h"
#include "moduletimer.h"

namespace DXModPlay
{
	class PlayerFactory {
		public:
			PlayerFactory(ModuleTimer *Timer, DirectSoundInterface *DXInterface);
			FormatPlayer *getPlayer(ModuleType type);

		protected:
			FormatPlayer *players[nModuleTypes];
	};
}
