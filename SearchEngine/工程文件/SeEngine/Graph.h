#pragma once
#include <string>
#include <list>
using namespace std;

class GraphNode
{
public:
	QString time;
	int urlId;
	int pageRank;
	vector<GraphNode*> linkedUrl;
	int linkedNum;

	GraphNode(int urlId)
	{
		this->urlId = urlId;
	}

};