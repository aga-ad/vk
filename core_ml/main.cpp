
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;

const size_t OUT = (size_t)-1;
typedef double num;

// Содержит индексы двух соседних кластеров
struct Neighbour {
    size_t left, right;
};

// left_child, right_child - индексы двух дочерних кластеров
// size - размер кластера
// center - центр кластера
struct Cluster {
    size_t left_child, right_child, size;
    num center;
    Cluster(size_t id, const vector<num>& points):
        left_child(id), right_child(id), size(1), center(points[id]) {}
    Cluster(size_t l, size_t r, const vector<Cluster>& clusters):
        left_child(l), right_child(r), size(clusters[l].size + clusters[r].size),
        center((clusters[l].center * clusters[l].size + clusters[r].center * clusters[r].size) / (clusters[l].size + clusters[r].size)) {}
};
// p - индекс родителя
// next_left, next_right - индекс соседних кластеров
struct ClusterInfo {
    size_t p, next_left, next_right;
    ClusterInfo(size_t l, size_t r): p(OUT), next_left(l), next_right(r) {}
    bool used() const {
        return p != OUT;
    }
};

vector<Cluster> slc(const vector<num>& points) {
    // Cоздается вектор кластеров, состоящих из одного элемента
    // ids[k] - k-порядковая статистика, pos[k] - позиция в отсортированном списке кластеров;
    // ids, pos используются для эффективного поиска соседей для каждого единичного кластера
    vector<size_t> ids(points.size());
    for (size_t i = 0; i < ids.size(); i++) {
        ids[i] = i;
    }
    sort(ids.begin(), ids.end(), [&points](size_t a, size_t b){return points[a] < points[b];});
    vector<size_t> pos(points.size());
    for (size_t i = 0; i < ids.size(); i++) {
        pos[ids[i]] = i;
    }
    vector<Cluster> clusters;
    vector<ClusterInfo> info;
    clusters.reserve(2 * points.size() - 1);
    info.reserve(2 * points.size() - 1);
    for (size_t i = 0; i < points.size(); i++) {
        size_t position = pos[i];
        size_t l, r;
        if (position > 0) {
            l = ids[position - 1];
        } else {
            l = OUT;
        }
        if (position + 1 < points.size()) {
            r = ids[position + 1];
        } else {
            r = OUT;
        }
        clusters.push_back(Cluster(i, points));
        info.push_back(ClusterInfo(l, r));
    }
    // Создается куча с парами соседних кластеров
    //
    // Используя кучу, достаем пару ближайших кластеров,
    // если каждый из них не был объединен, то объединяем их в новый кластер,
    // добавляем две новые пары соседних кластеров
    vector<Neighbour> neighbours;
    neighbours.reserve(ids.size() - 1);
    for (size_t i = 0; i + 1 < ids.size(); i++) {
        neighbours.push_back(Neighbour{ids[i], ids[i + 1]});
    }
    auto comp = [&clusters](const Neighbour& l, const Neighbour& r) {
        return clusters[l.right].center - clusters[l.left].center > clusters[r.right].center - clusters[r.left].center;
    };
    make_heap(neighbours.begin(), neighbours.end(), comp);
    while (neighbours.size() > 0) {
        Neighbour cur = neighbours.front();
        pop_heap(neighbours.begin(), neighbours.end(), comp);
        neighbours.pop_back();
        if (!info[cur.left].used() && !info[cur.right].used()) {
            clusters.push_back(Cluster(cur.left, cur.right, clusters));
            info.push_back(ClusterInfo(info[cur.left].next_left, info[cur.right].next_right));
            size_t new_cluster = clusters.size() - 1;
            info[cur.left].p = new_cluster;
            info[cur.right].p = new_cluster;
            if (info[cur.left].next_left != OUT) {
                info[info[cur.left].next_left].next_right = new_cluster;
                neighbours.push_back(Neighbour{info[cur.left].next_left, new_cluster});
                push_heap(neighbours.begin(), neighbours.end(), comp);
            }
            if (info[cur.right].next_right != OUT) {
                info[info[cur.right].next_right].next_left = new_cluster;
                neighbours.push_back(Neighbour{new_cluster, info[cur.right].next_right});
                push_heap(neighbours.begin(), neighbours.end(), comp);
            }
        }
    }
    return clusters;
}

int main() {
    std::uniform_real_distribution<double> unif(0, 1);
    std::default_random_engine re;
    vector<num> vec;
    for (size_t i = 0; i < (1 << 20); i++) {
        vec.push_back(unif(re));
    }
    auto start = std::chrono::high_resolution_clock::now();
    auto clusters = slc(vec);
    auto end = std::chrono::high_resolution_clock::now();
    auto time_taken_1 = end - start;
    std::cout << "slc took "
    << std::chrono::duration_cast<std::chrono::milliseconds>(time_taken_1).count() << " ms\n";
    //for (size_t i = 0; i < clusters.size(); i++) {
    //    cout << i << ":  " << clusters[i].l << " " << clusters[i].r << "  " << clusters[i].center << endl;
    //}
    return 0;
}
