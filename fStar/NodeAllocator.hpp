//
// Created by ja on 31/05/2026.
//

#ifndef IOA_SEMESTRALKA_NodeAllocator_H
#define IOA_SEMESTRALKA_NodeAllocator_H



#include <filesystem>
#import "fStar.hpp"

class NodeAllocator {
    private:
    map<int,fStar::Node*> allocatedNodes  = map<int,fStar::Node*>();
    int nextID = 0;

public:
    NodeAllocator(){};
    ~NodeAllocator() {
        for (auto it = allocatedNodes.begin(); it != allocatedNodes.end(); ++it) {
            delete (*it).second;
        }
    };

    fStar::Node* MakeNode(std::string name, float x, float y, bool is_center) {
        fStar::Node* node = new fStar::Node;
        node->name=name;
        node->x=x;
        node->y=y;
        node->id=nextID++;
        node->is_center = is_center;
        node->belongs_to_p_group = fStar::Node::NO_GROUP;
        allocatedNodes[node->id] = node;

        return node;
    }
    fStar::Node* MakeNode(std::string name, float x, float y, int id, bool is_center) {
        fStar::Node* node = new fStar::Node;
        node->name=name;
        node->x=x;
        node->y=y;
        node->id=id;
        node->is_center = is_center;
        node->belongs_to_p_group = fStar::Node::NO_GROUP;
        allocatedNodes[node->id] = node;

        this->nextID = max(this->nextID, id+1);

        return node;
    }

    void DestroyNode(int node_id) {
        fStar::Node* node = allocatedNodes[node_id];
        allocatedNodes.erase(node_id);

        delete node;
    }

    fStar::Node* operator[](int node_id) {
        return allocatedNodes[node_id];
    }

    void nuke() {
        for (auto it = allocatedNodes.begin(); it != allocatedNodes.end(); ++it) {
            delete (*it).second;
        }

        allocatedNodes.clear();
    }

};


#endif //IOA_SEMESTRALKA_NodeAllocator_H
