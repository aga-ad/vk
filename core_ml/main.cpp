
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;


typedef double num;

// Содержит индексы соседних точек
struct Neighbour {
    size_t left, right;
    num distance;
    Neighbour(size_t left, size_t right, num distance):
        left(left), right(right), distance(distance) {}
};

// Содержит индексы кластеров - потомков
struct Cluster {
    size_t left_child, right_child;
    Cluster (size_t id):
        left_child(id), right_child(id) {}
    Cluster(size_t left_child, size_t right_child):
        left_child(left_child), right_child(right_child) {}
};

// Содержит индекс корневого кластера и индексы самой левой и правой точкек кластера
struct ClusterInfo {
    size_t root, left_bound, right_bound;
    ClusterInfo(size_t id):
        root(id), left_bound(id), right_bound(id) {}
};

vector<Cluster> slc(const vector<num>& points) {
    // Сортируем индексы точек по возрастанию,
    // Получаем пары соседних точек, сортируем их по возрастанию расстояния
    vector<Cluster> clusters;
    vector<ClusterInfo> info;
    clusters.reserve(2 * points.size() - 1);
    info.reserve(points.size());
    for (size_t i = 0; i < points.size(); i++) {
        clusters.push_back(Cluster(i));
        info.push_back(ClusterInfo(i));
    }
    vector<size_t> ids(points.size());
    for (size_t i = 0; i < ids.size(); i++) {
        ids[i] = i;
    }
    sort(ids.begin(), ids.end(), [&points](size_t a, size_t b){return points[a] < points[b];});
    vector<Neighbour> neighbours;
    neighbours.reserve(ids.size() - 1);
    for (size_t i = 1; i < ids.size(); i++) {
        neighbours.push_back(Neighbour{ids[i - 1], ids[i], points[ids[i]] - points[ids[i - 1]]});
    }
    sort(neighbours.begin(), neighbours.end(), [](const Neighbour& a, const Neighbour& b){return a.distance < b.distance;});
    // Объединяем кластеры с минимальным расстоянием, поддерживаем info
    for (size_t i = 0; i < neighbours.size(); i++) {
        size_t l = neighbours[i].left;
        size_t r = neighbours[i].right;
        size_t new_cluster = clusters.size();
        clusters.push_back(Cluster(info[l].root, info[r].root));
        info[info[l].left_bound].root = new_cluster;
        info[info[r].right_bound].root = new_cluster;
        info[info[l].left_bound].right_bound = info[r].right_bound;
        info[info[r].right_bound].left_bound = info[l].left_bound;
    }
    return clusters;
}

int main() {
    vector<num> vec;
    num t;
    while (cin >> t) {
        vec.push_back(t);
    }
    auto clusters = slc(vec);
    for (size_t i = 0; i < clusters.size(); i++) {
        if (i < vec.size()) {
            cout << i << ": " << vec[i] << endl;
        } else {
            cout << i << ":  " << clusters[i].left_child << " " << clusters[i].right_child  << endl;
        }
    }
    return 0;
}
