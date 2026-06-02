//
// Created by ja on 31/05/2026.
//

#ifndef IOA_SEMESTRALKA_LOADER_H
#define IOA_SEMESTRALKA_LOADER_H

// TODO: Save load fStar + whole problem

#include <filesystem>
#include <list>
#import "fStar.hpp"
#include "ftxui/dom/node.hpp"

class Loader {
    private:
    map<int,fStar::Node*> allocatedNodes  = map<int,fStar::Node*>();
    int nextID = 0;

public:
    Loader(){};
    ~Loader() {
        for (auto it = allocatedNodes.begin(); it != allocatedNodes.end(); ++it) {
            delete (*it).second;
        }
    };

    fStar::Node* MakeNode(std::string name, float x, float y) {
        fStar::Node* node = new fStar::Node;
        node->name=name;
        node->x=x;
        node->y=y;
        node->id=nextID++;
        allocatedNodes[node->id] = node;

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

    void save(filesystem::path path);
    void load(std::string path);
};


#endif //IOA_SEMESTRALKA_LOADER_H
