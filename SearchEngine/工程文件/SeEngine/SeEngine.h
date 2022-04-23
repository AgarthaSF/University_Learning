#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_SeEngine.h"
using namespace std;



class SeEngine : public QMainWindow
{
    Q_OBJECT

public:
    SeEngine(QWidget *parent = Q_NULLPTR);
    void search();

public slots:
    void indexInitial();
    void startInitial();

private:
    Ui::SeEngineClass ui;
};
