//
// Created by martin on 16. 4. 2026.
//

#ifndef IOA_SEMESTRALKA_GUI_HPP
#define IOA_SEMESTRALKA_GUI_HPP

#include <ftxui/dom/elements.hpp>  // for Fit, canvas, operator|, border, Element
#include <ftxui/screen/screen.hpp>  // for Cell, Screen

#include "ftxui/dom/canvas.hpp"  // for Canvas
#include "ftxui/dom/node.hpp"    // for Render
#include "ftxui/screen/color.hpp"  // for Color, Color::Red, Color::Blue, Color::Green, ftxui

#include "../fStar/fStar.hpp"
#include <functional>

using namespace ftxui;
using namespace fStar;

class Transformer {
    private:
    std::vector< std::function<coor(coor)> > transorms;
    public:
    Transformer(){};

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
        coor newCoord = coor{node->x, node->y};
        for (auto transform : transorms) {
            newCoord = transform(newCoord);
        };
        return newCoord;
    }


    static auto Scale(float* scale) {
        return [scale](coor cor){ return coor({(cor.x**scale),(cor.y**scale)}) ;};
    };
    static auto Move(float* moveX, float* moveY) {
        return [moveX, moveY](coor cor){ return coor({(cor.x+*moveX),(cor.y+*moveY)}) ;};
    }
};


class gui {

private:
    FStar* fstar;
    Canvas c;// = Canvas(100, 100);
    Transformer* transformer;

    void DrawNodes() {
        for (FStarIterator::NodeIterator it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node* node = *it;

            coor cor = transformer->transform(node);
            int x = cor.x;
            int y = cor.y;
            //c.DrawPointCircle(x, y, 5);
            c.DrawPointEllipse(x+5,y,9,4);
            c.DrawText(x, y, node->name);
        }
    }

    void DrawEdges() {
        for (FStarIterator::EdgeIterator it = fstar->begin_edges(); it != fstar->end_edges(); ++it) {
            fStar::Edge edge = *it;

            coor c1 = transformer->transform(edge.from);
            int x1 = c1.x; //(edge.from->x*scaleFactor) + moveX;
            int y1 = c1.y; //(edge.from->y*scaleFactor) + moveY;
            coor c2 = transformer->transform(edge.to);
            int x2 = c2.x;//(edge.to->x*scaleFactor) + moveX;
            int y2 = c2.y;//(edge.to->y*scaleFactor) + moveY;

            c.DrawPointLine(x1,y1,x2,y2);

            int textX = (x1+x2)/2;
            int textY = (y1+y2)/2;

            if (edge.from->id < edge.to->id) {
                //nad line
                c.DrawText(textX,textY-3, to_string(edge.weight));
            }
            else {
                //pod
                c.DrawText(textX,textY+2, to_string(edge.weight));
            }
        }
    }
public:
    static inline const float scaleFactor = 4.5;
    static inline const float moveX = 10;
    static inline const float moveY = 10;


    gui(FStar* fstar, Transformer* transformer) : transformer(transformer){
        this->fstar = fstar;
    };
    void run() {
        c = Canvas(100, 100);

        DrawEdges();
        DrawNodes();

        auto document = canvas(&c) | border;
        auto screen = Screen::Create(Dimension::Fit(document));
        Render(screen, document);
        screen.Print();
    };


};



#endif //IOA_SEMESTRALKA_GUI_HPP
