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

    string addNode(string name, string x, string y) {
        try {
            fStar::Node* node = loader->MakeNode(name, std::stoi(x), std::stoi(y) );
            star->addNode(node);
        }catch (const std::exception& e) {
            return "Oparation failed\n" + std::string(e.what());
        }

        return "Node " + name + " added successfully";
    };
    void deleteNode();
    void modifyNode();

    void addEdge();
    void deleteEdge();
    void modifyEdge();

    fStar::FStar getFStar();
};

#endif //IOA_SEMESTRALKA_CONTROLER_H
