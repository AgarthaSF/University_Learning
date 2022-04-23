#pragma once

#include <QWidget>
#include "ui_cutBox.h"

class cutBox : public QWidget
{
	Q_OBJECT


signals:
	void startSignal();

public:
	cutBox(QWidget *parent = Q_NULLPTR);
	void showGraph();



private:
	Ui::cutBox ui;
};
