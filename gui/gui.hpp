#include <ftxui/component/component.hpp>          // Pre Renderer, ResizableSplitLeft, CatchEvent
#include <ftxui/component/screen_interactive.hpp> // Pre ScreenInteractive
#include <ftxui/component/event.hpp>              // Pre Event, Mouse
#include <memory>

#include <string>
#include <limits.h>
#include <format>
#include <math.h>

#include "controler.h"
#include "Transformer.hpp"
#include "../fStar/Alg.hpp"
#include "../fStar/fStar.hpp"
#include "ftxui/dom/table.hpp"


using namespace ftxui;
using namespace fStar;

class gui {
private:
    FStar* fstar;
    Controler* controler;

    Canvas c;
    Transformer* transformer;
    DistanceMatrix* D;

    float _canvas_zoom = 1.0f;
    float _canvas_pan_x = 10.0f;
    float _canvas_pan_y = -100.0f;
    bool is_dragging = false;

    int graph_panel_size = 70;

    // Tab state: 0 = Graph, 1 = Distance Matrix
    int active_tab = 0;

    fStar::Node* selected_node = nullptr;
    // for pan
    int last_mouse_x = 0;
    int last_mouse_y = 0;
    int canvas_mouse_x = 0;
    int canvas_mouse_y = 0;

    //for edit node
    std::string edit_name = "";
    std::string edit_x = "";
    std::string edit_y = "";

    Component add_node;
    std::string add_node_name = "";
    std::string add_node_x = "";
    std::string add_node_y = "";

    //Edge edit and delete
    std::deque<std::string*>  edge_weight_buffers; // one entry per out-edge
    //std::vector<Component>    edge_row_inputs;     // Input for each weight
    //std::vector<Component>    edge_row_del_btns;   // "Del" button per edge
    Component                 edge_section;        // Container::Vertical of rows

    // Status feedback
    std::string status_msg = "Status msg";

    void DrawNodes() {
        for (FStarIterator::NodeIterator it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node* node = *it;
            coor cor = transformer->transform(node);

            int sub_x = cor.x * 2; // Canvas works with sub pixels
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

            c.DrawPointLine(c1.x * 2, c1.y * 2, c2.x * 2, c2.y * 2); // in sub-pixels

            //if (c1.x >= c2.x) {
                coor t = coor();
                t.x = (c1.x + c2.x);
                t.y = (c1.y + c2.y);
                c.DrawText(t.x, t.y, std::to_string( edge.weight )); // in middle of points - (c1.x + c2.x)/2
            //}
        }
    }

    void SetSelectedNode(fStar::Node* node) {
        if (node == nullptr) {
            this->selected_node = nullptr;
            this->edit_name = "";
            this->edit_x = "";
            this->edit_y = "";
            return;
        }
        this->selected_node = node;
        this->edit_name = node->name;
        this->edit_x = std::to_string(node->x);
        this->edit_y = std::to_string(node->y);

        RebuildEdgeSection();
    }

    void FindClickedNode(int mouse_x, int mouse_y) {

        if (mouse_x > this->graph_panel_size) {
            return;
        }
        SetSelectedNode(nullptr);
        int target_sub_x = mouse_x * 2;
        int target_sub_y = mouse_y * 2;

        const int tolerance = 4;


        for (FStarIterator::NodeIterator it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node* node = *it;
            coor cor = transformer->transform(node);

            int node_sub_x = cor.x * 2;
            int node_sub_y = cor.y;

            // Distance in subpixel of canvas
            if (std::abs(node_sub_x - target_sub_x) <= tolerance &&
                std::abs(node_sub_y - target_sub_y) <= tolerance) {
                this->SetSelectedNode(node);
                break;
            }
        }
    }

    void RebuildEdgeSection() {
        for (string* s : edge_weight_buffers) {
            delete s;
        }
        edge_weight_buffers.clear();


        if (!selected_node) {
            edge_section = Container::Vertical({});
            return;
        }

        Components rows;
        auto end = fstar->end_out_edges(selected_node->id);
        for (auto it = fstar->begin_out_edges(selected_node->id); it != end; ++it) {
            fStar::Edge edge = *it;
            string* input_val = new string( std::to_string( edge.weight ));
            this->edge_weight_buffers.push_back(input_val);

            auto input = Input(input_val, "w");

            int to_id = edge.to->id;
            // Capture to_id by value; capture this by reference for fstar access
            auto del_btn = Button("Delete", [this, to_id, edge] {
                fstar->deleteEdge(selected_node->id, to_id);
                status_msg = "Edge to " + edge.to->name + " deleted.";
                RebuildEdgeSection();
            });

            auto apply_button = Button( "Apply", [this, to_id, edge, input_val] {
                // fstar->editWeight(selected_node->id, to_id, std::stoi(input_val) )
                status_msg = "Weight from " + edge.to->name + " updeted";
                RebuildEdgeSection();
            });


            auto row = ftxui::Renderer(
            Container::Horizontal({ input, apply_button, del_btn }),
            [=] {   // capture w_input, del_btn, from, to by value
                return hbox({
                    text(edge.from->name + " -[") | vcenter,
                    input->Render() | size(WIDTH, EQUAL, 9) | vcenter,
                    text("]-> " + edge.to->name + " ") | vcenter,
                    filler(),
                    apply_button->Render()  | vcenter,
                    del_btn->Render() | color(Color::Red) | vcenter,
                    });
                }
            );
            rows.push_back(row);
        }

        edge_section = Container::Vertical(std::move(rows));
    }

    Component AddNodeComponent() {
        auto name_in = Input(&this->add_node_name, "Name");
        auto x_in = Input(&this->add_node_x, "X");
        auto y_in = Input(&this->add_node_y, "Y");
        auto add_button = Button("Add", [this, name_in, x_in, y_in] {
            controler->addNode(this->add_node_name, add_node_x, add_node_y);
        });

        auto container = Container::Vertical({
            name_in,
            x_in,
            y_in,
            add_button
        });

        return Renderer(container, [&, container] {
            Elements elements;

            elements.push_back(hbox(text("Name: "), name_in->Render()));
            elements.push_back(hbox(text("X: "), x_in->Render()));
            elements.push_back(hbox(text("Y: "), y_in->Render()));
            elements.push_back(add_button->Render());


            return vbox(elements);
        });
    }

    Component GraphComponent() {
        auto graphContainer = Renderer([&] {
            // automatically scale gaph
            int dynamic_w = std::max(100, graph_panel_size*2);
            int dynamic_h = std::max(270, static_cast<int>(300 * _canvas_zoom));

            c = Canvas(dynamic_w, dynamic_h);
            DrawEdges();
            DrawNodes();

            std::string status_text = "Zoom: " + std::to_string(_canvas_zoom).substr(0, 4) +
                " | PanX: " + std::to_string((int)_canvas_pan_x) +
                " | PanY: " + std::to_string((int)_canvas_pan_y) +
                " | MouseX: " + std::to_string(canvas_mouse_x) +
                " | MouseY: " + std::to_string(canvas_mouse_y);

            return vbox({
                canvas(&c) | flex,
                separator(),
                text(status_text) | color(Color::Yellow)
                }) | border;
            });

        // Events
        auto graphComponent = CatchEvent(graphContainer, [&](Event event) {
            if (!event.is_mouse()) {
                return false;
            }
            //else
                auto mouse = event.mouse();

                if (mouse.x >= graph_panel_size) {
                    return false;
                }

                // Keep old coord - for movemet calculation
                int prev_canvas_x = canvas_mouse_x;
                int prev_canvas_y = canvas_mouse_y;

                canvas_mouse_x = mouse.x - 1; // invert because of different origin
                canvas_mouse_y = mouse.y - 1; //

                // SCROLL - ZOOM:
                if (mouse.button == Mouse::WheelUp) {
                    _canvas_zoom *= 1.1f;
                    if (_canvas_zoom > 10.0f) _canvas_zoom = 10.0f; // Safe sealing
                    return true;
                }
                if (mouse.button == Mouse::WheelDown) {
                    _canvas_zoom /= 1.1f;
                    if (_canvas_zoom < 0.2f) _canvas_zoom = 0.2f;   // Safe bottom
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

                // MOVEMENT
                if (mouse.motion == Mouse::Moved && is_dragging) {
                    int delta_x = canvas_mouse_x - prev_canvas_x;
                    int delta_y = canvas_mouse_y - prev_canvas_y;

                    _canvas_pan_x += delta_x;
                    _canvas_pan_y -= delta_y*2;
                    return true;
                }

                return true;
            });
        return graphComponent;
    }

    Component MenuComponent() {
        auto name_input  = Input(&edit_name,  "node name");
        auto x_input     = Input(&edit_x,     "X coord");
        auto y_input     = Input(&edit_y,     "Y coord");

        auto apply_button = Button("  Apply Changes  ", [&] { /*ApplyNodeEdits();*/; });
        auto delete_node_button = Button("  Delete node  ", [&]() {
            std::string name = selected_node->name;
            fstar->deleteNode(selected_node->id);
            SetSelectedNode(nullptr);
            this->status_msg="Deleted node " + name;
        });

        auto add_node_section = AddNodeComponent();

        // Proxy component: always delegates render & events to the current
        // edge_section, which is rebuilt by RebuildEdgeSection() on node change.
        // This lets the parent container stay fixed while edge components vary.
        // auto edge_proxy = CatchEvent(
        //     Renderer([&] { return edge_section->Render(); }),
        //     [&](Event e)  { return edge_section->OnEvent(e); }
        // );

        auto container = Container::Vertical({
            name_input,
            x_input,
            y_input,
            apply_button,
            edge_section,
            add_node_section,
            delete_node_button,
        });

        return  Renderer(container, [&, name_input, x_input, y_input, apply_button, delete_node_button, add_node_section] {
            Elements menu_elements;
            if (this->selected_node != nullptr) {
                coor cor = transformer->transform(selected_node);
                menu_elements.push_back(text(" Edit Node ") | bold | color(Color::Green));

                menu_elements.push_back(separator());

                menu_elements.push_back(hbox({ text("Name : "), name_input->Render() }));
                menu_elements.push_back(hbox({ text("Pos X : "), x_input->Render() }));
                menu_elements.push_back(hbox({ text("Pos Y : "), y_input->Render() }));

                menu_elements.push_back(text("Drawed X: " + std::to_string(cor.x)));
                menu_elements.push_back(text("Drawed Y: " + std::to_string(cor.y)));

                menu_elements.push_back(separator());
                // Apply button
                menu_elements.push_back(
                    apply_button -> Render()
                );

                menu_elements.push_back(separator());

                menu_elements.push_back(text(" Out Edges ") | bold);
                menu_elements.push_back(edge_section->Render());

                menu_elements.push_back(
                  delete_node_button -> Render() | color(Color::Red)
                );

                menu_elements.push_back(separator());
                menu_elements.push_back(AddNodeComponent()->Render());
            }
            else {
                menu_elements.push_back(text(" MENU ") | bold);
                menu_elements.push_back(separator());
                menu_elements.push_back(text("Clickt on node to select."));
            }
            //menu_elements.push_back(vfill());

            if (!status_msg.empty()) {
                    menu_elements.push_back(separator());
                    menu_elements.push_back(text(status_msg) | color(Color::Yellow));
                }

            menu_elements.push_back(separator());
            menu_elements.push_back(text("[q to exit]") | dim);

            return vbox(std::move(menu_elements)) | border;
            });

    }

    Component MatrixComponent() {
        return Renderer([&] {
            Elements rows;
            rows.push_back(text(" Distance Matrix ") | bold | color(Color::Cyan));
            rows.push_back(separator());

            if (!D) {
                rows.push_back(text("No matrix available."));
                return vbox(std::move(rows)) | border;
            }

            // Header row: node names
            Elements header_cells;
            header_cells.push_back(text("     ") | size(WIDTH, EQUAL, 8));

            std::vector<std::vector<string>> tableData = std::vector<std::vector<string>>();

            vector<string> firstRow = vector<string>(1);
            fStar::FStarIterator::NodeIterator end = fstar->end_nodes();
            for (fStar::FStarIterator::NodeIterator i = fstar->begin_nodes(); i != end; ++i) {
                firstRow.push_back((*i)->name);
            }
            tableData.push_back(firstRow);


            for (fStar::FStarIterator::NodeIterator it = fstar->begin_nodes(); it != end; ++it) {
                vector<string> row = vector<string>();
                row.push_back((*it)->name);
                for (fStar::FStarIterator::NodeIterator it2 = fstar->begin_nodes(); it2 != end; ++it2) {
                    const float curWeigt = (*D)[(*it)->id][(*it2)->id];
                    row.push_back(std::to_string(curWeigt));
                }
                tableData.push_back(row);
            }

            auto table = ftxui::Table(tableData);
            table.SelectRows(0,-1).SeparatorVertical(LIGHT);
            table.SelectColumns(0,-1).SeparatorHorizontal(LIGHT);

            table.SelectAll().Border(LIGHT);
            table.SelectRow(0).Decorate(bold);
            table.SelectRow(0).SeparatorVertical(LIGHT);
            table.SelectRow(0).Border(HEAVY);
            table.SelectRow(0).DecorateCells(center);

            table.SelectColumn(0).Decorate(bold);
            table.SelectColumn(0).SeparatorHorizontal(LIGHT);
            table.SelectColumn(0).Border(HEAVY);
            table.SelectColumn(0).DecorateCells(center);


            rows.push_back(table.Render());


            return vbox(std::move(rows)) | border | flex;
        });
    }

public:

    gui(FStar* fstar, Controler* controler) : fstar(fstar), controler(controler){
        this->transformer = new Transformer();
        *transformer += Transformer::Scale(&_canvas_zoom);
        *transformer += Transformer::Move(&_canvas_pan_x, &_canvas_pan_y);
        *transformer += Transformer::FlipY(0.0);

        this->D = new DistanceMatrix(fstar);

        SetSelectedNode(fstar->getNode(1));
    }
    ~gui() {
        delete transformer;
        delete D; // !!!!

        for (string* s : edge_weight_buffers) {
            delete s;
        }
    }

    void run() {
        auto screen = ScreenInteractive::Fullscreen();

        auto graph_component = GraphComponent();
        auto menu_component = MenuComponent();
        auto matrix_component = MatrixComponent();

        // Tab bar //leftpanel + right pannel
        std::vector<std::string> tab_labels = { " Graph ", " Distance Matrix " };
        auto tab_toggle = Toggle(&tab_labels, &active_tab);

        auto tab_container = Container::Tab(
            {
                graph_component,
                matrix_component
            },
            &active_tab
        );

        auto left_container = Container::Vertical({
            tab_toggle,
            tab_container
        });

        auto left_renderer = Renderer(left_container, [&] {
            return vbox({
                tab_toggle->Render(),
                tab_container->Render() | flex,
            });
        });

        auto full_view = ResizableSplitLeft(
            left_renderer,
            menu_component,
            &graph_panel_size
        );


        auto main_component = CatchEvent(full_view, [&](Event event) {
            if (event == Event::Character('q')) {
                screen.ExitLoopClosure()();
                return true;
            }
            // 'r' shortcut to recalculate matrix from anywhere
            if (event == Event::Character('r')) {
                //RebuildDistanceMatrix();
                return true;
            }
            return false;
        });

        screen.Loop(main_component);
    }
};