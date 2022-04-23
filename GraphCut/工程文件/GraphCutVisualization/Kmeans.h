#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;


class Pointnd
{
public:
	vector<double> values;
	int dimensions;
	int pointId;
	int clusterId = -1;
};

class Cluster
{
public:
	int clusterId;
	vector<double> center;
	vector<Pointnd> points;

	Cluster(int id, Pointnd center)
	{
		this->clusterId = id;
		for (int i = 0; i < center.dimensions; i++)
			this->center.push_back(center.values[i]);
		points.push_back(center);
	}

	void addPoint(Pointnd point){
		point.clusterId = this->clusterId;
		points.push_back(point);
	}

	void removePoint(Pointnd point)
	{
		for (int i = 0; i < points.size(); i++)
			if (points[i].pointId == point.pointId)
				points.erase(points.begin() + i);
	}

	void updateCenter()
	{
		if (points.size() == 0)
			return;

		for (int i = 0; i < points[0].dimensions; i++)
		{
			double sum = 0;
			for (int j = 0; j < points.size(); j++)
				sum += points[j].values[i];		
			center[i] = sum / points.size();
		}
	}
};

class KMeans
{
public:
	vector<Cluster> km_clusters;
	int clustersNum;  //the parameter K
	int dimensions;

	KMeans(int K)
	{
		clustersNum = K;
	}


	int getNearCluster(Pointnd point)
	{
		double dict = 0;
		double mindict = 100000;
		int nearestId = 0;

		for (int i = 0; i < clustersNum; i++){
			double sum = 0;
			for (int j = 0; j < dimensions; j++)
				sum += pow(km_clusters[i].center[j] - point.values[j], 2);

			dict = sqrt(sum);
			if (dict < mindict){
				mindict = dict;
				nearestId = km_clusters[i].clusterId;
			}
		}
		return nearestId;
	}

	vector<Cluster> km_algorithm(vector<Pointnd>& points)
	{
		srand(20);
		int points_size = points.size();
		this->dimensions = points[0].values.size();
		//��ʼ����������
		vector<int> used_points;

		for (int i = 0; i < clustersNum; i++)
		{
			int index = 0;
			bool findFlag = false;
			while (!findFlag)
			{
				index = rand() % points_size;

				if (used_points.size() == 0)
					findFlag = true;

				int cnt = 0;
				for (int i = 0; i < used_points.size(); i++)
					if (index == used_points[i])
						cnt++;

				if (cnt == 0)
					findFlag = true;
			}
			used_points.push_back(index);

			points[index].clusterId = i;
			Cluster cluster(i, points[index]);
			km_clusters.push_back(cluster);
		}

		//��������������ľ��࣬�����¾������ģ��ٴε���ֱ������
		bool converge_flag = false;
		while (!converge_flag)
		{
			converge_flag = true;

			//�����������ľ���
			for (int i = 0; i < points.size(); i++)
			{
				int nearestId = getNearCluster(points[i]);
				int currentId = points[i].clusterId;

				if (currentId != nearestId)
				{
					converge_flag = false;  //�����Ĺ����и���˵���㷨δ����
					if (currentId != -1) {  //�����Ϊ-1˵��֮ǰ�õ�λ�����������У��Ƚ��õ��֮ǰ�ĵ����Ƴ�
						for (int j = 0; j < clustersNum; j++)
							if (km_clusters[j].clusterId == currentId)
								km_clusters[j].removePoint(points[i]);
					}

					for (int j = 0; j < clustersNum; j++)
						if (km_clusters[j].clusterId == nearestId)
							km_clusters[j].addPoint(points[i]);

					points[i].clusterId = nearestId;
				}
			}

			//���µ�����������
			for (int i = 0; i < clustersNum; i++)
				km_clusters[i].updateCenter();
		}
		return km_clusters;
	}
};