#pragma execution_character_set("utf-8")
#include "GraphCutVisualization.h"
#include <iostream>
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsEllipseItem>
#include "Graph.h"
#include "ratioCut.h"
#include "smallestCut.h"

using namespace std;
extern string type, sample;
vector<int> clsRes;
double smallestRes;
double ratioRes;
Graph* newGraph;
vector<pair<double, double>> position;
const double PI = 3.1415926;

GraphCutVisualization::GraphCutVisualization(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    selectBox = new cutBox;
    connect(ui.action, SIGNAL(triggered()), this, SLOT(showBox()));
    connect(selectBox, SIGNAL(startSignal()), this, SLOT(drawGraph()));
    connect(ui.pushButton, &QPushButton::pressed, this, &GraphCutVisualization::showResult);

    QPixmap pixmap(ui.graphicsView->size());
    pixmap.fill(Qt::white);
    scene = new QGraphicsScene;
    scene->addPixmap(pixmap);
    ui.graphicsView->setScene(scene);
}

void GraphCutVisualization::showBox()
{
    selectBox->show();
}

void GraphCutVisualization::showResult()
{
    for (int i = 0; i < clsRes.size(); i++)
    {
        if (clsRes[i] == 0)
        {
            QGraphicsEllipseItem* circleItem = new QGraphicsEllipseItem();
            circleItem->setBrush(QBrush(QColor(138, 43, 226)));
            circleItem->setRect(QRectF(position[i].first, position[i].second, 20, 20));
            scene->addItem(circleItem);
        }
        else if (clsRes[i] == 1)
        {
            QGraphicsEllipseItem* circleItem = new QGraphicsEllipseItem();
            circleItem->setBrush(QBrush(QColor(0, 139, 0)));
            circleItem->setRect(QRectF(position[i].first, position[i].second, 20, 20));
            scene->addItem(circleItem);
        }
        else if (clsRes[i] == 2)
        {
            QGraphicsEllipseItem* circleItem = new QGraphicsEllipseItem();
            circleItem->setBrush(QBrush(QColor(238, 238, 0)));
            circleItem->setRect(QRectF(position[i].first, position[i].second, 20, 20));
            scene->addItem(circleItem);
        }
    }



    if (type == "Smallest")
    {
        QString contend = "SmallestCutֵΪ " + QString::number(smallestRes);
        QMessageBox::information(NULL, "Information", contend, QMessageBox::Yes);
    }
    else if (type == "Ratio")
    {
        QString contend = "RatioCutֵΪ " + QString::number(ratioRes);
        QMessageBox::information(NULL, "Information", contend, QMessageBox::Yes);
    }
}

void GraphCutVisualization::drawGraph()
{
    scene->clear();
    position.clear();

    string filename = sample + ".txt";
    if (type == "Smallest"){
        smallestCut(filename);        
    }
    else if (type == "Ratio"){
        ratiocut(filename);
    }

    int nodeNum = newGraph->m_nodeNum;

    double part = double(360) / nodeNum;

    for (int i = 0; i < nodeNum; i++){
        position.push_back({ 40 + 240 * sin(i * part * PI / 180), 40 + 240 * cos(i * part * PI / 180) });
    }

    for (int i = 0; i < nodeNum; i++){
        for (int j = i; j < nodeNum; j++){
            if (newGraph->m_adjMatrix[i][j] != 0)
            {
                QGraphicsLineItem* lineItem = new QGraphicsLineItem();
                lineItem->setLine(position[i].first + 10, position[i].second + 10,
                    position[j].first + 10, position[j].second + 10);
                scene->addItem(lineItem);
            }
        }
    }

    for (int i = 0; i < nodeNum; i++){
        QGraphicsEllipseItem* circleItem = new QGraphicsEllipseItem();
        circleItem->setBrush(QBrush(QColor(0, 160, 230)));
        circleItem->setRect(QRectF(position[i].first, position[i].second, 20, 20));
        scene->addItem(circleItem);
    }
}