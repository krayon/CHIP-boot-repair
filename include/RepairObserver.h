#ifndef _DEF_REPAIR_OBSERVER_H
#define _DEF_REPAIR_OBSERVER_H
#include <string>
class RepairObserver {
	public:
		virtual void onNotify(const std::string & progressText, float progressFraction, const std::string * details)=0;
		virtual ~RepairObserver() {}
};

#endif
