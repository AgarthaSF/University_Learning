#pragma once
#include "Kmeans.h"
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "Eigen/Core"
#include <vector>
using namespace Eigen;
using namespace std;

struct Node
{
	int parentGraph;  //所属子图
	int data;         //节点编号
};


class Graph
{
public:

	Graph(){}
	Graph(int n, int e, int sub);
	void initial(string filename);
	void printInfo();

	int m_nodeNum;         //节点数量  
	int m_edgeNum;         //边数量
	int m_subNum;          //子图数量
	double** m_adjMatrix;     //邻接矩阵
	double** m_degMatrix;     //度数矩阵
	double** m_laplaceMatrix; //拉普拉斯矩阵
	int* m_belong;         //子图归属
};

Graph::Graph(int n, int e, int sub)
{
	m_nodeNum = n;
	m_edgeNum = e;
	m_subNum = sub;

	m_belong = new int[n];

	m_adjMatrix = new double*[n];
	for (int i = 0; i < n; i++)
		m_adjMatrix[i] = new double[n];

	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			m_adjMatrix[i][j] = 0;

	m_degMatrix = new double* [n];
	for (int i = 0; i < n; i++)
		m_degMatrix[i] = new double[n];

	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			m_degMatrix[i][j] = 0;

	m_laplaceMatrix = new double* [n];

	for (int i = 0; i < n; i++)
		m_laplaceMatrix[i] = new double[n];

	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			m_laplaceMatrix[i][j] = 0;
}


void initial(Graph *&g, string filename)
{
	ifstream ifs(filename);
	int nodenum, edgenum, subnum;
	ifs >> nodenum >> edgenum >> subnum;
	g = new Graph(nodenum, edgenum, subnum);

	g->m_nodeNum = nodenum;
	g->m_edgeNum = edgenum;
	g->m_subNum = subnum;

	int node1, node2;
	double weight;
	while (ifs >> node1 >> node2 >> weight)
	{
		g->m_adjMatrix[node1][node2] = weight;
		g->m_adjMatrix[node2][node1] = weight;
		g->m_degMatrix[node1][node1] += weight;
		g->m_degMatrix[node2][node2] += weight;
	}

	for (int i = 0; i < g->m_nodeNum; i++)
		for (int j = 0; j < g->m_nodeNum; j++)
			g->m_laplaceMatrix[i][j] = g->m_degMatrix[i][j] - g->m_adjMatrix[i][j];
}

void Graph::printInfo()
{
	cout << "adj Matrix " << endl;\
 
	for (int i = 0; i < m_nodeNum; i++)
	{
		for (int j = 0; j < m_nodeNum; j++)
		{
			cout << setw(4) << m_adjMatrix[i][j] << " ";
		}
		cout << endl;
	}

	cout << "deg Matrix " << endl;
	for (int i = 0; i < m_nodeNum; i++)
	{
		for (int j = 0; j < m_nodeNum; j++)
		{
			cout << setw(4) << m_degMatrix[i][j] << " ";
		}
		cout << endl;
	}

	cout << "laplace Matrix " << endl;
	for (int i = 0; i < m_nodeNum; i++)
	{
		for (int j = 0; j < m_nodeNum; j++)
		{
			cout << setw(4) << m_laplaceMatrix[i][j] << " ";
		}
		cout << endl;
	}
}


Eigen::MatrixXd ConvertToEigenMatrix(double** matrix, int n)
{
	std::vector<std::vector<double>> data;
	for (int i = 0; i < n; i++)
	{
		vector<double> temp;
		for (int j = 0; j < n; j++)
		{
			temp.push_back(matrix[i][j]);
		}
		data.push_back(temp);
	}

	Eigen::MatrixXd eMatrix(data.size(), data[0].size());
	for (int i = 0; i < data.size(); ++i)
		eMatrix.row(i) = Eigen::VectorXd::Map(&data[i][0], data[0].size());
	return eMatrix;
}