//
// Created by ja on 31/05/2026.
//

#ifndef IOA_SEMESTRALKA_CONTROLER_H
#define IOA_SEMESTRALKA_CONTROLER_H
#include "../fStar/fStar.hpp"
#include  "../fStar/Loader.hpp"
class Controler {
    private:
    fStar::FStar* star;
    Loader* loader;

    public:
    Controler(fStar::FStar* fstar, Loader* loader): star(fstar), loader(loader) {};

    string addNode(string name, string x, string y, fStar::Node* newNodeToRet) {
        try {
            fStar::Node* node = loader->MakeNode(name, std::stoi(x), std::stoi(y) );
            star->addNode(node);
            newNodeToRet = node;
        }catch (const std::exception& e) {
            return "Oparation failed\n" + std::string(e.what());
        }

        return "Node " + name + " added successfully to x " + x + " y " + y;
    };
    string deleteNode(int nodeToDelId) {
        this->star->deleteNode(nodeToDelId);
        loader->DestroyNode(nodeToDelId);
        return "Node deleted";
    };
    string modifyNode(fStar::Node* nodeToMod, string newName, string snewX, string snewY) {
        float newX,newY;
        try {
             newX = std::stof(snewX);
        }catch (...) {
            return "Error while parsing " + snewX;
        }
        try {
            newY = std::stof(snewY);
        }catch (...) {
            return "Error while parsing " + snewY;
        }

        nodeToMod->name=newName;
        nodeToMod->x=newX;
        nodeToMod->y=newY;
    };

    void addEdge();
    string deleteEdge(fStar::Edge edgeToDel) {
        star->deleteEdge(edgeToDel.from->id, edgeToDel.to->id);
        return"Edge to " + edgeToDel.to->name + " deleted.";
    };
    string modifyEdge(fStar::Edge edge, string newWeoght) {
        float newWeight;
        try {
            newWeight = std::stof(newWeoght);
        }catch (...) {
            return "Error while parsing " + newWeoght;
        }

        star->modifieEdge(edge.from->id, edge.to->id, newWeight);

        return "New Edge w " + newWeoght + " modified.";
    };

    fStar::FStar getFStar();
};

#endif //IOA_SEMESTRALKA_CONTROLER_H
