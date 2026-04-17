//
// Created by martin on 13. 4. 2026.
//

#include <iostream>

#include "fStar/fStar.hpp"
#include "gui/gui.hpp"

using namespace std;
using namespace fStar;
int main() {
    fStar::Node n0 = fStar::Node({0,0, 0, "node 0"});
    fStar::Node n1 = fStar::Node({10,0, 1, "node 1"});
    fStar::Node n2 = fStar::Node({0,10, 2, "node 2"});

    FStar fsStar = FStar();
    fsStar.addEdge(&n0, &n1, 10);
    fsStar.addEdge(&n1, &n0, 10);
    fsStar.addEdge(&n0, &n2, 20);

    fsStar.addEdge(&n1, &n2, 10);
    fsStar.addEdge(&n2, &n1, 30);

    cout << "good?" << endl;

    FStarIterator::NodeIterator start = fsStar.begin_nodes();
    FStarIterator::NodeIterator end = fsStar.end_nodes();

    for (FStarIterator::NodeIterator it = start; it != end; ++it) {
        cout << (*it)->name << "\n";
    }
    cout << endl;

    for (FStarIterator::EdgeIterator it = fsStar.begin_edges(); it != fsStar.end_edges(); ++it) {
        cout << (*it).from->name << " --[" << (*it).weight << "]-> " << (*it).to->name << "\n";
    }

    Transformer t;
    float scale = gui::scaleFactor;
    auto scaleOperation = Transformer::Scale(&scale);
    float mx = gui::moveX;
    float my = gui::moveY;
    auto moveOperation = Transformer::Move(&mx, &my);
    t+=scaleOperation;
    t+=moveOperation;

    gui g = gui(&fsStar, &t);
    g.run();

    char ch;
    cin >> ch;
    return 0;
}