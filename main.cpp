#include <QApplication>
#include "Widgets/NQVTKWindow.h"

int main(int argc, char *argv[]) 
{
	// Set up Qt application
	QApplication app(argc, argv);
	NQVTKWindow *window = new NQVTKWindow;
	window->resize(800, 600);
	window->show();

	// Go!
	app.exec();

	// Clean up and exit
	return 0;
}
