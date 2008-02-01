#include <QApplication>
#include "Widgets/NQVTKWidget.h"

int main(int argc, char *argv[]) 
{
	// Set up Qt application
	QApplication app(argc, argv);
	NQVTKWidget *widget = new NQVTKWidget;
	widget->show();

	// Go!
	app.exec();

	// Clean up and exit
	return 0;
}
