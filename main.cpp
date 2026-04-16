//
// Created by martin on 13. 4. 2026.
//

#include <iostream>

#include "fStar/fStar.hpp"

using namespace std;

int main() {
    Node n0 = Node({0,0, 0, "node 0"});
    Node n1 = Node({10,0, 1, "node 1"});
    Node n2 = Node({0,10, 2, "node 2"});

    FStar fsStar = FStar();
    fsStar.addEdge(&n0, &n1, 10);
    fsStar.addEdge(&n1, &n0, 10);
    fsStar.addEdge(&n0, &n2, 10);

    fsStar.addEdge(&n2, &n1, 10);
    fsStar.addEdge(&n2, &n1, 20);

    cout << "good?";
    return 0;
}