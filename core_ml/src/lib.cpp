#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>

using namespace std;

const size_t OUT = (size_t)-1;
typedef double num;

struct Neighbour {
    size_t l, r;
};

struct Cluster {
    size_t l, r, p, ll, rr;
    num center;
    Cluster(size_t id, size_t ll, size_t rr, const vector<num>& points): l(id), r(id), p(OUT), ll(ll), rr(rr), center(points[id]) {}
    Cluster(size_t l, size_t r, size_t ll, size_t rr, const vector<Cluster>& clusters): l(l), r(r), p(OUT), ll(ll), rr(rr), center((clusters[l].center + clusters[r].center) / 2) {}
    bool used() const {
        return p != OUT;
    }
};

vector<Cluster> slc(const vector<num>& points) {
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
    clusters.reserve(2 * points.size() - 1);
    for (size_t i = 0; i < points.size(); i++) {
        size_t position = pos[i];
        size_t ll, rr;
        if (position > 0) {
            ll = ids[position - 1];
        } else {
            ll = OUT;
        }
        if (position + 1 < points.size()) {
            rr = ids[position + 1];
        } else {
            rr = OUT;
        }
        clusters.push_back(Cluster(i, ll, rr, points));
    }

    vector<Neighbour> neighbours;
    neighbours.reserve(ids.size() - 1);
    for (size_t i = 0; i + 1 < ids.size(); i++) {
        neighbours.push_back(Neighbour{ids[i], ids[i + 1]});
    }

    auto comp = [&clusters](const Neighbour& l, const Neighbour& r) {
        return clusters[l.r].center - clusters[l.l].center > clusters[r.r].center - clusters[r.l].center;
    };

    make_heap(neighbours.begin(), neighbours.end(), comp);

    while (neighbours.size() > 0) {
        //cout << clusters.size() << " " << (long long)neighbours.size() << endl;
        Neighbour cur = neighbours.front();
        pop_heap(neighbours.begin(), neighbours.end(), comp);
        neighbours.pop_back();
        if (!clusters[cur.l].used() && !clusters[cur.r].used()) {
            clusters.push_back(Cluster(cur.l, cur.r, clusters[cur.l].ll, clusters[cur.r].rr, clusters));
            size_t new_cluster = clusters.size() - 1;
            clusters[cur.l].p = new_cluster;
            clusters[cur.r].p = new_cluster;
            clusters[clusters[cur.l].ll].rr = new_cluster;
            clusters[clusters[cur.r].rr].ll = new_cluster;

            if (clusters[cur.l].ll != OUT) {
                neighbours.push_back(Neighbour{clusters[cur.l].ll, new_cluster});
                push_heap(neighbours.begin(), neighbours.end(), comp);
            }
            if (clusters[cur.r].r != OUT) {
                neighbours.push_back(Neighbour{new_cluster, clusters[cur.r].rr});
                push_heap(neighbours.begin(), neighbours.end(), comp);
            }
        }
    }
    return clusters;
}

int main() {
    vector<num> vec;
    for (size_t i = 0; i < (1 << 24); i++) {
        vec.push_back(i);
    }
    auto start = std::chrono::high_resolution_clock::now();
    auto clusters = slc(vec);
    auto end = std::chrono::high_resolution_clock::now();
    auto time_taken_1 = end - start;
    std::cout << "Forward transform took "
    << std::chrono::duration_cast<std::chrono::milliseconds>(time_taken_1).count() << " ms\n";
    /*for (size_t i = 0; i < clusters.size(); i++) {
        cout << i << ":  " << clusters[i].l << " " << clusters[i].r << "  " << clusters[i].center << endl;
    }*/
    return 0;
}
