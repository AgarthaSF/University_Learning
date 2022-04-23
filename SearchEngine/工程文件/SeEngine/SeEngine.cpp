#pragma execution_character_set("utf-8")
#include "SeEngine.h"
#include "InvertedIndex.h"
#include <set>
#include <map>
#include <algorithm>
#include <unordered_set>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QTableWidgetItem>

InvertedIndex* invIndex;

SeEngine::SeEngine(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.action_init, SIGNAL(triggered()), this, SLOT(indexInitial()));
    connect(ui.action_restart, SIGNAL(triggered()), this, SLOT(startInitial()));
    connect(ui.pushButton, &QPushButton::pressed, this, &SeEngine::search);
}


void SeEngine::indexInitial()
{
    invIndex = new InvertedIndex;

    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("���ļ�"),
        "./",
        tr("Documents(*.txt);;All files(*.*)"));

    if (fileName.isEmpty())
        return;

    QMessageBox msg(this);
    msg.setWindowTitle("������ʼ���д�����");
    msg.show();

    invIndex->dataProcess(fileName);
    msg.close();


    multimap<double, int> PRorder;
    for (int i = 0; i < invIndex->pageRankRes.size(); i++)
    {
        PRorder.insert({ invIndex->pageRankRes[i], i });
    }

    ifstream ifs("Url2IndexOffset.txt", ios::binary);
    QFile qf("Url2IndexContend.txt");
    qf.open(QIODevice::ReadOnly);
    QDataStream in(&qf);
    int cnt = 0;
    for (auto it = PRorder.rbegin(); it != PRorder.rend() && cnt < 10; it++, cnt++)
    {
        ifs.seekg(4 * (it->second));
        int offset;
        ifs.read(reinterpret_cast<char*>(&offset), sizeof(int));
        qf.seek(offset);
        char* urlContend;
        uint readLen;
        in.readBytes(urlContend, readLen);
        ui.tableWidget->setItem(cnt, 0, new QTableWidgetItem(QString::fromLocal8Bit(urlContend)));
    }

    ui.tableWidget->setHorizontalHeaderLabels(QStringList() << "PageRank Top10 Url");
    ifs.close();
    qf.close();

    QMessageBox::information(this,
        tr("��ʼ��"),
        tr("������ʼ�����"),
        QMessageBox::Ok);
}

void SeEngine::search()
{
    ui.tableWidget->clear();
    ui.tableWidget->setRowCount(10);
    //��ȡ��ѯ����
    QString contend = ui.lineEdit->text();
    QStringList words = contend.split(' ');
    unordered_set<int> urlRes;

    qDebug() << invIndex->InvertedIndex.size() << endl;

    //���ݵ���������ȡ������ҳ���
    for (int i = 0; i < words.size(); i++) {
        for (auto it = invIndex->InvertedIndex[words[i]].begin();
            it != invIndex->InvertedIndex[(QString)words[i]].end(); it++)
        {
            urlRes.insert(*it);
        }
    }

    //����pagerank��������ҳ�������
    multimap<double, int> PRorder;
    for (auto it = urlRes.begin(); it != urlRes.end(); it++) {
        PRorder.insert({ invIndex->pageRankRes[*it], *it });
    }

    ui.tableWidget->setRowCount(PRorder.size());

    //�����ļ���������ҳ���ӳ��Ϊ��ʵUrl
    ifstream ifs("Url2IndexOffset.txt", ios::binary);
    QFile qf("Url2IndexContend.txt");
    qf.open(QIODevice::ReadOnly);
    QDataStream in(&qf);
    int cnt = 0;
    for (auto it = PRorder.rbegin(); it != PRorder.rend(); it++)
    {
        ifs.seekg(4 * (it->second));
        int offset;
        ifs.read(reinterpret_cast<char*>(&offset), sizeof(int));
        qf.seek(offset);
        char* urlContend;
        uint readLen;
        in.readBytes(urlContend, readLen);
        ui.tableWidget->setItem(cnt, 0, new QTableWidgetItem(QString::fromLocal8Bit(urlContend)));
        cnt++;
    }
    ui.tableWidget->setHorizontalHeaderLabels(QStringList() << "Search Result Url");
    ifs.close();
    qf.close();
}

void SeEngine::startInitial()
{
    ifstream ifs("PageRank.txt", ios::binary);
    int total = 0;
    ifs.read(reinterpret_cast<char*>(&total), sizeof(int));

    if (total == 0)
    {
        QMessageBox::information(this,
            tr("���³�ʼ��"),
            tr("δ��⵽�����ļ��������ٽ���һ�γ�ʼ��"),
            QMessageBox::Ok);
    }

    if (total != 0)  //��Ϊ0��˵��֮ǰ�Ѿ���ʼ����
    {
        QMessageBox::StandardButton box;
        box = QMessageBox::question(this, "��ʾ", "��⵽�����ļ����Ƿ�ʹ�ø��ļ���ʼ������?", QMessageBox::Yes | QMessageBox::No);
        if (box == QMessageBox::No)
            return;
        else
        {

            QMessageBox msg(this);
            msg.setWindowTitle("������ʼ���д�����");
            msg.show();
            invIndex = new InvertedIndex;

            //�ؽ�pagerank����
            double* pr = new double[total];
            ifs.read((char*)pr, sizeof(double) * total);

            invIndex->pageRankRes.resize(total);
            for (int i = 0; i < total; i++)
                invIndex->pageRankRes[i] = pr[i];

            //�ؽ���������
            QFile qf("InvIndex.txt");
            qf.open(QIODevice::ReadOnly);
            QString lineContend;
            QString wordContend;
            QStringList lineList;
            while (!qf.atEnd())
            {
                lineContend = qf.readLine();
                lineList = lineContend.split(' ');
                wordContend = lineList[0];
                for (int i = 1; i < lineList.size() - 1; i++)
                    invIndex->InvertedIndex[wordContend].insert(lineList[i].toInt());
            }
            qf.close();

            multimap<double, int> PRorder;
            for (int i = 0; i < invIndex->pageRankRes.size(); i++)
            {
                PRorder.insert({ invIndex->pageRankRes[i], i });
            }

            ifstream ifs("Url2IndexOffset.txt", ios::binary);
            QFile qfu2i("Url2IndexContend.txt");
            qfu2i.open(QIODevice::ReadOnly);
            QDataStream in(&qfu2i);
            int cnt = 0;
            for (auto it = PRorder.rbegin(); it != PRorder.rend() && cnt < 10; it++, cnt++)
            {
                ifs.seekg(4 * (it->second));
                int offset;
                ifs.read(reinterpret_cast<char*>(&offset), sizeof(int));
                qfu2i.seek(offset);
                char* urlContend;
                uint readLen;
                in.readBytes(urlContend, readLen);
                ui.tableWidget->setItem(cnt, 0, new QTableWidgetItem(QString::fromLocal8Bit(urlContend)));
            }

            ui.tableWidget->setHorizontalHeaderLabels(QStringList() << "PageRank Top10 Url");
            ifs.close();
            qfu2i.close();
            msg.close();
            QMessageBox::information(this,
                tr("��ʼ��"),
                tr("������ʼ�����"),
                QMessageBox::Ok);
        }
    }
    ifs.close();
}