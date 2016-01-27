#ifndef _DEF_CONSOLE_REPAIR_VIEW_H
#define _DEF_CONSOLE_REPAIR_VIEW_H


#include "RepairObserver.h"

class ConsoleRepairView : public RepairObserver {
	public:
		ConsoleRepairView();
		void main();
		virtual void onNotify(const std::string & progressText, float progressFraction, const std::string * details);
};

#endif
