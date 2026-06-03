//
// Created by ja on 03/06/2026.
//

#include <iostream>
#include <fstream>

#include "Loader.hpp"

#include "ftxui/dom/node.hpp"


void Loader::save(std::filesystem::path path, fStar::FStar* star) {
    //TODO: Validate path - mkdir -p

    ofstream outStarFile(path.string());

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

void Loader::load(std::string path, fStar::FStar *star, NodeAllocator *nodeAllocator) {
    //TODO: Validate path - if path bad = exception


}
