#include "PanSharpener.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	GDALAllRegister();
	QApplication a(argc, argv);
	PanSharpener w;

    w.show();
	return a.exec();
}
