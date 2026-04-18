//
// Created by martin on 17. 4. 2026.
//

#ifndef IOA_SEMESTRALKA_ALG_HPP
#define IOA_SEMESTRALKA_ALG_HPP

#include <limits>
#include <queue>

#include "fStar.hpp"

using namespace fStar;
class DistanceMatrix {
private:
    FStar* fstar;
    float** Distances;

    struct index_weight_holder {
        float weight;
        int index;

        bool operator()(const index_weight_holder& a, const index_weight_holder& b) {
            return a.weight > b.weight;
        }
    };
    // struct Compare {
    //     bool operator()(const index_weight_holder& a, const index_weight_holder& b) {
    //         return a.weight > b.weight;
    //     }
    // };

     void dijkstra(int from_index) {
         priority_queue<index_weight_holder, std::vector<index_weight_holder>, index_weight_holder> pq;
         (Distances[from_index][from_index]) = 0.;
         pq.push({0., from_index});

         while (!pq.empty()) {
             index_weight_holder cur = pq.top();
             pq.pop();
             int u_ind = cur.index;

             //if the popped distance is greater than the recorded distance for this vertex(dist[u]), it means
             //this vertex has already been processed with a smaller distance, so skip it and continue to the next iteration.
             if (cur.weight > this->Distances[from_index][u_ind]) {
                 continue;
             }

             //For each neighbor v of u, check if the path through u gives a smaller distance than the current dist[v].
             //If it does, update dist[v] = dist[u] + edge weight(d) and push (dist[v], v) into the priority queue.
             for (fStar::FStarIterator::OutEdgeIterator it = fstar->begin_out_edges(u_ind); it != fstar->end_out_edges(u_ind); ++it) {
                 Edge edge = *it;
                 int v_ind = edge.to->id;
                 float newDistance = this->Distances[from_index][u_ind] + edge.weight;

                 if (newDistance < this->Distances[from_index][v_ind] ) {
                     Distances[from_index][v_ind] = newDistance;
                     pq.push({Distances[from_index][v_ind], v_ind});
                 }
             }
         }
    }

public:
    DistanceMatrix(FStar* star) : fstar(star) {
        Distances = new float*[fstar->sizeNodes()];
        for (int i = 0; i < fstar->sizeNodes(); i++) {
            Distances[i] = new float[fstar->sizeNodes()]();
            std::fill(Distances[i], Distances[i] + fstar->sizeNodes(), std::numeric_limits<float>::infinity());
        }

        // for (fStar::FStarIterator::EdgeIterator it = star->begin_edges(); it != star->end_edges(); ++it) {
        //     fStar::Edge edge = *it;
        //     Distances[edge.from->id][edge.to->id] = edge.weight;
        // }

        for (int i = 0; i < fstar->sizeNodes(); i++) {
            dijkstra(i);
        }

    };

    const float* operator [] (int from) {
        return Distances[from];
    }

    ~DistanceMatrix() {
        for (int i = 0; i < fstar->sizeNodes(); i++) {
            delete[] Distances[i];
        }
        delete[] Distances;
    }


};



#endif //IOA_SEMESTRALKA_ALG_HPP
