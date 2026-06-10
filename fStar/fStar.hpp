//
// Created by martin on 16. 4. 2026.
//

#ifndef IOA_SEMESTRALKA_FSTAR_HPP
#define IOA_SEMESTRALKA_FSTAR_HPP
#include <limits>
#include <map>
#include <string>
#include <vector>


using namespace std;
namespace fStar {
    struct coor {
        float x;
        float y;

        coor operator+(coor other) const {
            return coor{x+other.x, y+other.y};
        }

        coor operator*(int multiplier) const{
            return coor{x*multiplier, y*multiplier};
        }
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

        void deleteEdge(Node* to) {
            //std::erase_if(this->_edges, [to](FStarEdgeEntry cur){return cur.node_to == to;});
            int index = 0;
            for (FStarEdgeEntry edge : *this->_edges) {
                if (edge.node_to == to) {
                    break;
                }
                index++;
            }
            if (_edges->at(index).node_to != to) {
                return;
                //not found
            }

            this->_edges->erase(this->_edges->begin() + index);

        }
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
                ++it;
                return *this;
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

        class OutEdgeIterator {
        private:
            vector<FStarEdgeEntry>::iterator it;
            Node* node_from;
        public:
            OutEdgeIterator(vector<FStarEdgeEntry>::iterator iterator, Node* from) : it(iterator), node_from(from) {};

            virtual OutEdgeIterator& operator++() {
                ++it;
                return *this;
            }

            virtual bool operator!=(const OutEdgeIterator& other) const {
                return it != other.it;
            }

            virtual Edge operator*() const {
                return Edge({node_from, it->node_to, it->weight});
            }
        };
    };


    //template <typedef node_type>
    class FStar  {
    protected:
        map<int, FStarNodeEdges*>* Edges;
        //map<int, Node*>* Nodes;
        float minX = std::numeric_limits<float>::infinity();
        float maxX = -std::numeric_limits<float>::infinity();
        float minY = std::numeric_limits<float>::infinity();
        float maxY = -std::numeric_limits<float>::infinity();
        size_t numEdges = 0;

        FStarNodeEdges* _findNodeEdges_encap(Node* node);
        FStarNodeEdges* _addNode_encap(Node* node);
        int _findEdgeEntryIndex_encap(Node* node_to, FStarNodeEdges* _fsr_edges);
    public:
        FStar();

        void addNode(Node* node);
        void addEdge(Node* from, Node* to, float weight, bool oneway=false);
        void deleteNode(int nodeId);
        void deleteEdge(int fromNodeId, int toNodeId, bool oneway=false);
        void modifieEdge(int from, int to, float newWeight, bool oneway=false);
        void nuke();

        Node* getNode(int id){return (*Edges)[id]->node_from;};

        size_t sizeNodes(){return this->Edges->size();}
        size_t sizeEdges(){return numEdges;};

        float getMaxX(){return maxX;};
        float getMinX(){return minX;};
        float getMaxY(){return maxY;};
        float getMinY(){return minY;};


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

        FStarIterator::OutEdgeIterator begin_out_edges(int node_from_id) {
            FStarNodeEdges* _node_edg = (*this->Edges)[node_from_id];

            return FStarIterator::OutEdgeIterator(_node_edg->_edges->begin(), _node_edg->node_from);
        }
        FStarIterator::OutEdgeIterator end_out_edges(int node_from_id) {
            FStarNodeEdges* _node_edg = (*this->Edges)[node_from_id];

            return FStarIterator::OutEdgeIterator(_node_edg->_edges->end(), _node_edg->node_from);
        }

        ~FStar();
    };
}







#endif //IOA_SEMESTRALKA_FSTAR_HPP
