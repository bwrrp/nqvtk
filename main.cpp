#include <QApplication>
#include "Widgets/NQVTKWidget.h"

int main(int argc, char *argv[]) 
{
	// Set up Qt application
	QApplication app(argc, argv);
	NQVTKWidget *widget = new NQVTKWidget;
	widget->resize(800, 600);
	widget->show();

	// Go!
	app.exec();

	// Clean up and exit
	return 0;
}
