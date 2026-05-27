//
// Created by ja on 27/05/2026.
//

#ifndef IOA_SEMESTRALKA_TRANSFORMER_H
#define IOA_SEMESTRALKA_TRANSFORMER_H

#include <functional>
#include "../fStar/fStar.hpp"

using namespace fStar;

class Transformer {
private:
    std::vector< std::function<coor(coor)> > transorms;
public:
    Transformer() {};

    Transformer& operator+=(std::function<coor(coor)> newTransform)
    {
        this->transorms.push_back(newTransform);
        return *this;
    }

    // // friends defined inside class body are inline and are hidden from non-ADL lookup
    // friend Transformer operator+(X lhs,        // passing lhs by value helps optimize chained a+b+c
    //                    const X& rhs) // otherwise, both parameters may be const references
    // {
    //     lhs += rhs; // reuse compound assignment
    //     return lhs; // return the result by value (uses move constructor)
    // }

    coor transform(fStar::Node* node) {
        coor newCoord = coor{ node->x, node->y };
        for (auto transform : transorms) {
            newCoord = transform(newCoord);
        };
        return newCoord;
    }


    static auto Scale(float* scale) {
        return [scale](coor cor) { return coor({ (cor.x * *scale),(cor.y * *scale) }); };
    };
    static auto Move(float* moveX, float* moveY) {
        return [moveX, moveY](coor cor) { return coor({ (cor.x + *moveX),(cor.y + *moveY) }); };
    }
    static auto FlipY(float* height) {
        return [height](coor cor) { return coor({ (cor.x),*height - (cor.y) }); };
    }
    static auto FlipY(float height) {
        return [height](coor cor) { return coor({ (cor.x),height - (cor.y) }); };
    }
};


#endif //IOA_SEMESTRALKA_TRANSFORMER_H
