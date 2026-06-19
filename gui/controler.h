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
    vector<fStar::Node*> centers;

private:
    void rebuildCenterContainer() {
        for (int i = 0; i < this->centers.size(); i++) {
            (this->centers[i])->belongs_to_p_group = i;
        }
    }

    public:
    Controler(fStar::FStar* fstar, NodeAllocator* loader): star(fstar), loader(loader) {
        this->distancaMatrix = new DistanceMatrix(fstar);

        auto nodeEnd = star->end_nodes();
        for (auto it = star->begin_nodes(); it != nodeEnd; ++it) {
            fStar::Node* node = *it;
            if (node->is_center) {
                this->centers.push_back(node);
            }
        }
        rebuildCenterContainer();
    };
    ~Controler() {
        delete distancaMatrix;
    }

    string addNode(string name, string sx, string sy, bool is_center, fStar::Node** newNodeToRet) {
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
            fStar::Node* node = loader->MakeNode(name, x, y, is_center );
            star->addNode(node);
            *newNodeToRet = node;
        }catch (const std::exception& e) {
            return "Oparation failed\n" + std::string(e.what());
        }

        if (is_center) {
            (*newNodeToRet)->belongs_to_p_group = this->centers.size();
            this->centers.push_back(*newNodeToRet);
        }

        return "Node " + name + " added successfully to x " + sx + " y " + sy;
    };
    string deleteNode(int nodeToDelId) {
        fStar::Node* nodeToDelete = star->getNode(nodeToDelId);
        if (!nodeToDelete) {
            return "Could not found node id " + nodeToDelId;
        }

        if (nodeToDelete->is_center) {
            this->centers.erase(this->centers.begin() + nodeToDelete->belongs_to_p_group);
            rebuildCenterContainer();
        }

        this->star->deleteNode(nodeToDelId);
        loader->DestroyNode(nodeToDelId);
        return "Node deleted";
    };
    string modifyNode(fStar::Node* nodeToMod, string newName, string snewX, string snewY, bool is_center) {
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

        if (nodeToMod->is_center != is_center) {
            nodeToMod->is_center=is_center;

            if (is_center) {
                nodeToMod->belongs_to_p_group = this->centers.size();
                this->centers.push_back(nodeToMod);
            }
            else {
                this->centers.erase(this->centers.begin() + nodeToMod->belongs_to_p_group);
                rebuildCenterContainer();
                nodeToMod->belongs_to_p_group = fStar::Node::NO_GROUP;
            }
        }




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

    string rcalcucateDistanceMatrix() {
        // TODO: Implement
        return "Not implemented yet";
    }

    string runAlgorithm(string p, string temperature, string cooling) {
        // TODO: Implement
        return "Not implemented yet";
    }

    fStar::FStar* getFStar() {
        return this->star;
    };
    DistanceMatrix* D() {
        return this->distancaMatrix;
    }

};

#endif //IOA_SEMESTRALKA_CONTROLER_H
