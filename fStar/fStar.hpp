//
// Created by martin on 16. 4. 2026.
//

#ifndef IOA_SEMESTRALKA_FSTAR_HPP
#define IOA_SEMESTRALKA_FSTAR_HPP
#include <map>
#include <string>
#include <vector>


using namespace std;
namespace fStar {
    struct coor {
        float x;
        float y;
    };

    //template <typedef node_id_type>
    struct Node : public coor {
        int id;
        string name;
    };

    struct Edge {
        Node* from;
        Node* to;
        float weight;
    };

    //template <typedef node_type>
    struct FStarEdgeEntry {
        Node* node_to;
        float weight;
    };

    struct FStarNodeEdges {
        Node* node_from;
        vector<FStarEdgeEntry>* _edges;
        FStarNodeEdges(){this->_edges = new vector<FStarEdgeEntry>();};
        ~FStarNodeEdges(){delete this->_edges;};
    };

    namespace FStarIterator {
        using namespace fStar;

        class NodeIterator {
            friend class FStar;
        private:
            map<int, FStarNodeEdges*>::iterator it;

        public:
            NodeIterator(map<int, FStarNodeEdges*>::iterator it) : it(it) {}

            virtual NodeIterator& operator++() {
                ++it; return *this;
            }

            virtual bool operator!=(const NodeIterator& other) const {
                return it != other.it;
            }

            virtual Node* operator*() const {
                return it->second->node_from;
            }
        };

        class EdgeIterator{
        private:
            map<int, FStarNodeEdges*>::iterator map_it;
            map<int, FStarNodeEdges*>::iterator map_end;
            vector<FStarEdgeEntry>::iterator vec_it;

            void advance_to_valid() {
                while (map_it != map_end) {
                    auto edges = map_it->second->_edges;
                    if (vec_it != edges->end()) {
                        return;
                    }
                    ++map_it;
                    if (map_it != map_end) {
                        vec_it = map_it->second->_edges->begin();
                    }
                }
            }

        public:
            EdgeIterator(
                map<int, FStarNodeEdges*>::iterator m_it, map<int, FStarNodeEdges*>::iterator m_end) : map_it(m_it), map_end(m_end)
            {
                if (map_it != map_end) {
                    vec_it = map_it->second->_edges->begin();
                    advance_to_valid();
                }
            }

            EdgeIterator& operator++() {
                ++vec_it;
                advance_to_valid();
                return *this;
            }

            bool operator!=(const EdgeIterator& other) const {
                return map_it != other.map_it ||
                       (map_it != map_end && vec_it != other.vec_it);
            }

            Edge operator*() const {
                FStarEdgeEntry edge_entry = *vec_it;
                return Edge({map_it->second->node_from, edge_entry.node_to, edge_entry.weight});
            }
        };
    };


    //template <typedef node_type>
    class FStar  {
    protected:
        map<int, FStarNodeEdges*>* Edges;
        //map<int, Node*>* Nodes;

        FStarNodeEdges* _findNodeEdges_encap(Node* node);
        FStarNodeEdges* _addNode_encap(Node* node);
        int _findEdgeEntryIndex_encap(Node* node_to, FStarNodeEdges* _fsr_edges);
    public:
        FStar();

        void addNode(Node* node);
        void addEdge(Node* from, Node* to, float weight);

        FStarIterator::NodeIterator begin_nodes() {
            return FStarIterator::NodeIterator(Edges->begin());
        }

        FStarIterator::NodeIterator end_nodes() {
            return FStarIterator::NodeIterator(Edges->end());
        }
        FStarIterator::EdgeIterator begin_edges() {
            return FStarIterator::EdgeIterator(Edges->begin(), Edges->end());
        }

        FStarIterator::EdgeIterator end_edges() {
            return FStarIterator::EdgeIterator(Edges->end(), Edges->end());
        }
        ~FStar();
    };
}







#endif //IOA_SEMESTRALKA_FSTAR_HPP
