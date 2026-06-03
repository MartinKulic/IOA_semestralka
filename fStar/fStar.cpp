//
// Created by martin on 16. 4. 2026.
//

#include "fStar.hpp"

#include <iostream>
#include <ostream>
#include <stdexcept>


using namespace fStar;

FStar::FStar() {
    //this->Nodes = new map<int, Node*>();
    this->Edges = new map<int, FStarNodeEdges*>();
}

FStarNodeEdges * FStar::_findNodeEdges_encap(Node *node) {
    auto it = this->Edges->find(node->id);
    if (it == this->Edges->end()) {
        return nullptr;
    }
    return it->second;
}

FStarNodeEdges * FStar::_addNode_encap(Node *node) {
    if (this->Edges->find(node->id) != this->Edges->end()) {
        throw std::invalid_argument("Node with id" + to_string(node->id) + " already exists.");
    }

    if (node->x > this->maxX) {
        this->maxX = node->x;
    }
    if (node->x < this->minX) {
        this->minX = node->x;
    }
    if (node->y > this->maxY) {
        this->maxY = node->y;
    }
    if (node->y < this->minY) {
        this->minY = node->y;
    }

    FStarNodeEdges* nodeRecord = new FStarNodeEdges();
    nodeRecord->node_from = node;
    (*this->Edges)[node->id] = nodeRecord;
    return nodeRecord;
}

/// Return: index of node_to in FStarNodeEdges _edges vector; -1 if not found
int FStar::_findEdgeEntryIndex_encap(Node *node_to, FStarNodeEdges *_fsr_edges) {
    int index = 0;
    for (FStarEdgeEntry edge: *_fsr_edges->_edges) {
        if (edge.node_to == node_to) {
            return index;
        }
    }
    return -1;
}

void FStar::addNode(Node *node) {
    this->_addNode_encap(node);
}

void FStar::deleteNode(int nodeFrom) {
    auto mapEntry = this->Edges->find(nodeFrom);
    if (mapEntry == this->Edges->end()) {
        //node is not in fStar
        return;
    }
    FStarNodeEdges* nodeEdgesToDelete = mapEntry->second;
    for (FStarEdgeEntry edge: *nodeEdgesToDelete->_edges) {
        Node* neigbour = edge.node_to;
        auto mapEntryN = this->Edges->find(neigbour->id);
        if (mapEntryN == this->Edges->end()) {
            //node is not in fStar
            continue;
        }
        FStarNodeEdges* nodeEdgesN = mapEntryN->second;
        nodeEdgesN->deleteEdge(nodeEdgesToDelete->node_from);
        this->numEdges--;
    }
    this->numEdges -= nodeEdgesToDelete->_edges->size();

    Node* nodeToDelete = nodeEdgesToDelete->node_from;
    this->Edges->erase(mapEntry);
    delete nodeEdgesToDelete;
    //delete nodeToDelete;
}

void FStar::addEdge(Node *from, Node *to, float weight, bool oneway) {
    FStarNodeEdges* _fsr_nodeFromEdges = this->_findNodeEdges_encap(from);
    if (_fsr_nodeFromEdges == nullptr) {
        //Node does not exist jet
        _fsr_nodeFromEdges = this->_addNode_encap(from);
    }

    FStarNodeEdges* _fsr_nodeToEdges = this->_findNodeEdges_encap(to);
    if (_fsr_nodeToEdges == nullptr) {
        _fsr_nodeToEdges = this->_addNode_encap(to);
    }

    int edgeIndex = this->_findEdgeEntryIndex_encap(to, _fsr_nodeFromEdges);
    if (edgeIndex != -1) {
        std::cerr << "Edge from " << _fsr_nodeFromEdges->node_from->id << " to " << to->id << " already exists. Ignoring error rewriting value" << std::endl;
        _fsr_nodeFromEdges->_edges->erase(_fsr_nodeFromEdges->_edges->begin() + edgeIndex);
        this->numEdges--;
    }

    FStarEdgeEntry edge = FStarEdgeEntry();
    edge.node_to = to;
    edge.weight = weight;

    _fsr_nodeFromEdges->_edges->push_back(edge);
    this->numEdges++;
    if (!oneway) {
        this->addEdge(to,from,weight, true);
    }
}

void FStar::deleteEdge(int fromNodeId, int toNodeId, bool oneway) {
    auto mapEntryFrom = this->Edges->find(fromNodeId);
    if (mapEntryFrom == this->Edges->end()) {
        return; //node is not in fStar
    }
    auto mapEntryTo = this->Edges->find(toNodeId);
    if (mapEntryTo == this->Edges->end()) {
        return; //node is not in fStar
    }

    FStarNodeEdges* nodeEdgesFrom = mapEntryFrom->second;
    FStarNodeEdges* nodeEdgesTo = mapEntryTo->second;
    nodeEdgesFrom->deleteEdge(nodeEdgesTo->node_from);
    numEdges--;

    if (!oneway) {
        nodeEdgesTo->deleteEdge(nodeEdgesFrom->node_from);
        numEdges--;
    }

}

void FStar::modifieEdge(int from, int to, float newWeight, bool oneway) {
    auto mapEntryFrom = this->Edges->find(from);
    if (mapEntryFrom == this->Edges->end()) {
        return; //node is not in fStar
    }
    FStarNodeEdges* fromEntry = mapEntryFrom->second;
    FStarEdgeEntry* edgeEntryToMod = nullptr;
     for (FStarEdgeEntry& edge: *fromEntry->_edges) {
        if (edge.node_to->id == to) {
            edgeEntryToMod = &edge;
            break;
        }
     }
    if (edgeEntryToMod == nullptr) {
        return; //node is not found
    }
    (edgeEntryToMod->weight) = newWeight;

    if (!oneway) {
        this->modifieEdge(to, from, newWeight, true);
    }
}

void FStar::nuke() {
    for (auto const& rec: *Edges) {
        //rec.first //key
        //rec.second //value

        auto node_edges = rec.second;
        //delete((node_edges)->_edges);
        //delete(node_edges)->node_from;
        delete(node_edges);
    }
    Edges->clear();
}


FStar::~FStar() {
    for (auto const& rec: *Edges) {
        //rec.first //key
        //rec.second //value

        auto node_edges = rec.second;
        //delete((node_edges)->_edges);
        //delete(node_edges)->node_from;
        delete(node_edges);
    }

    delete(Edges);
    // for (auto const& rec : *Nodes) {
    //     auto node = rec.second;
    //     delete(node);
    // }
}
