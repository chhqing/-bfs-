#include <iostream>
#include <algorithm>
#include <omp.h>
#include <string>
#include <vector>
#include <queue>
#include <time.h>
#include <chrono>
#include <fstream>
using namespace std;

vector<vector<int>> getgraph(int n, int m) {
	srand(time(0));
	vector<vector<int>> graph(n);
	for (int i = 0; i < n; i++) {
		int nn = rand() % m + 1 - graph[i].size();
		int j = 0;
		while (j < nn) {
			int near = rand() % n;
			if (near == i || find(graph[i].begin(), graph[i].end(), near) != graph[i].end())continue;
			else {
				graph[i].push_back(near);
				graph[near].push_back(i);
				j++;
			}
		}
	}
	return graph;
}

vector<int> bfs(vector<vector<int>>& graph) {//串行的bfs
	vector<int> dis(graph.size(), -1);
	queue<int> bqueue;
	dis[0] = 0;
	bqueue.push(0);
	while (!bqueue.empty()) {
		int s = bqueue.size();
		int node = 0;
		for (int i = 0; i < s; i++) {
			node = bqueue.front();
			bqueue.pop();
			for (int j : graph[node]) {
				if (dis[j] == -1) {
					bqueue.push(j);
					dis[j] = dis[node] + 1;
				}
			}
		}
	}
	return dis;
}

vector<int> openmp_bfs(vector<vector<int>> graph,int numcore) {//并行的bfs
	vector<int> dis(graph.size(), -1);
	queue<int> bqueue;
	dis[0] = 0;
	bqueue.push(0);
	omp_set_num_threads(numcore);
	omp_lock_t changeq, changedis;
	omp_init_lock(&changeq);
	omp_init_lock(&changedis);
	while (!bqueue.empty()) {
		int s = bqueue.size();
		int node=0;
#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < s; i++) {
			omp_set_lock(&changeq);
			node = bqueue.front();
			bqueue.pop();
			//cout << "thread:" << omp_get_thread_num() << "node: " << node << endl;
			omp_unset_lock(&changeq);
			for (int j :graph[node]) {
				if (dis[j] == -1) {
					omp_set_lock(&changeq);
        			bqueue.push(j);
					omp_unset_lock(&changeq);
					omp_set_lock(&changedis);
					dis[j] = dis[node] + 1;
					omp_unset_lock(&changedis);
				}
			}
		}
	}
	return dis;
}

int main() {
	//int n, m;
	//cin >> n >> m;
	//int n = 1000000, m = 500;
	int n = 50000, m = 5000;
	vector<vector<int>> graph = getgraph(n, m);
	/*fstream f;
	f.open("data.txt", ios::out | ios::app);
	for (int i = 0; i < graph.size(); i++) {
		for (int j = 0; j < graph[i].size(); j++) {
			f << i << " " << graph[i][j] << endl;;
		}
	}
	f.close();*/
	/*vector<vector<int>> graph(n);
	fstream f;
	f.open("data.txt", ios::in);
	int num1, num2;
	while (f >> num1 >> num2) {//读取数据
		graph[num1].push_back(num2);
	}
	f.close();*/
	auto start1 = chrono::steady_clock::now();
	vector<int>dis1 = bfs(graph);
	auto end1 = chrono::steady_clock::now();
	double ms1 = chrono::duration<double, milli>(end1 - start1).count();
	cout << "serial bfs: " << ms1 << "ms" << endl;

	int numcore = 8;
	auto start2 = chrono::steady_clock::now();
	vector<int>dis2 = openmp_bfs(graph, numcore);
	auto end2 = chrono::steady_clock::now();
	double ms2 = chrono::duration<double, milli>(end2 - start2).count();
	cout << "parallel bfs with " << numcore << " cores: " << ms2 << "ms" << endl;

	/*for (int i = 0; i < dis1.size(); i++) {
		cout << dis1[i] << " ";
	}
	cout << endl;
	for (int i = 0; i < dis2.size(); i++) {
		cout << dis2[i] << " ";
	}
	cout << endl;*/
	int flag = 0;
	int count = 0;
	for (int i = 0; i < graph.size(); i++) {
		if (dis1[i] != dis2[i]) {
			flag = 1;
			count++;
		}
	}
	cout << "error rate:" << count / 100000 << endl;
	if (!flag)cout << "distance same, parallel bfs right" << endl;
	else cout << "parallel bfs error" << endl;
}