//
// Created by martin on 13. 4. 2026.
//

#include <iostream>
#include <cmath>

#include "fStar/Alg/DistanceMatrix.hpp"
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

    // for (int i = 0; i < 9; i++) {
    //     fStar::Node* n = l.MakeNode(to_string(i+1), float(coordList[i*2]), float(coordList[(i*2)+1]));//fStar::Node({float(coordList[i*2]), float(coordList[(i*2)+1]), i, to_string(i+1)});
    // }
    // for (int i = 0; i < 30; i++) {
    //     fStar::Node* n = l.MakeNode(to_string(i), 10*i, 20, false);//fStar::Node({float(coordList[i*2]), float(coordList[(i*2)+1]), i, to_string(i+1)});
    //
    // }

    FStar fsStar = FStar();
    // for (int i = 0; i < 30; i++) {
    //     fsStar.addNode(l[i]);
    // }


    Controler c = Controler(&fsStar, &l);

    // c.load("../save2");
    // c.clearResult();

    //c.runAlgorithm("1", "10", "1");
    //c.runAlgorithm("2", "10", "1");

    //fStar::Edge edgeToDel = {l[5], l[6], 10};
    //c.deleteEdge(edgeToDel);

    //c.load("../save_not_full_graph");
    // fStar::Node* ntm = l[7];
    // c.modifyNode(ntm, ntm->name, std::to_string(ntm->x), std::to_string(ntm->y), true);

    //c.runAlgorithm("2", "1000", "1");
    //std::cout << "alg fin";

    gui g = gui(&fsStar, &c);
    g.run();

    // char ch;
    // cin >> ch;
    return 0;
}