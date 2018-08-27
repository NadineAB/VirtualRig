#include "VirtualRig.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	VirtualRig w; 
	w.show();
	return a.exec();
}
