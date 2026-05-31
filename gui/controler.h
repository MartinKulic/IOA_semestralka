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
        return "Node deleted";
    };
    void modifyNode(fStar::Node* nodeToMod, string newName, string snewX, string snewY) {
        float newX = std::stof(snewX);
        float newY = std::stof(snewY);

    };

    void addEdge();
    void deleteEdge();
    void modifyEdge();

    fStar::FStar getFStar();
};

#endif //IOA_SEMESTRALKA_CONTROLER_H
