//
// Created by martin on 13. 4. 2026.
//

#include <iostream>
#include <cmath>

#include "fStar/Alg.hpp"
#include "fStar/fStar.hpp"
#include "gui/gui.hpp"

using namespace std;
using namespace fStar;
int main() {
    int coordList[] = {10,10, 40,20, 70,20, 90,10, 90,50, 80,80, 50,60, 30,40, 20,80};
    int edgesList[] = {1,2, 1,8,
        2,1, 2,3, 2,8,
        3,2, 3,4, 3,5,
        4,3,
        5,3, 5,7, 5,8,
        6,7,
        7,5, 7,6, 7,8, 7,9,
        8,1, 8,2, 8,5, 8,7, 8,9,
        9,7, 9,8
    };

    vector<fStar::Node> nodes = vector<fStar::Node>();
    for (int i = 0; i < 9; i++) {
        fStar::Node n = fStar::Node({float(coordList[i*2]), float(coordList[(i*2)+1]), i, to_string(i+1)});
        nodes.push_back(n);
    }

    FStar fsStar = FStar();

    for (int i = 0; i < 24; i++) {
        int id_node_from = edgesList[i*2]-1;
        int id_node_to = edgesList[(i*2)+1]-1;
        fStar::Node* node_from = &nodes[id_node_from];
        fStar::Node* node_to = &nodes[id_node_to];

        float weight =  sqrt(pow(node_to->x - node_from->x, 2) + pow(node_to->y - node_from->y ,  2));

        fsStar.addEdge(node_from, node_to, weight);
    }


    cout << "good?" << endl;

    FStarIterator::NodeIterator start = fsStar.begin_nodes();
    FStarIterator::NodeIterator end = fsStar.end_nodes();

    cout << "Node iterator" << endl;
    for (FStarIterator::NodeIterator it = start; it != end; ++it) {
        cout << (*it)->name << "\n";
    }
    cout << endl;

    cout << "Edges iterator" << endl;
    for (FStarIterator::EdgeIterator it = fsStar.begin_edges(); it != fsStar.end_edges(); ++it) {
        cout << (*it).from->name << " --[" << (*it).weight << "]-> " << (*it).to->name << "\n";
    }

    cout << "out edges iterator" <<endl;
    for (FStarIterator::OutEdgeIterator it = fsStar.begin_out_edges(7); it != fsStar.end_out_edges(7); ++it) {
        cout << (*it).from->name << " --[" << (*it).weight << "]-> " << (*it).to->name << "\n";
    }

    cout << "Num of nodes: " << fsStar.sizeNodes() << "\nNum of edges: " << fsStar.sizeEdges() << "\n";

    DistanceMatrix dm = DistanceMatrix(&fsStar);

    for (int i = 0; i < fsStar.sizeNodes(); i++) {
        for (int j = 0; j < fsStar.sizeNodes(); j++) {
            cout << dm[i][j] << " ";
        }
        cout << endl;
    }

    Transformer t;
    float scale = gui::scaleFactor;
    auto scaleOperation = Transformer::Scale(&scale);
    float mx = gui::moveX;
    float my = gui::moveY;
    auto moveOperation = Transformer::Move(&mx, &my);
    float height = 100;
    t+=scaleOperation;
    t+=moveOperation;
    t+=Transformer::FlipY(&height);

    gui g = gui(&fsStar, &t);
    g.run();

    char ch;
    cin >> ch;
    return 0;
}