#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <Python.h>
#include "Numeric/arrayobject.h"

using namespace std;

const size_t OUT = (size_t)-1;
typedef double num;

// Содержит индексы двух соседних кластеров
struct Neighbour {
    size_t l, r;
};

// l, r - индексы двух дочерних кластеров
// p - индекс родителя
// ll, rr - индекс соседних кластеров
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
        return clusters[l.r].center - clusters[l.l].center > clusters[r.r].center - clusters[r.l].center;
    };
    make_heap(neighbours.begin(), neighbours.end(), comp);
    while (neighbours.size() > 0) {
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

/*int main() {
    vector<num> vec;
    for (size_t i = 0; i < (1 << 20); i++) {
        vec.push_back(i);
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
}*/


static PyObject *
example_wrapper(PyObject *dummy, PyObject *args)
{
    PyObject *arg1=NULL, *arg2=NULL, *out=NULL;
    PyObject *arr1=NULL, *arr2=NULL, *oarr=NULL;

    if (!PyArg_ParseTuple(args, "OOO!", &arg1, &arg2,
        &PyArray_Type, &out)) return NULL;

    arr1 = PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
    if (arr1 == NULL) return NULL;
    arr2 = PyArray_FROM_OTF(arg2, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
    if (arr2 == NULL) goto fail;
#if NPY_API_VERSION >= 0x0000000c
    oarr = PyArray_FROM_OTF(out, NPY_DOUBLE, NPY_ARRAY_INOUT_ARRAY2);
#else
    oarr = PyArray_FROM_OTF(out, NPY_DOUBLE, NPY_ARRAY_INOUT_ARRAY);
#endif
    if (oarr == NULL) goto fail;

    /* code that makes use of arguments */
    /* You will probably need at least
       nd = PyArray_NDIM(<..>)    -- number of dimensions
       dims = PyArray_DIMS(<..>)  -- npy_intp array of length nd
                                     showing length in each dim.
       dptr = (double *)PyArray_DATA(<..>) -- pointer to data.

       If an error occurs goto fail.
     */

    Py_DECREF(arr1);
    Py_DECREF(arr2);
#if NPY_API_VERSION >= 0x0000000c
    PyArray_ResolveWritebackIfCopy(oarr);
#endif
    Py_DECREF(oarr);
    Py_INCREF(Py_None);
    return Py_None;

 fail:
    Py_XDECREF(arr1);
    Py_XDECREF(arr2);
#if NPY_API_VERSION >= 0x0000000c
    PyArray_DiscardWritebackIfCopy(oarr);
#endif
    Py_XDECREF(oarr);
    return NULL;
}
