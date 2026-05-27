#include <ftxui/component/component.hpp>          // Pre Renderer, ResizableSplitLeft, CatchEvent
#include <ftxui/component/screen_interactive.hpp> // Pre ScreenInteractive
#include <ftxui/component/event.hpp>              // Pre Event, Mouse
#include <memory>

#include <string>
#include <format>

#include "Transformer.h"
#include "../fStar/fStar.hpp"


using namespace ftxui;
using namespace fStar;

class gui {
private:
    FStar* fstar;
    Canvas c;
    Transformer* transformer;

    float _canvas_zoom = 1.0f;
    float _canvas_pan_x = 10.0f;
    float _canvas_pan_y = -100.0f;
    bool is_dragging = false;

    int graph_panel_size = 70;

    // selected Node
    fStar::Node* selected_node = nullptr;
    int last_mouse_x = 0;
    int last_mouse_y = 0;
    int canvas_mouse_x = 0;
    int canvas_mouse_y = 0;

    void DrawNodes() {
        for (FStarIterator::NodeIterator it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node* node = *it;
            coor cor = transformer->transform(node);

            // Text vykres¾ujeme na znakové súradnice plátna. 
            // V FTXUI Canvas::DrawText berie sub-pixelové súradnice, preto násobíme:
            int sub_x = cor.x * 2;
            int sub_y = cor.y * 2;

            if (selected_node == node) {
                c.DrawText(sub_x, sub_y, "> " + node->name + " <");
            }
            else {
                c.DrawText(sub_x, sub_y, node->name);
            }
        }
    }

    void DrawEdges() {
        for (FStarIterator::EdgeIterator it = fstar->begin_edges(); it != fstar->end_edges(); ++it) {
            fStar::Edge edge = *it;
            coor c1 = transformer->transform(edge.from);
            coor c2 = transformer->transform(edge.to);
            // Èiary kreslíme v sub-pixeloch plátna
            c.DrawPointLine(c1.x * 2, c1.y * 2, c2.x * 2, c2.y * 2);

            if (c1.x >= c2.x) {
                coor t = coor();
                t.x = (c1.x + c2.x)/2;
                t.y = (c1.y + c2.y)/2;
                c.DrawText(t.x * 2, t.y * 2, to_string( edge.weight ));
            }
        }
    }

    void FindClickedNode(int mouse_x, int mouse_y) {
        selected_node = nullptr;
        if (mouse_x > this->graph_panel_size) {
            return;
        }

        int target_sub_x = mouse_x * 2;
        int target_sub_y = mouse_y * 2;

        const int tolerance = 4;

        for (FStarIterator::NodeIterator it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node* node = *it;
            coor cor = transformer->transform(node);

            int node_sub_x = cor.x * 2;
            int node_sub_y = cor.y;

            // Výpoèet vzdialenosti v sub-pixelovom priestore plátna
            if (std::abs(node_sub_x - target_sub_x) <= tolerance &&
                std::abs(node_sub_y - target_sub_y) <= tolerance) {
                selected_node = node;
                break;
            }
        }
    }

public:

    gui(FStar* fstar) : fstar(fstar) {
        this->transformer = new Transformer();
        *transformer += Transformer::Scale(&_canvas_zoom);
        *transformer += Transformer::Move(&_canvas_pan_x, &_canvas_pan_y);
        *transformer += Transformer::FlipY(0.0);
    }
    ~gui() {
        delete transformer;
    }

    void run() {
        auto screen = ScreenInteractive::Fullscreen();


        auto graph_component = Renderer([&] {
            // automatically scale gaph
            int dynamic_w = graph_panel_size*2;
            int dynamic_h = std::max(200, static_cast<int>(300 * _canvas_zoom));

            c = Canvas(dynamic_w, dynamic_h);
            DrawEdges();
            DrawNodes();

            std::string status_text = "Zoom: " + std::to_string(_canvas_zoom).substr(0, 4) +
                " | PanX: " + std::to_string((int)_canvas_pan_x) +
                " | PanY: " + std::to_string((int)_canvas_pan_y) +
                " | MouseX: " + std::to_string(canvas_mouse_x) +
                " | MouseY: " + std::to_string(canvas_mouse_y) +
                    "sidePanel" + std::to_string(graph_panel_size);

            return vbox({
                canvas(&c) | flex,
                separator(),
                text(status_text) | color(Color::Yellow)
                }) | border;
            });

        graph_component = CatchEvent(graph_component, [&](Event event) {
            if (event.is_mouse()) {
                auto mouse = event.mouse();

                // Uchovanie starých súradníc pred aktualizáciou pre presný výpoèet posunu (delta)
                int prev_canvas_x = canvas_mouse_x;
                int prev_canvas_y = canvas_mouse_y;

                canvas_mouse_x = mouse.x - 1; // invert because of diferent origin
                canvas_mouse_y = mouse.y - 1; //

                // SCROLL - ZOOM:
                if (mouse.button == Mouse::WheelUp) {
                    _canvas_zoom *= 1.1f;
                    if (_canvas_zoom > 10.0f) _canvas_zoom = 10.0f; // Bezpeènostný strop
                    return true;
                }
                if (mouse.button == Mouse::WheelDown) {
                    _canvas_zoom /= 1.1f;
                    if (_canvas_zoom < 0.2f) _canvas_zoom = 0.2f;   // Bezpeènostné dno
                    return true;
                }

                // L CLICK - SELECT
                if (mouse.button == Mouse::Left && mouse.motion == Mouse::Pressed) {
                    FindClickedNode(canvas_mouse_x, canvas_mouse_y);
                    return true;
                }

                // R CLICK
                if (mouse.button == Mouse::Right) {
                    if (mouse.motion == Mouse::Pressed) {
                        is_dragging = true;
                        return true;
                    }
                    if (mouse.motion == Mouse::Released) {
                        is_dragging = false;
                        return true;
                    }
                }

                // 4. POHYB (ahanie): Aktualizácia offsetov, ak dríme pravé tlaèidlo
                if (mouse.motion == Mouse::Moved && is_dragging) {
                    int delta_x = canvas_mouse_x - prev_canvas_x;
                    int delta_y = canvas_mouse_y - prev_canvas_y;

                    // Priamo meníme premenné, na ktoré ukazujú lambdy v Transformerovi
                    _canvas_pan_x += delta_x;
                    _canvas_pan_y -= delta_y*2;
                    return true;
                }

                return true;
            }
            return false;
            });

        // Right menu
        auto menu_component = Renderer([&] {
            Elements menu_elements;
            if (selected_node != nullptr) {
                coor cor = transformer->transform(selected_node);
                menu_elements.push_back(text(" EDITÁCIA VRCHOLA ") | bold | color(Color::Green));
                menu_elements.push_back(separator());
                menu_elements.push_back(text("Názov: " + selected_node->name));
                menu_elements.push_back(text("Vykreslené X: " + std::to_string(cor.x)));
                menu_elements.push_back(text("Vykreslené Y: " + std::to_string(cor.y)));
            }
            else {
                menu_elements.push_back(text(" MENU ") | bold);
                menu_elements.push_back(separator());
                menu_elements.push_back(text("Klikni na vrchol pre výber."));
            }
            //menu_elements.push_back(vfill());
            menu_elements.push_back(separator());
            menu_elements.push_back(text("[Stlaè 'q' pre ukonèenie]") | dim);
            return vbox(std::move(menu_elements)) | border;
            });

        auto main_container = ResizableSplitLeft(graph_component, menu_component, &graph_panel_size);
        auto global_component = CatchEvent(main_container, [&](Event event) {
            if (event == Event::Character('q')) {
                screen.ExitLoopClosure()();
                return true;
            }
            return false;
            });

        screen.Loop(global_component);
    }
};