//
// Created by martin on 17. 4. 2026.
//

#ifndef IOA_SEMESTRALKA_ALG_HPP
#define IOA_SEMESTRALKA_ALG_HPP

#include <limits>
#include <queue>

#include "fStar.hpp"

using namespace fStar;

struct IndexEncoder {
    map<int,int> id_to_index;
    map<int,int> index_to_id;

    public: IndexEncoder() {
        this->id_to_index = map<int,int>();
        this->index_to_id = map<int,int>();
    }
};

struct DM_Row {
private:
    float* row;
    IndexEncoder* index_encoder_;
public:
    DM_Row(float * row, IndexEncoder * index_encoder) {
        this->row =row;
        this->index_encoder_ = index_encoder;
    };

    const float operator [] (int from) {
        return row[index_encoder_->id_to_index[from]];
    }

};

class DistanceMatrix {
private:
    FStar* fstar;
    float** Distances;
    int _size;
    IndexEncoder* index_encoder_;

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
             for (fStar::FStarIterator::OutEdgeIterator it = fstar->begin_out_edges(index_encoder_->index_to_id[u_ind]); it != fstar->end_out_edges(index_encoder_->index_to_id[u_ind]); ++it) {
                 Edge edge = *it;
                 int v_ind = index_encoder_->id_to_index[ edge.to->id ];
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
        this->index_encoder_ = new IndexEncoder();

        for (int i = 0; i < fstar->sizeNodes(); i++) {
            Distances[i] = new float[fstar->sizeNodes()]();
            std::fill(Distances[i], Distances[i] + fstar->sizeNodes(), std::numeric_limits<float>::infinity());
        }
        this->_size = star->sizeNodes();

        auto star_node_end = this->fstar->end_nodes();
        int metrix_index = 0;
        for (auto node = this->fstar->begin_nodes(); node != star_node_end; ++node) {
            this->index_encoder_->index_to_id[metrix_index] = (*node)->id;
            this->index_encoder_->id_to_index[(*node)->id] = metrix_index;
            metrix_index++;
        }

        // for (fStar::FStarIterator::EdgeIterator it = star->begin_edges(); it != star->end_edges(); ++it) {
        //     fStar::Edge edge = *it;
        //     Distances[edge.from->id][edge.to->id] = edge.weight;
        // }

        for (int i = 0; i < fstar->sizeNodes(); i++) {
            dijkstra(i);
        }

    };

    DM_Row operator [] (int from) {

        return DM_Row( Distances[this->index_encoder_->id_to_index[from]], this->index_encoder_);
    }

    int size() const {
        return this->_size;
    }

    ~DistanceMatrix() {
        for (int i = 0; i < this->size(); i++) {
            delete[] Distances[i];
        }
        delete[] Distances;

        delete(this->index_encoder_);
    }


};



#endif //IOA_SEMESTRALKA_ALG_HPP
