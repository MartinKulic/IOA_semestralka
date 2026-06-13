//
// Created by martin on 16. 4. 2026.
//

#ifndef IOA_SEMESTRALKA_FSTAR_HPP
#define IOA_SEMESTRALKA_FSTAR_HPP
#include <limits>
#include <map>
#include <string>
#include <vector>
#include <type_traits>


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

    template<typename Tnode>
    struct Edge {
        Tnode* from;
        Tnode* to;
        float weight;
    };

    template <typename Tnode>
    struct FStarEdgeEntry {
        Tnode* node_to;
        float weight;
    };

    template <typename Tnode>
    struct FStarNodeEdges {
        Tnode* node_from;
        vector<FStarEdgeEntry<Tnode>>* _edges;
        FStarNodeEdges(){this->_edges = new vector<FStarEdgeEntry<Tnode>>();};
        ~FStarNodeEdges(){delete this->_edges;};

        void deleteEdge(Tnode* to) {
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

        template<typename Tnode>
        class NodeIterator {
            friend class FStar;
        private:
            typename map<int, FStarNodeEdges<Tnode>*>::iterator it;

        public:
            NodeIterator(typename map<int, FStarNodeEdges<Tnode>*>::iterator it) : it(it) {}

            virtual NodeIterator& operator++() {
                ++it;
                return *this;
            }

            virtual bool operator!=(const NodeIterator& other) const {
                return it != other.it;
            }

            virtual Tnode* operator*() const {
                return it->second->node_from;
            }
        };

        template<typename Tnode>
        class EdgeIterator{
        private:
            typename map<int, FStarNodeEdges<Tnode>*>::iterator map_it;
            typename map<int, FStarNodeEdges<Tnode>*>::iterator map_end;
            typename vector<FStarEdgeEntry<Tnode>>::iterator vec_it;

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
                typename map<int, FStarNodeEdges<Tnode>*>::iterator m_it, typename map<int, FStarNodeEdges<Tnode>*>::iterator m_end) : map_it(m_it), map_end(m_end)
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

            Edge<Tnode> operator*() const {
                FStarEdgeEntry edge_entry = *vec_it;
                return Edge<Tnode>({map_it->second->node_from, edge_entry.node_to, edge_entry.weight});
            }
        };

        template<typename Tnode>
        class OutEdgeIterator {
        private:
            typename vector<FStarEdgeEntry<Tnode>>::iterator it;
            Tnode* node_from;
        public:
            OutEdgeIterator(typename vector<FStarEdgeEntry<Tnode>>::iterator iterator, Tnode* from) : it(iterator), node_from(from) {};

            virtual OutEdgeIterator& operator++() {
                ++it;
                return *this;
            }

            virtual bool operator!=(const OutEdgeIterator& other) const {
                return it != other.it;
            }

            virtual Edge<Tnode> operator*() const {
                return Edge<Tnode>({node_from, it->node_to, it->weight});
            }
        };
    };


    //template <typedef node_type>
    template <typename Tnode>
    class FStar  {
        static_assert(std::is_base_of<Tnode, Tnode>::value, "Tnode must be Node or derived from it");
    protected:
        map<int, FStarNodeEdges<Tnode>*>* Edges;
        //map<int, Node*>* Nodes;
        float minX = std::numeric_limits<float>::infinity();
        float maxX = -std::numeric_limits<float>::infinity();
        float minY = std::numeric_limits<float>::infinity();
        float maxY = -std::numeric_limits<float>::infinity();
        size_t numEdges = 0;

        FStarNodeEdges<Tnode>* _findNodeEdges_encap(Tnode* node);
        FStarNodeEdges<Tnode>* _addNode_encap(Tnode* node);
        int _findEdgeEntryIndex_encap(Tnode* node_to, FStarNodeEdges<Tnode>* _fsr_edges);
    public:
        FStar();

        void addNode(Tnode* node);
        void addEdge(Tnode* from, Tnode* to, float weight, bool oneway=false);
        void deleteNode(int nodeId);
        void deleteEdge(int fromNodeId, int toNodeId, bool oneway=false);
        void modifieEdge(int from, int to, float newWeight, bool oneway=false);
        void nuke();

        Tnode* getNode(int id){return (*Edges)[id]->node_from;};

        size_t sizeNodes(){return this->Edges->size();}
        size_t sizeEdges(){return numEdges;};

        float getMaxX(){return maxX;};
        float getMinX(){return minX;};
        float getMaxY(){return maxY;};
        float getMinY(){return minY;};


        FStarIterator::NodeIterator<Tnode> begin_nodes() {
            return FStarIterator::NodeIterator<Tnode>(Edges->begin());
        }
        FStarIterator::NodeIterator<Tnode> end_nodes() {
            return FStarIterator::NodeIterator<Tnode>(Edges->end());
        }

        FStarIterator::EdgeIterator<Tnode> begin_edges() {
            return FStarIterator::EdgeIterator<Tnode>(Edges->begin(), Edges->end());
        }
        FStarIterator::EdgeIterator<Tnode> end_edges() {
            return FStarIterator::EdgeIterator<Tnode>(Edges->end(), Edges->end());
        }

        FStarIterator::OutEdgeIterator<Tnode> begin_out_edges(int node_from_id) {
            FStarNodeEdges<Tnode>* _node_edg = (*this->Edges)[node_from_id];

            return FStarIterator::OutEdgeIterator<Tnode>(_node_edg->_edges->begin(), _node_edg->node_from);
        }
        FStarIterator::OutEdgeIterator<Tnode> end_out_edges(int node_from_id) {
            FStarNodeEdges<Tnode>* _node_edg = (*this->Edges)[node_from_id];

            return FStarIterator::OutEdgeIterator<Tnode>(_node_edg->_edges->end(), _node_edg->node_from);
        }

        ~FStar();
    };
}

#include "fStar.tpp"

#endif //IOA_SEMESTRALKA_FSTAR_HPP
