#ifndef _DEF_GTK_REPAIR_VIEW_H
#define _DEF_GTK_REPAIR_VIEW_H

#include <gtk/gtk.h>

#include "RepairObserver.h"

class GtkRepairView : RepairObserver {
	public:
		static void * waitForFel(void * thisObj);
		static void repairThread(GtkWidget *, void * thisObj);
		GtkRepairView(int argc, char *argv[]);

		void onNotify(const std::string & progressText, float progressFraction, const std::string * details);

	private:
		static void * repair(void * thisObj);
		GtkWidget *window;

		GtkWidget *vbox;
		GtkWidget *progbar;

		GtkWidget *hbox;
		GtkWidget *instructionsLink;
		GtkWidget *button;
		GtkWidget *label;
		GtkWidget *halign;
		GtkWidget *valign;

};

#endif
