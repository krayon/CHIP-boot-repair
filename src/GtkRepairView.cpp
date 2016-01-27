#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include "RepairTool.h"
#include "GtkRepairView.h"

const std::string DESCRIPTION = "This tool will repair issues related to the NAND memory on C.H.I.P.\n The whole process takes just a few seconds.";
void GtkRepairView::onNotify(const std::string & progressText, float progressFraction, const std::string * details) {
	gdk_threads_enter();
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR(progbar), progressText.c_str());
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progbar),progressFraction);
	std::string labelText = details ? *details : "";
	gtk_label_set_text(GTK_LABEL(label), labelText.c_str());
	gdk_threads_leave();
}

//static
void * GtkRepairView::waitForFel(void * thisObj) {
	gdk_threads_enter();
	GtkRepairView * view = (GtkRepairView *)thisObj;
	RepairTool::staticWaitForFel(view);
	const std::string message = "Click Repair to begin";
	gtk_widget_set_sensitive(view->button, TRUE);
	gdk_threads_leave();
	view->onNotify("C.H.I.P. in FEL mode found",0.05, &message);
	return view->button;
}

//static
void * GtkRepairView::repair(void * thisObj) {
	GtkRepairView * view = (GtkRepairView *)thisObj;
	RepairTool::runSimple(view,false);
	return nullptr;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void GtkRepairView::repairThread(GtkWidget * widget,void * thisObj) {
	GtkRepairView * view = (GtkRepairView *)thisObj;
	gtk_widget_set_sensitive(view->button, FALSE);
	g_thread_new("repair", GtkRepairView::repair, view);
}
#pragma GCC diagnostic pop


GtkRepairView::GtkRepairView(int argc, char *argv[]) {

	gdk_threads_init();

	gtk_init(&argc, &argv);

	uid_t uid=getuid(), euid=geteuid();
	if (uid!=0 || uid!=euid) {
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new(NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				"You need to be root to run the C.H.I.P repair tool");
		gtk_window_set_title(GTK_WINDOW(dialog), "Error");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		exit(1);
	}

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 150);
	gtk_window_set_title(GTK_WINDOW(window), "C.H.I.P. Boot Repair");
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);

	g_signal_connect(G_OBJECT(window), "destroy",
			G_CALLBACK(gtk_main_quit), G_OBJECT(window));

	vbox = gtk_vbox_new(TRUE, 5);

	label = gtk_label_new(DESCRIPTION.c_str());

	progbar = gtk_progress_bar_new();
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR(progbar),
			"Waiting for a C.H.I.P. in FEL mode...");

	gtk_box_pack_start(GTK_BOX(vbox), progbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);


	valign = gtk_alignment_new(0, 1, 0, 0);
	gtk_container_add(GTK_CONTAINER(vbox), valign);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	hbox = gtk_hbox_new(TRUE, 3);

	button = gtk_button_new_with_label("Repair");
	gtk_widget_set_sensitive(button, FALSE);
	gtk_widget_set_size_request(button, 70, 30);
	gtk_container_add(GTK_CONTAINER(hbox), button);

	g_signal_connect(button, "clicked",
			G_CALLBACK(GtkRepairView::repairThread), this);

	halign = gtk_alignment_new(1,0,0,0);
	gtk_container_add(GTK_CONTAINER(halign), hbox);

	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 0);


	/* Create new thread */
	auto thread = g_thread_new("waitForFel", GtkRepairView::waitForFel, this);

	if( ! thread )
	{
		g_print( "Error: could not make thread\n");
		return;
	}

	gtk_widget_show_all(window);

	gtk_main();

}

void show_error() {
}

int main(int argc, char *argv[]) {
	auto view = new GtkRepairView(argc,argv);
	delete view;
}

