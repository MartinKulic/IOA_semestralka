//
// Created by martin on 16. 4. 2026.
//

#ifndef IOA_SEMESTRALKA_FSTAR_HPP
#define IOA_SEMESTRALKA_FSTAR_HPP
#include <map>
#include <string>
#include <vector>

using namespace std;
//template <typedef node_id_type>
struct Node {
    int x;
    int y;
    int id;
    string name;
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

    ~FStar();
};

#endif //IOA_SEMESTRALKA_FSTAR_HPP
