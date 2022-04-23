#include "cutBox.h"
#include <string>
using namespace std;
string type, sample;

cutBox::cutBox(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.pushButton, &QPushButton::pressed, this, &cutBox::showGraph);
}


void cutBox::showGraph()
{
	QString qstype = ui.type->currentText();
	QString qssample = ui.sample->currentText();
	type = qstype.toStdString();
	sample = qssample.toStdString();
	emit startSignal();
}
