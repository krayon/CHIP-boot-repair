#ifndef _DEF_REPAIR_TOOL_H
#define _DEF_REPAIR_TOOL_H

#include <list>
#include <iostream>
#include <vector>
#include <string>
using namespace std;
using Strings = vector<string>;
#include "RepairObserver.h"
class RepairTool {
public:
	RepairTool();
	static void runSimple(RepairObserver * view, bool wait);
	bool repair(bool wait);
	void repairLoop(bool wait);
	static int staticCheckForFel();
	static void staticWaitForFel(RepairObserver * observer = nullptr);

	void addObserver(RepairObserver * observer);
private:
	std::list<RepairObserver *> * observers;

	static int do_fel(const Strings & commands, char **returnBuffer);
	void waitForFel();
	int spl_write();
	int spl_w_ecc_write();
	int uboot_write();
	int uboot_scr_write();
	int fel_exe();
	void complete();
	int checkForFel();
	void notify(const std::string & progressText, float progressFraction,const std::string * details= nullptr);
};

#endif
