#pragma once
#include "Graph.h"
#include "Kmeans.h"
#include<iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <queue>
using namespace std;

extern Graph* newGraph;

extern vector<int> clsRes;
extern double ratioRes;

void ratiocut(string filename)
{
    initial(newGraph, filename);
    MatrixXd eigenMat = ConvertToEigenMatrix(newGraph->m_laplaceMatrix, newGraph->m_nodeNum);
    EigenSolver<Eigen::MatrixXd> eigen_solver(eigenMat);
    MatrixXcd eValue = eigen_solver.eigenvalues();

    vector<pair<double, int>> vecEvalue;
    for (int i = 0; i < newGraph->m_nodeNum; i++)
        vecEvalue.push_back({ eValue(i, 0).real(),i });

    sort(vecEvalue.begin(), vecEvalue.end());

    MatrixXcd ev = eigen_solver.pseudoEigenvectors();
    int clusterNum = newGraph->m_subNum;
    KMeans km(clusterNum);

    vector<Pointnd> points(newGraph->m_nodeNum);
    for (int i = 0; i < newGraph->m_nodeNum; i++)
    {
        points[i].pointId = i;
        points[i].dimensions = newGraph->m_subNum;
        for (int j = 0; j < clusterNum; j++)
        {
            points[i].values.push_back(ev(i, vecEvalue[j].second).real());
        }
    }


    vector<Cluster> res = km.km_algorithm(points);
    vector<int> ptcls;

    ptcls.resize(newGraph->m_nodeNum);

    for (int i = 0; i < res.size(); i++)
    {
        for (int j = 0; j < res[i].points.size(); j++)
        {
            ptcls[res[i].points[j].pointId] = i;
        }
    }


    double ratiocnt = 0;
    for (int i = 0; i < clusterNum; i++)
    {
        double current = 0;
        vector<int> clsPointId;
        for (int j = 0; j < res[i].points.size(); j++)
            clsPointId.push_back(res[i].points[j].pointId);

        for (int j = 0; j < newGraph->m_nodeNum; j++)
        {
            int curcnt = 0;

            for (int k = 0; k < res[i].points.size(); k++)
                if (clsPointId[k] == j)  //说明该点在当前集合内部
                    curcnt++;

            if (curcnt == 0)  //说明在外部集合
            {
                for (int k = 0; k < clsPointId.size(); k++)
                    current += newGraph->m_adjMatrix[j][clsPointId[k]];
            }
        }
        ratiocnt += current / res[i].points.size();
    }

    ratiocnt = ratiocnt / 2;

    clsRes = ptcls;
    ratioRes = ratiocnt;
}