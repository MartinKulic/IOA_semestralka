//
// Created by ja on 31/05/2026.
//

#ifndef IOA_SEMESTRALKA_CONTROLER_H
#define IOA_SEMESTRALKA_CONTROLER_H
#include "../fStar/fStar.hpp"
#include  "../fStar/Loader.hpp"
#include "../fStar/Alg.hpp"

class Controler {
    private:
    fStar::FStar* star;
    Loader* loader;
    DistanceMatrix* distancaMatrix;

    public:
    Controler(fStar::FStar* fstar, Loader* loader): star(fstar), loader(loader) {
        this->distancaMatrix = new DistanceMatrix(fstar);
    };
    ~Controler() {
        delete distancaMatrix;
    }

    string addNode(string name, string sx, string sy, fStar::Node** newNodeToRet) {
        float x,y;
        try {
            x = std::stof(sx);
        }catch (...) {
            return "Error while parsing x " + sx;
        }
        try {
            y = std::stof(sy);
        }catch (...) {
            return "Error while parsing y " + sy;
        }


        try {
            fStar::Node* node = loader->MakeNode(name, x, y );
            star->addNode(node);
            *newNodeToRet = node;
        }catch (const std::exception& e) {
            return "Oparation failed\n" + std::string(e.what());
        }

        return "Node " + name + " added successfully to x " + sx + " y " + sy;
    };
    string deleteNode(int nodeToDelId) {
        this->star->deleteNode(nodeToDelId);
        loader->DestroyNode(nodeToDelId);
        return "Node deleted";
    };
    string modifyNode(fStar::Node* nodeToMod, string newName, string snewX, string snewY) {
        if (nodeToMod == nullptr) {           // <-- guard against spurious calls
            return "Why and more likely HOW TF is modifie node called";
        }

        float newX,newY;
        try {
             newX = std::stof(snewX);
        }catch (...) {
            return "Modifie Node - Error while parsing x" + snewX;
        }
        try {
            newY = std::stof(snewY);
        }catch (...) {
            return "Modifie Node - Error while parsing y" + snewY;
        }

        nodeToMod->name=newName;
        nodeToMod->x=newX;
        nodeToMod->y=newY;

        return "Sucsessfull updated node " + nodeToMod->name;
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
            return "Error while parsing weight " + newWeoght;
        }

        star->modifieEdge(edge.from->id, edge.to->id, newWeight);

        return "New Edge w " + newWeoght + " modified.";
    };

    fStar::FStar* getFStar() {
        return this->star;
    };
    DistanceMatrix* D() {
        return this->distancaMatrix;
    }

};

#endif //IOA_SEMESTRALKA_CONTROLER_H
