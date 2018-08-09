#include "virtua.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	virtua w; 
	w.show();
	return a.exec();
}
