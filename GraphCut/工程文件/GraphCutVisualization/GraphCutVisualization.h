#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GraphCutVisualization.h"
#include "cutBox.h"

class GraphCutVisualization : public QMainWindow
{
    Q_OBJECT
public:
    GraphCutVisualization(QWidget *parent = Q_NULLPTR);
    void showResult();
    cutBox *selectBox;
    QGraphicsScene* scene;

public slots:
    void showBox();
    void drawGraph();

private:
    Ui::GraphCutVisualizationClass ui;
};
