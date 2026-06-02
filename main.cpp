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

    NodeAllocator l = NodeAllocator();

    for (int i = 0; i < 9; i++) {
        fStar::Node* n = l.MakeNode(to_string(i+1), float(coordList[i*2]), float(coordList[(i*2)+1]));//fStar::Node({float(coordList[i*2]), float(coordList[(i*2)+1]), i, to_string(i+1)});
    }

    FStar fsStar = FStar();

    for (int i = 0; i < 24; i++) {
        int id_node_from = edgesList[i*2]-1;
        int id_node_to = edgesList[(i*2)+1]-1;
        fStar::Node* node_from = l[id_node_from];
        fStar::Node* node_to = l[id_node_to];

        float weight =  sqrt(pow(node_to->x - node_from->x, 2) + pow(node_to->y - node_from->y ,  2));

        fsStar.addEdge(node_from, node_to, weight, true);
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

    // cout << "out edges iterator" <<endl;
    // for (FStarIterator::OutEdgeIterator it = fsStar.begin_out_edges(7); it != fsStar.end_out_edges(7); ++it) {
    //     cout << (*it).from->name << " --[" << (*it).weight << "]-> " << (*it).to->name << "\n";
    // }

    cout << "Num of nodes: " << fsStar.sizeNodes() << "\nNum of edges: " << fsStar.sizeEdges() << "\n";

    // fsStar.deleteNode(2);
    //
    // cout << "Node iterator" << endl;
    // for (FStarIterator::NodeIterator it = start; it != end; ++it) {
    //     cout << (*it)->name << "\n";
    // }
    // cout << endl;
    //
    // cout << "Edges iterator" << endl;
    // for (FStarIterator::EdgeIterator it = fsStar.begin_edges(); it != fsStar.end_edges(); ++it) {
    //     cout << (*it).from->name << " --[" << (*it).weight << "]-> " << (*it).to->name << "\n";
    // }
    // cout << "Num of nodes: " << fsStar.sizeNodes() << "\nNum of edges: " << fsStar.sizeEdges() << "\n";
    //
    // fsStar.deleteNode(3);
    // cout << "Node iterator" << endl;
    // for (FStarIterator::NodeIterator it = start; it != end; ++it) {
    //     cout << (*it)->name << "\n";
    // }
    // cout << endl;
    //
    // cout << "Edges iterator" << endl;
    // for (FStarIterator::EdgeIterator it = fsStar.begin_edges(); it != fsStar.end_edges(); ++it) {
    //     cout << (*it).from->name << " --[" << (*it).weight << "]-> " << (*it).to->name << "\n";
    // }
    // cout << "Num of nodes: " << fsStar.sizeNodes() << "\nNum of edges: " << fsStar.sizeEdges() << "\n";

    //float height = 100;
    //t+=Transformer::FlipY(&height);
    //
    //gui g = gui(&fsStar, &t);
    Controler c = Controler(&fsStar, &l);


    gui g = gui(&fsStar, &c);
    g.run();

    char ch;
    cin >> ch;
    return 0;
}