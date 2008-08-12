#include <QApplication>
#include "Widgets/NQVTKWindow.h"

int main(int argc, char *argv[]) 
{
	// Set up Qt application
	QApplication app(argc, argv);
	NQVTKWindow *window = new NQVTKWindow;
	
	// Create renderers and styles
	window->CreateRenderers();
	
	// Show the window to get a GL context
	window->show();

	// Load the data
	// TODO: some multithreading might be nice, because we're showing a window
	// alternatively, find another hook before showing but after initGL
	window->LoadData();

	// Go!
	app.exec();

	// Clean up and exit
	return 0;
}
