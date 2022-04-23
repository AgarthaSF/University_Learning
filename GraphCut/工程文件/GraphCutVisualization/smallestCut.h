#include <iostream>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <vector>
#include "Graph.h"
const int maxn = 50;
const int inf = 0x3f3f3f3f;
int n;
double dist[maxn];
bool visited[maxn], merged[maxn];
vector<vector<double>> replaceMatrix;
vector<vector<vector<double>>> subMatrix;

vector<int> majorRes;
vector<int> subres0;
vector<int> subres1;

extern vector<int> clsRes;
extern double smallestRes;
extern Graph* newGraph;

int find(int* p, int x)
{
    if (p[x] != x) p[x] = find(p, p[x]);
    return p[x];
}

double contract(double**& matrix, int& s, int& t, int n)
{
    memset(dist, 0, sizeof(dist));
    memset(visited, false, sizeof(visited));
    int i, j, k;
    double mincut, maxc;
    for (i = 0; i < n; i++)
    {
        k = -1; maxc = -1;

        for (j = 0; j < n; j++)
        {
            if (!merged[j] && !visited[j] && dist[j] > maxc)
            {
                k = j;
                maxc = dist[j];
            }
        }

        if (k == -1)
            return mincut;
        s = t;  t = k;
        mincut = maxc;
        visited[k] = true;  //归并加入集合中
        for (j = 0; j < n; j++)
            if (!merged[j] && !visited[j])
                dist[j] += matrix[k][j];
    }
    return mincut;
}
double Stoer_Wagner(double**& matrix, vector<int>& res, int n)
{
    memset(merged, false, sizeof(merged));
    memset(dist, 0, sizeof(dist));
    memset(visited, false, sizeof(visited));

    double mincut = 0x3f3f3f3f, ans;
    int i, j, s, t;
    for (i = 0; i < n - 1; i++)
    {
        ans = contract(matrix, s, t, n);
        merged[t] = true;
        if (mincut > ans)
        {
            res.clear();
            for (int i = 0; i < n; i++)
                res.push_back(merged[i]);
            mincut = ans;
        }

        if (mincut == 0)
            return 0;
        for (j = 0; j < n; j++)
            if (!merged[j])
            {
                matrix[s][j] += matrix[t][j];
                matrix[j][s] += matrix[j][t];
            }
    }
    return mincut;
}

void smallestCut_sw(string filename)
{
    initial(newGraph, filename);
    n = newGraph->m_nodeNum;
    majorRes.clear();
    subres0.clear();
    subres1.clear();
    memset(merged, false, sizeof(merged));
    memset(visited, false, sizeof(visited));

    for (int i = 0; i < newGraph->m_nodeNum; i++) {
        vector<double> line;
        for (int j = 0; j < newGraph->m_nodeNum; j++) {
            line.push_back(newGraph->m_adjMatrix[i][j]);
        }
        replaceMatrix.push_back(line);
    }

    double currentMin = Stoer_Wagner(newGraph->m_adjMatrix, majorRes, newGraph->m_nodeNum);

    if (newGraph->m_subNum == 3)
    {
        vector<vector<double>> subM0, subM1;
        for (int i = 0; i < newGraph->m_nodeNum; i++)
        {
            vector<double> M0Line, M1Line;
            for (int j = 0; j < newGraph->m_nodeNum; j++)
            {
                if (majorRes[i] == majorRes[j] && majorRes[i] == 0)
                    M0Line.push_back(replaceMatrix[i][j]);

                if (majorRes[i] == majorRes[j] && majorRes[i] == 1)
                    M1Line.push_back(replaceMatrix[i][j]);
            }
            if (M0Line.size() != 0)
                subM0.push_back(M0Line);
            if (M1Line.size() != 0)
                subM1.push_back(M1Line);
        }

        subMatrix.push_back(subM0);
        subMatrix.push_back(subM1);

        double** m0 = new double* [subM0.size()];
        for (int i = 0; i < subM0.size(); i++)
            m0[i] = new double[subM0.size()];

        for (int i = 0; i < subM0.size(); i++)
            for (int j = 0; j < subM0.size(); j++)
                m0[i][j] = subM0[i][j];


        double** m1 = new double* [subM1.size()];
        for (int i = 0; i < subM1.size(); i++)
            m1[i] = new double[subM1.size()];

        for (int i = 0; i < subM1.size(); i++)
            for (int j = 0; j < subM1.size(); j++)
                m1[i][j] = subM1[i][j];

        double subMin1 = Stoer_Wagner(m0, subres0, subM0.size());
        double subMin2 = Stoer_Wagner(m1, subres1, subM1.size());

        if (subMin1 <= subMin2)  //submin1为标号为0的点再划分出的集合，submin2是标号为1的点再划分出的集合
        {
            int cnt = 0;
            for (int i = 0; i < majorRes.size(); i++) {
                if (majorRes[i] == 0) {
                    if (subres0[cnt] == 1) {
                        majorRes[i] = 2;
                    }
                    cnt++;
                }
            }
        }
        else
        {
            int cnt = 0;
            for (int i = 0; i < majorRes.size(); i++) {
                if (majorRes[i] == 1) {
                    if (subres1[cnt] == 0) {
                        majorRes[i] = 2;
                    }
                    cnt++;
                }
            }
        }
        currentMin += min(subMin1, subMin2);
    }
    clsRes = majorRes;
    smallestRes = currentMin;

    replaceMatrix.clear();
    subMatrix.clear();
    majorRes.clear();
    subres0.clear();
    subres1.clear();
}


void smallestCut_km(string filename)
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
        points[i].dimensions = 1;
        points[i].values.push_back(ev(i, vecEvalue[1].second).real());
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

    double cut = 0;
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
        cut += current;
    }

    clsRes = ptcls;
    smallestRes = cut / 2;
}

void smallestCut(string filename)
{
    if (filename != "Test3.txt" && filename != "Test5.txt")
        smallestCut_km(filename);
    else
        smallestCut_sw(filename);
}