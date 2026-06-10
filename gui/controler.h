//
// Created by ja on 31/05/2026.
//

#ifndef IOA_SEMESTRALKA_CONTROLER_H
#define IOA_SEMESTRALKA_CONTROLER_H
#include "../fStar/fStar.hpp"
#include  "../fStar/NodeAllocator.hpp"
#include "../fStar/Alg.hpp"
#include "../fStar/Loader.hpp"

class Controler {
    private:
    fStar::FStar* star;
    NodeAllocator* loader;
    DistanceMatrix* distancaMatrix;

    public:
    Controler(fStar::FStar* fstar, NodeAllocator* loader): star(fstar), loader(loader) {
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

    string addEdge(fStar::Node* from, fStar::Node* to, string sWeight) {
        if (!to || !from) {
            return "Edge points are not defined";
        }

        float weight;
        try {
            weight = std::stof(sWeight);
        }catch (...) {
            return "Error while parsing weight " + sWeight;
        }

        star->addEdge(from, to, weight);

        return "OK";
    };
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

    float calculateEuclideanDistance(fStar::Node* from, fStar::Node* to) {
        if (!from || !to) {
            return -1.f;
        }
        return sqrtf(pow(to->x - from->x,2)+pow(to->y - from->y, 2));
    }
    string calculateEuclideanDistance(fStar::Node* from, fStar::Node* to, string* dest) {
        if (!from || !to) {
            return "Edge point are not defined";
        }
        *dest = std::to_string(calculateEuclideanDistance(from, to));
        return "Value calculated";
    }

    string recalculateAllDistances() {
        auto endEdgeIt = star->end_edges();
        for (auto edgeIt = star->begin_edges(); edgeIt != endEdgeIt; ++edgeIt) {
            fStar::Edge edge = *edgeIt;

            fStar::Node* from = edge.from;
            fStar::Node* to = edge.to;

            float newWeight = calculateEuclideanDistance(from, to);
            star->modifieEdge(from->id, to->id, newWeight, true);
        }

        return "Recalculate all distances";
    }

    string save(std::string path) {
        //TODO: Implement
        try {
            Loader::save(path, star);
        }catch (exception e) {
            return e.what();
        }

        return "Saved";
    }

    string load(std::string path,  bool ignoreId = false) {
        try {
            Loader::load(path, star, loader, ignoreId);
        }catch (exception e) {
            return e.what();
        }

        delete(this->distancaMatrix);
        this->distancaMatrix = new DistanceMatrix(this->star);

        return "Loaded";
    }

    fStar::FStar* getFStar() {
        return this->star;
    };
    DistanceMatrix* D() {
        return this->distancaMatrix;
    }

};

#endif //IOA_SEMESTRALKA_CONTROLER_H
