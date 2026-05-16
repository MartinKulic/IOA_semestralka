#include <ftxui/component/component.hpp>          // Pre Renderer, ResizableSplitLeft, CatchEvent
#include <ftxui/component/screen_interactive.hpp> // Pre ScreenInteractive
#include <ftxui/component/event.hpp>              // Pre Event, Mouse
#include <memory>

#include <string>
#include <format>

#include "../fStar/fStar.hpp"


using namespace ftxui;
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
};

class gui {
private:
    FStar* fstar;
    Canvas c;
    Transformer* transformer;
    float zoom_factor = 1.0f;
    float offset_x = 10.0f;
    float offset_y = 10.0f;
    bool is_dragging = false;

    // Premenná pre uloenie aktuálne vybraného vrcholu (nullptr ak iadny)
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

    // Pomocná funkcia na zistenie, èi pouívate¾ klikol blízko nejakého vrcholu
    void HandleMouseClick(int mouse_x, int mouse_y) {
        // m_x a m_y sú relatívne znakové súradnice vo vnútri borderu grafu
        // Prepoèítame ich na strednú hodnotu sub-pixelov Braille matice
        int target_sub_x = mouse_x * 2;
        int target_sub_y = mouse_y * 2;

        const int tolerance = 4; // Tolerancia zásahu v sub-pixeloch
        selected_node = nullptr;

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
    static inline const float scaleFactor = 1;
    static inline const float moveX = 1;
    static inline const float moveY = 10;

    gui(FStar* fstar, Transformer* transformer) : fstar(fstar), transformer(transformer) {
        *transformer += Transformer::Scale(&zoom_factor);
        *transformer += Transformer::Move(&offset_x, &offset_y);
    }

    void run() {
        auto screen = ScreenInteractive::Fullscreen();
        int graph_panel_size = 70;

        auto graph_component = Renderer([&] {
            // Ve¾kos plátna dynamicky kálujeme pod¾a zoomu, aby objekty neboli orezané mimo pevných 100x100
            int dynamic_w = std::max(200, static_cast<int>(300 * zoom_factor));
            int dynamic_h = std::max(200, static_cast<int>(300 * zoom_factor));

            c = Canvas(dynamic_w, dynamic_h);
            DrawEdges();
            DrawNodes();

            std::string status_text = "Zoom: " + std::to_string(zoom_factor).substr(0, 4) +
                " | PanX: " + std::to_string((int)offset_x) +
                " | PanY: " + std::to_string((int)offset_y) +
                " | [Koliesko = Zoom | Pravé tl. myi = Posun]";

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

                canvas_mouse_x = mouse.x - 1; // Mínus 1 kvôli ¾avému okraju borderu
                canvas_mouse_y = mouse.y - 1; // Mínus 1 kvôli hornému okraju borderu

                // 1. ZOOM: Spracovanie kolieska myi
                if (mouse.button == Mouse::WheelUp) {
                    zoom_factor *= 1.1f;
                    if (zoom_factor > 10.0f) zoom_factor = 10.0f; // Bezpeènostný strop
                    return true;
                }
                if (mouse.button == Mouse::WheelDown) {
                    zoom_factor /= 1.1f;
                    if (zoom_factor < 0.2f) zoom_factor = 0.2f;   // Bezpeènostné dno
                    return true;
                }

                // 2. KLIKNUTIE: Výber uzla ¾avým tlaèidlom
                if (mouse.button == Mouse::Left && mouse.motion == Mouse::Pressed) {
                    HandleMouseClick(canvas_mouse_x, canvas_mouse_y);
                    return true;
                }

                // 3. PAN: Zaèiatok a koniec ahania pravým tlaèidlom myi
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
                    offset_x += delta_x;
                    offset_y += delta_y;
                    return true;
                }

                return true;
            }
            return false;
            });

        // Komponent pre pravé menu bez zmien
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