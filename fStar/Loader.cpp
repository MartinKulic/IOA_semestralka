//
// Created by ja on 03/06/2026.
//

#include <iostream>
#include <fstream>

#include "Loader.hpp"

#include "ftxui/dom/node.hpp"


void Loader::save(std::filesystem::path path, fStar::FStar* star) {
    //TODO: Validate path - mkdir -p
    std::filesystem::create_directories(path);

    std::filesystem::path starFilePath = path / Loader::STAR_FILE_NAME;
    ofstream outStarFile(starFilePath.string());

    outStarFile << star->sizeNodes() << "\n";
    auto endNodes = star->end_nodes();
    for (auto nodeIt = star->begin_nodes(); nodeIt != endNodes; ++nodeIt) {
        fStar::Node* node = *nodeIt;

        outStarFile << node->id << " " << node->name << " " << node->x << " " << node->y << "\n";
    }

    outStarFile << "e\n" << star->sizeEdges() << "\n";

    auto endEdges = star->end_edges();
    for (auto edgeIt = star->begin_edges(); edgeIt != endEdges; ++edgeIt) {
        fStar::Edge edge = *edgeIt;
        outStarFile << edge.from->id << " " << edge.to->id << " " << edge.weight << "\n";
    }
    outStarFile.close();

}

void Loader::load(std::filesystem::path path, fStar::FStar *star, NodeAllocator *nodeAllocator, bool ignoreId) {
    //TODO: Validate path - if path bad = exception;

    star->nuke();
    nodeAllocator->nuke();
    //
    std::filesystem::path starFilePath = path / Loader::STAR_FILE_NAME;
    ifstream inStarFile(starFilePath);

    if (!inStarFile.good()) {
        throw std::invalid_argument("Could not open file " + starFilePath.string() + "\nMake sure " +path.string() + " is correct\n" );
    }

    char nextChar='k';

    inStarFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //read first line = num of nodes
    nextChar = inStarFile.peek();

    while (!inStarFile.eof() && nextChar != 'e') {
        int id;
        string name;
        float x;
        float y;

        inStarFile >> id;
        inStarFile >> name;
        inStarFile >> x;
        inStarFile >> y;

         fStar::Node* node;
        if (ignoreId) {
            node = nodeAllocator->MakeNode(name, x, y);
        }else {
            node = nodeAllocator->MakeNode(name, x, y, id);
        }

        star->addNode(node);
        inStarFile >> nextChar;
        inStarFile.putback(nextChar);
    }

    inStarFile.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); //read line = e
    inStarFile.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); //read line = num of nodes
    while (!inStarFile.eof()) {
        int from, to;
        float weight;

        inStarFile >> from;
        inStarFile >> to;
        inStarFile >> weight;

        fStar::Node* node_from = (*nodeAllocator)[from];
        fStar::Node* node_to = (*nodeAllocator)[to];

        star->addEdge(node_from, node_to, weight,true);

    }

}
