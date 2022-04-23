#pragma execution_character_set("utf-8")
#include "Graph.h"
#include "math.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <QTextStream>
#include <QHash>
#include <QDebug>
#include <QFile>
#include <QDataStream>
using namespace std;

namespace std {
	template<> struct hash<QString> {
		std::size_t operator()(const QString& s) const noexcept {
			return (size_t)qHash(s);
		}
	};
}

class InvertedIndex
{
public:
	void dataProcess(QString filename);
	void initial();
	void PageRank();

public:
	unordered_map<QString, unordered_set<int>> InvertedIndex;   //由单词索引网址节点，网址由数字Id标识
	unordered_map<QString, GraphNode*> urlIndex;				//由网址索引url节点
	GraphNode* currentUrlNode;
	QString pathname;
	vector<double> pageRankRes;
	vector<double> beforeRes;
};

void InvertedIndex::dataProcess(QString filename)
{
	pathname = filename;
	initial();
	PageRank();
}

void InvertedIndex::initial()
{
	int currentUrlId = -1;
	QFile qf(pathname);
	qf.open(QIODevice::ReadOnly);

	QString lineContend;
	QString processContend;
	QString UrlBuffer[10000];
	QString timeBuffer[10000];
	int offsetBuffer[10000];

	int offset = 0;
	int bufCnt = -1;

	QFile writeContend("Url2IndexContend.txt");   //建立id到url的二进制索引文件
	writeContend.open(QIODevice::WriteOnly);
	QDataStream dataStream(&writeContend);

	ofstream writeIndex("Url2IndexOffset.txt", ios::binary);

	while (!qf.atEnd())
	{
		lineContend = qf.readLine();
		if (lineContend.size() == 0)
			continue;

		processContend = lineContend.mid(2, lineContend.size() - 4);
		if (lineContend[0] == "P")
		{
			currentUrlId++;
			bufCnt++;
			currentUrlNode = new GraphNode(currentUrlId);
			urlIndex[processContend] = currentUrlNode;

			offsetBuffer[bufCnt] = offset;
			UrlBuffer[bufCnt] = processContend;

			offset += strlen(processContend.toStdString().c_str()) + 4;

			if (bufCnt == 9999)
			{
				bufCnt = -1;
				for (int i = 0; i < 10000; i++)
				{
					const char* temp = UrlBuffer[i].toStdString().c_str();
					dataStream.writeBytes(temp, strlen(temp));
				}
				writeIndex.write((const char*)offsetBuffer, sizeof(int) * 10000);
			}

		}
		else if (lineContend[0] == "T")
		{
			currentUrlNode->time = processContend;
			timeBuffer[bufCnt] = processContend;
 		}
		else if (lineContend[0] == "Q")
		{
			QStringList words = processContend.split(' ');
			for (int i = 0; i < words.size(); i++)
				InvertedIndex[words[i]].insert(currentUrlId);
		}
	}

	if (bufCnt != 0)
	{
		for (int i = 0; i < bufCnt + 1; i++)
		{
			string test = UrlBuffer[i].toStdString();
			const char* temp = test.c_str();
			dataStream.writeBytes(temp, strlen(temp));
		}
		writeIndex.write((const char*)offsetBuffer, sizeof(int) * (bufCnt+1));
	}

	QFile writeInv("InvIndex.txt");   //存储倒排索引文件
	writeInv.open(QIODevice::WriteOnly);
	QTextStream writei(&writeInv);

	for (auto it = InvertedIndex.begin(); it != InvertedIndex.end(); it++)
	{
		writei << it->first << " ";
		for(auto sit = it->second.begin(); sit != it->second.end(); sit++)
			writei << *sit << " ";
		writei << endl;
	}
}


void InvertedIndex::PageRank()
{
	int currentUrlId = -1;

	pageRankRes.resize(urlIndex.size());

	for (int i = 0; i < pageRankRes.size(); i++)
		pageRankRes[i] = 1;

	QFile qf(pathname);
	qf.open(QIODevice::ReadOnly);

	QString lineContend;
	QString processContend;
	QString currentURLStr;
	int linkedNum = 0;

	while (!qf.atEnd())
	{
		lineContend = qf.readLine();
		if (lineContend.size() == 0)
			continue;

		//先获取超链接节点
		if (lineContend[0] == "P")
		{
			currentUrlNode->linkedNum = linkedNum;
			linkedNum = 0;
			currentUrlId++;
			currentURLStr = lineContend.mid(2, lineContend.size() - 4);
			currentUrlNode = urlIndex[currentURLStr];
		}

		if (lineContend[0] == "L")
		{
			linkedNum++;
			processContend = lineContend.mid(2, lineContend.size() - 4);

			if (urlIndex.count(processContend) != 0)
			{
				currentUrlNode->linkedUrl.push_back(urlIndex[processContend]);
			}
		}
	}
	currentUrlNode->linkedNum = linkedNum;


	double eps = 0.1;
	double initialCnt = 0.15 / urlIndex.size();

	while (eps > 0.0001)
	{
		eps = 0;
		beforeRes = pageRankRes;

		for (int i = 0; i < pageRankRes.size(); i++)
			pageRankRes[i] = initialCnt;

		for (auto it = urlIndex.begin(); it != urlIndex.end(); it++)
			for (int i = 0; i < it->second->linkedUrl.size(); i++)
				pageRankRes[it->second->linkedUrl[i]->urlId] += 0.85 * beforeRes[it->second->urlId] / double(it->second->linkedNum);

		for (auto it = urlIndex.begin(); it != urlIndex.end(); it++)
			if (pageRankRes[it->second->urlId] != beforeRes[it->second->urlId])
				eps += abs(pageRankRes[it->second->urlId] - beforeRes[it->second->urlId]);
	}

	ofstream ofs("PageRank.txt", ios::binary);
	int totalcnt = pageRankRes.size();
	double* prArray = new double[totalcnt];
	for (int i = 0; i < totalcnt; i++)
		prArray[i] = pageRankRes[i];
	ofs.write(reinterpret_cast<const char*>(&totalcnt), sizeof(int));
	ofs.write((const char*)prArray, sizeof(double) * totalcnt);
	ofs.close();
}