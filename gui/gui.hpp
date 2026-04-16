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

using namespace ftxui;
using namespace fStar;
class gui {
private:
    FStar* fstar;
    Canvas c;// = Canvas(100, 100);

    float scaleFactor = 4.5;
    float moveX = 10;
    float moveY = 10;

    void DrawNodes() {
        for (FStarIterator::NodeIterator it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node* node = *it;
            int x = (node->x*scaleFactor)+moveX;
            int y = (node->y*scaleFactor)+moveY;
            //c.DrawPointCircle(x, y, 5);
            c.DrawPointEllipse(x+5,y,9,4);
            c.DrawText(x, y, node->name);
        }
    }

    void DrawEdges() {
        for (FStarIterator::EdgeIterator it = fstar->begin_edges(); it != fstar->end_edges(); ++it) {
            fStar::Edge edge = *it;

            int x1 = (edge.from->x*scaleFactor) + moveX;
            int y1 = (edge.from->y*scaleFactor) + moveY;
            int x2 = (edge.to->x*scaleFactor) + moveX;
            int y2 = (edge.to->y*scaleFactor) + moveY;

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
    gui(FStar* fstar) {
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
