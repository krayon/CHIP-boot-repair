#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

#ifdef _WIN32
#include <windows.h>
#define sleep(val) Sleep(1000L * val)
#else
#include <unistd.h>
#endif

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

extern "C" {
#include "libsunxi.h"
}
#include "RepairTool.h"
#include "RepairObserver.h"
int timeout = 30;

const int SUCCESS = 0;
const int FAILURE = 1;
const char *FEL_FOUND="C.H.I.P. in FEL mode found";

// static method
void RepairTool::runSimple(RepairObserver * view, bool wait) {
	RepairTool *repairTool = new RepairTool();
	repairTool->addObserver(view);
  repairTool->repair(wait);
	delete repairTool;
}

/* This filePrefix will be used to locate the files that the FEL tool uses */
const std::string filePrefix() {
#ifdef DEVELOPMENT
#define PREFIX "./payload/"
#elif defined(__APPLE__)
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
      return std::string("/share/");
    }
    CFRelease(resourcesURL);
    chdir(path);
    std::string PREFIX(path);
    PREFIX.append("/share/");
    std::cout << "Prefix: " << PREFIX <<std::endl;
#elif defined(__linux__)
#define PREFIX "/usr/share/chip-boot-repair/"
#else
#define PREFIX "./payload/"
#endif
	return PREFIX;
}

Strings fel_ver = { "./fel", "ver"};
Strings fel_spl = { "./fel", "spl", "PREFIXsunxi-spl.bin"};
Strings fel_write_spl = { "./fel", "write", "0x43200000", "PREFIXsunxi-spl-with-ecc.bin" };
Strings fel_write_uboot = { "./fel", "write", "0x4a000000", "PREFIXpadded-uboot" };
Strings fel_write_uboot_script = { "./fel", "write", "0x43100000", "PREFIXuboot.scr" };
Strings fel_execute = { "./fel", "exe", "0x4a000000" };

// From http://bits.minhazulhaque.com/cpp/find-and-replace-all-occurrences-in-cpp-string.html
void find_and_replace(string& source, string const& find, string const& replace)
{

    for(string::size_type i = 0; (i = source.find(find, i)) != string::npos;)
    {
        source.replace(i, find.length(), replace);
        i += replace.length();
    }
}

char ** prefixedStringArray(const std::string & prefix, const Strings & strings) {
	char ** result = new char*[strings.size()];
	for (unsigned int i=0; i < strings.size(); i++) {
		string s = string(strings[i]);
		find_and_replace(s,"PREFIX",prefix);
		result[i] = strdup(s.c_str());
	}
	return result;
}


bool RepairTool::repair(bool wait) {
	if (wait)
		waitForFel();
	spl_write();
	spl_w_ecc_write();
	uboot_write();
	uboot_scr_write();
	fel_exe();
	complete();
	return true;
}

void RepairTool::repairLoop(bool wait) {
	for (;;)
		repair(wait);
}

RepairTool::RepairTool() {
	observers = new std::list<RepairObserver *>();
}

void RepairTool::addObserver(RepairObserver * observer) {
	observers->push_back(observer);
}

/* Function to call the libsunxi fel method
 * @arg arc: main's argc
 * @arg argv: main's argv
 * @callback: an optional callback to call after calling fel
 */
int RepairTool::do_fel(const Strings & commands, char **returnBuffer) {
	int argc = commands.size();
	char ** argv = prefixedStringArray(filePrefix(),commands); // this will leak, but don't care for now

	char * buffer;
    int result = fel(argc, argv, &buffer);
    if (returnBuffer) {
		if (buffer && strlen(buffer) > 0)
			*returnBuffer = buffer;
		else
			*returnBuffer = (char *)calloc(1,1);
    }
	return result;
}


const std::string FEL_NO_PERMISSION_STRING = "You don't have permission to run this program.\n Close and run: sudo chip-boot-repair";
const std::string FEL_NOT_FOUND_STRING = "FEL Device not found";
const std::string FEL_CANNOT_CLAIM_INTERFACE_STRING = "Disconnect CHIP, close the application, and try again.";
const std::string FEL_NEED_TO_BE_ROOT = "You need to bee root to run chip-repair-tool";

int RepairTool::staticCheckForFel() {
	int result = 0;
	char * buffer;
    result = do_fel(fel_ver, &buffer);
	if (result == SUCCESS) {
		if (buffer[0] != 'A')
			result = FAILURE;
	}
    if (buffer)
    	free(buffer);
	return result;
}

void RepairTool::staticWaitForFel(RepairObserver * observer) {
	while (true) {
		int result  = staticCheckForFel();
		if (result == SUCCESS) {
			break;
		}
		if (observer) {
			const std::string * warning  = nullptr;
			if (result == FEL_NO_PERMISSION) {
				warning = &FEL_NO_PERMISSION_STRING;
			} else if (result == FEL_CANNOT_CLAIM_INTERFACE) {
				warning = &FEL_CANNOT_CLAIM_INTERFACE_STRING;
			}
			if (warning)
				observer->onNotify("",0,warning);
		}
		sleep(1);
//This doesnt compile		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int RepairTool::checkForFel(){
	notify("Waiting for a C.H.I.P. in FEL mode...", 0);
	int result = staticCheckForFel();
	if (result == SUCCESS) {
		notify(FEL_FOUND, 0.05);
	}

	return result;
}

int RepairTool::spl_write(){
	notify("Upload SPL...", 0.1);
	return do_fel(fel_spl, NULL);
}

int RepairTool::spl_w_ecc_write(){
	notify("Upload SPL with ECC...", 0.3);
	return do_fel(fel_write_spl, NULL);
}

int RepairTool::uboot_write(){
	notify("Upload uboot...", 0.5);
	return do_fel(fel_write_uboot, NULL);
}

int RepairTool::uboot_scr_write(){
	notify("Uboot scr write...", 0.7);
	return do_fel(fel_write_uboot_script, NULL);
}

int RepairTool::fel_exe(){
	notify("Execute uboot script...", 0.9);
	int result = do_fel(fel_execute, NULL);
	sleep(3);
	return result;
}

void RepairTool::complete() {
	std::string details = "You may remove the jumper and unplug your C.H.I.P. now.";
#ifdef _WIN32
	details += "\nWindows will warn you that a USB device malfunctioned. It is not a problem.";
#endif
	notify("Repair Completed!", 1.0, &details);
}
void RepairTool::notify(const std::string & progressText, float progressFraction, const std::string * details) {
	for (auto observer : *observers) {
		observer->onNotify(progressText,progressFraction,details);
	}
}

void RepairTool::waitForFel() {
  while (true) {
		if (checkForFel() == SUCCESS)
			break;
		sleep(1);
//This doesnt compile		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
