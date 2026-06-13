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
    FStar<NodeType> *fstar;
    Controler *controler;

    Canvas c;
    Transformer *transformer;
    Transformer *rTransformer;


    const coor graph_text_draw_offset{-4, -6};

    float _canvas_zoom = 1.0f;
    float _canvas_pan_x = 10.0f;
    float _canvas_pan_y = -100.0f;
    bool is_dragging = false;

    int graph_panel_size = 70;

    // Tab state: 0 = Graph, 1 = Distance Matrix
    int active_tab = 0;
    int active_node_tab = 0;

    fStar::Node *selected_node = nullptr;
    // for pan
    int last_mouse_x = 0;
    int last_mouse_y = 0;
    int canvas_mouse_x = 0;
    int canvas_mouse_y = 0;
    float last_click_node_x = 0.0f;
    float last_click_node_y = 0.0f;
    bool draw_last_click = true;

    //for edit node
    std::string edit_name = "";
    std::string edit_x = "";
    std::string edit_y = "";

    Component add_node;
    std::string add_node_name = "Name";
    std::string add_node_x = "";
    std::string add_node_y = "";

    //Edge edit and delete
    std::deque<std::string *> edge_weight_buffers; // one entry per out-edge
    bool rebuildEdgeWeightBuffers = false;
    Components edge_current_rows;
    Component edge_section; // Container::Vertical of rows
    std::string add_edge_weight = "10";
    fStar::Node *node_edge_to = nullptr;
    bool is_select_node_to_mode = false;
    // bool is_select_node_to_mode_key_pressed = false;
    // bool is_select_node_to_mode_key_lastState = false;

    std::string path_load_from = "../save";
    std::string path_save_to = "../save";

    // Status feedback
    std::string status_msg = "Status msg";

    int menu_scroll_offset = 0;
    int menu_visible_height = 40; // will be updated each frame via Terminal::Size()

    void DrawNodes() {
        for (FStarIterator::NodeIterator it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node *node = *it;
            coor cor = transformer->transform(node);

            int sub_x = (cor.x * 2) + graph_text_draw_offset.x; // Canvas works with sub pixels
            int sub_y = (cor.y * 2) + graph_text_draw_offset.y;

            if (selected_node == node) {
                c.DrawText(sub_x, sub_y, "< " + node->name + " >", Color::Green);
            } else if (node_edge_to == node) {
                c.DrawText(sub_x, sub_y, "> " + node->name + " <");
            } else {
                c.DrawText(sub_x, sub_y, node->name);
            }
        }
    }

    void DrawEdges() {
        for (FStarIterator::EdgeIterator it = fstar->begin_edges(); it != fstar->end_edges(); ++it) {
            fStar::Edge edge = *it;
            coor c1 = transformer->transform(edge.from) * 2 + graph_text_draw_offset;
            coor c2 = transformer->transform(edge.to) * 2 + graph_text_draw_offset;

            c.DrawPointLine(c1.x, c1.y, c2.x, c2.y); // in sub-pixels

            //if (c1.x >= c2.x) {
            coor t = coor();
            t.x = (c1.x + c2.x) / 2;
            t.y = (c1.y + c2.y) / 2;
            c.DrawText(t.x, t.y, std::to_string(edge.weight)); // in middle of points - (c1.x + c2.x)/2
            //}
        }
    }

    fStar::Node *FindClickedNode(int mouse_x, int mouse_y) {
        if (mouse_x > this->graph_panel_size) {
            return nullptr;
        }
        //SetSelectedNode(nullptr);
        int target_sub_x = mouse_x * 2 - graph_text_draw_offset.x;
        int target_sub_y = mouse_y * 2 - graph_text_draw_offset.y - 5;

        const int tolerance = 4;


        for (FStarIterator::NodeIterator it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node *node = *it;
            coor cor = transformer->transform(node);

            int node_sub_x = cor.x * 2;
            int node_sub_y = cor.y;

            // Distance in subpixel of canvas
            if (std::abs(node_sub_x - target_sub_x) <= tolerance &&
                std::abs(node_sub_y - target_sub_y) <= tolerance) {
                //this->SetSelectedNode(node);
                return node;
                break;
            }
        }
        return nullptr;
    }

    void SetSelectedNode(fStar::Node *node) {
        fStar::Node *prevNode = this->selected_node;

        if (node == nullptr) {
            this->selected_node = nullptr;
            this->edit_name = "";
            this->edit_x = "";
            this->edit_y = "";
            return;
        } else {
            this->edit_name = node->name;
            this->edit_x = std::to_string(node->x);
            this->selected_node = node;
            this->edit_y = std::to_string(node->y);
        }

        this->rebuildEdgeWeightBuffers = prevNode != selected_node;
    }

    void RebuildEdgeRows() {
        for (auto &row: this->edge_current_rows) {
            row->Detach();
        }
        edge_current_rows.clear();

        for (string *s: edge_weight_buffers) {
            delete s;
        }
        edge_weight_buffers.clear();

        if (!selected_node) {
            return;
        }

        auto end = fstar->end_out_edges(selected_node->id);
        for (auto it = fstar->begin_out_edges(selected_node->id); it != end; ++it) {
            fStar::Edge edge = *it;
            string *input_val = new string(std::to_string(edge.weight));
            this->edge_weight_buffers.push_back(input_val);

            auto input = Input(input_val, "w");

            int to_id = edge.to->id;
            // Capture to_id by value; capture this by reference for fstar access
            auto del_btn = Button("Delete", [this, to_id, edge] {
                string respond = controler->deleteEdge(edge);
                status_msg = respond;
                this->rebuildEdgeWeightBuffers = true;
            });

            auto apply_button = Button("Apply", [this, to_id, edge, input_val] {
                string respond = controler->modifyEdge(edge, *input_val);
                status_msg = respond;
                this->rebuildEdgeWeightBuffers = true;
            });

            auto row = ftxui::Renderer(
                Container::Horizontal({input, apply_button, del_btn}),
                [=] {
                    // capture w_input, del_btn, from, to by value
                    return hbox({
                        text(edge.from->name + " -[") | vcenter,
                        input->Render() | size(WIDTH, EQUAL, 9) | vcenter,
                        text("]-> " + edge.to->name + " ") | vcenter,
                        filler(),
                        apply_button->Render() | vcenter,
                        del_btn->Render() | color(Color::Red) | vcenter,
                    });
                }
            );
            edge_section->Add(row);
            edge_current_rows.push_back(row);
        }
    }

    Component EdgeSectionComponent() {
        auto in_weight = Input(&add_edge_weight, "Weight");
        auto add_edge_button = Button("Add Edge", [&, this] {
            string respond = controler->addEdge(selected_node, node_edge_to, add_edge_weight);
            this->status_msg = respond;
            this->rebuildEdgeWeightBuffers = true;
            this->node_edge_to = nullptr;
            this->is_select_node_to_mode = false;
        });
        auto select_node_to_checkbox = Checkbox("Select node to", &this->is_select_node_to_mode);
        auto calculate_weight_button = Button("Calculate weight", [&, this] {
            status_msg = controler->calculateEuclideanDistance(selected_node, node_edge_to, &add_edge_weight);
        });

        auto container = Container::Vertical({
            edge_section,
            in_weight,
            add_edge_button,
            select_node_to_checkbox,
            calculate_weight_button
        });

        return Renderer(
            container, [&, this, in_weight, add_edge_button,select_node_to_checkbox, calculate_weight_button] {
                Elements elements;
                if (this->rebuildEdgeWeightBuffers) {
                    rebuildEdgeWeightBuffers = false;
                    RebuildEdgeRows();
                }
                elements.push_back(edge_section->Render());
                elements.push_back(separatorDouble());
                elements.push_back(text("Add Edge") | color(Color::Green));
                elements.push_back(separator());
                elements.push_back(hbox(
                    text(selected_node->name + " -[") | vcenter,
                    in_weight->Render() | size(WIDTH, EQUAL, 9) | vcenter,
                    text("]-> " + (node_edge_to ? node_edge_to->name : "select node to")) | vcenter,
                    filler(),
                    add_edge_button->Render() | vcenter | color(Color::BlueLight)
                ));
                elements.push_back(hbox(
                    calculate_weight_button->Render() | vcenter,
                    filler(),
                    select_node_to_checkbox->Render() | vcenter
                ));

                return vbox(elements);
            });
    }

    Component AddNodeComponent() {
        auto name_in = Input(&this->add_node_name, "Name");
        auto x_in = Input(&this->add_node_x, "X");
        auto y_in = Input(&this->add_node_y, "Y");
        auto add_button = Button("Add New Node", [&,this] {
            fStar::Node *newNode = nullptr;
            string s = controler->addNode(this->add_node_name, add_node_x, add_node_y, &newNode);
            this->status_msg = s;

            this->SetSelectedNode(newNode);
        });
        auto preview_location = Checkbox("Preview location", &this->draw_last_click);

        auto container = Container::Vertical({
            name_in,
            x_in,
            y_in,
            add_button,
            preview_location
        });

        return Renderer(container, [&, name_in, x_in, y_in, add_button, preview_location] {
            Elements elements;

            elements.push_back(text(" Add Node ") | bold | color(Color::Green));
            elements.push_back(separatorLight());
            elements.push_back(hbox(text("Name: "), name_in->Render()));
            elements.push_back(hbox(text("X: "), x_in->Render()));
            elements.push_back(hbox(text("Y: "), y_in->Render()));
            elements.push_back(preview_location->Render());
            elements.push_back(add_button->Render() | color(Color::BlueLight));


            return vbox(elements);
        });
    }

    Component NodeInfoComponent() {
        auto name_input = Input(&edit_name, "node name");
        auto x_input = Input(&edit_x, "X coord");
        auto y_input = Input(&edit_y, "Y coord");

        auto apply_button = Button("  Apply Node Changes  ", [&, this] {
            string respond = controler->modifyNode(this->selected_node, this->edit_name, this->edit_x, this->edit_y);
            this->status_msg = respond;
        });
        auto delete_node_button = Button("  Delete node  ", [&]() {
            std::string name = selected_node->name;
            fstar->deleteNode(selected_node->id);
            SetSelectedNode(nullptr);
            this->status_msg = "Deleted node " + name;
        });

        auto edge_section_var = EdgeSectionComponent();

        auto container = Container::Vertical({
            name_input,
            x_input,
            y_input,
            apply_button,
            delete_node_button,
            edge_section_var
        });

        return Renderer(container,
                        [&, name_input, x_input, y_input, apply_button, delete_node_button, edge_section_var, this] {
                            Elements menu_elements;
                            coor cor = transformer->transform(selected_node);

                            menu_elements.push_back(text(" Edit Node ") | bold | color(Color::Green));
                            menu_elements.push_back(separator());
                            menu_elements.push_back(text("ID: "+ std::to_string(this->selected_node->id)));
                            menu_elements.push_back(hbox({text("Name : "), name_input->Render()}));
                            menu_elements.push_back(hbox({text("Pos X : "), x_input->Render()}));
                            menu_elements.push_back(hbox({text("Pos Y : "), y_input->Render()}));
                            menu_elements.push_back(text("Drawed X: " + std::to_string(cor.x)));
                            menu_elements.push_back(text("Drawed Y: " + std::to_string(cor.y)));
                            menu_elements.push_back(separator());
                            menu_elements.push_back(hbox(apply_button->Render(), filler(),
                                                         delete_node_button->Render() | color(Color::Red)));
                            menu_elements.push_back(separatorDouble());
                            menu_elements.push_back(text(" Out Edges ") | bold | color(Color::Green));
                            menu_elements.push_back(separator());
                            menu_elements.push_back(edge_section_var->Render());

                            return vbox(menu_elements) | border;
                        });
    }

    Component DangerOperationsComponent() {
        auto recalculate_distances_button = Button("Recalculate All Distances", [&, this] {
            this->SetSelectedNode(nullptr);
            this->node_edge_to = nullptr;

            status_msg = controler->recalculateAllDistances();
        });

        auto colab = Collapsible("Danger", recalculate_distances_button | color(Color::Orange1));

        auto container = Container::Vertical({
            colab,
        });

        return Renderer(container, [&, colab] {
            Elements elements;

            elements.push_back(colab->Render());

            return vbox(elements) | border | color(Color::Red);
        });
    };

    Component LoadSaveComponent() {
        auto in_path_load_from = Input(&this->path_load_from, "path where to load");
        auto load_button = Button("Load", [&, this] {
            status_msg = controler->load(this->path_load_from);
        });

        auto in_path_save_from = Input(&this->path_save_to, "path where to save");
        auto save_button = Button("Save", [&, this] {
            status_msg = controler->save(this->path_save_to);
        });

        auto container = Container::Vertical({
            in_path_load_from,
            in_path_save_from,
            save_button,
            load_button
        });

        auto calabs = Collapsible("Load Save", Renderer(
                                      container, [&, in_path_load_from, in_path_save_from, load_button, save_button] {
                                          Elements elements;

                                          elements.push_back(hbox(
                                              in_path_load_from->Render(),
                                              filler(),
                                              load_button->Render() | color(Color::Orange3)
                                          ));
                                          elements.push_back(hbox(
                                              in_path_save_from->Render(),
                                              filler(),
                                              save_button->Render() | color(Color::GreenLight)
                                          ));

                                          return vbox(elements) | color(Color::Default);
                                      })) | color(Color::Green);


        return calabs | borderDashed | color(Color::CyanLight);
    }

    Component GraphComponent() {
        auto graphContainer = Renderer([&] {
            // automatically scale gaph
            int dynamic_w = std::max(100, graph_panel_size * 2);
            int dynamic_h = std::max(270, static_cast<int>(300 * _canvas_zoom));

            c = Canvas(dynamic_w, dynamic_h);
            DrawEdges();
            DrawNodes();
            if (draw_last_click && !selected_node) {
                coor cor = coor{last_click_node_x, last_click_node_y};
                cor = this->transformer->transform(cor);
                cor = cor * 2 + graph_text_draw_offset;
                c.DrawText(cor.x, cor.y, "x");
            }


            std::string status_text = "Zoom: " + std::to_string(_canvas_zoom).substr(0, 4) +
                                      " | PanX: " + std::to_string((int) _canvas_pan_x) +
                                      " | PanY: " + std::to_string((int) _canvas_pan_y) +
                                      " | MenuScroll: " + std::to_string(menu_scroll_offset) +
                                      " | ClickX: " + std::to_string(last_click_node_x) +
                                      " | ClickY: " + std::to_string(last_click_node_y) +
                                      // " | MouseX: " + std::to_string(canvas_mouse_x) +
                                      // " | MouseY: " + std::to_string(canvas_mouse_y) +
                                      " | ToMode: " + (is_select_node_to_mode ? "Y" : "N");
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
                if (_canvas_zoom < 0.2f) _canvas_zoom = 0.2f; // Safe bottom
                return true;
            }

            // L CLICK - SELECT
            if (mouse.button == Mouse::Left && mouse.motion == Mouse::Pressed) {
                fStar::Node *clicked_node = FindClickedNode(canvas_mouse_x, canvas_mouse_y);
                if (this->is_select_node_to_mode) {
                    this->node_edge_to = clicked_node;
                } else {
                    SetSelectedNode(clicked_node);
                }


                //   canvas_x = node_x * zoom + pan_x
                coor mouseClickCoord = coor{float(mouse.x), float(mouse.y * 2)};
                mouseClickCoord = this->rTransformer->reverseTransform(mouseClickCoord);
                this->last_click_node_x = mouseClickCoord.x;
                this->last_click_node_y = mouseClickCoord.y;

                this->add_node_x = std::to_string(last_click_node_x);
                this->add_node_y = std::to_string(last_click_node_y);

                return clicked_node != nullptr;
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
                _canvas_pan_y -= delta_y * 2;
                return true;
            }

            return false;
        });
        return graphComponent;
    }

    Component MenuComponent() {
        auto node_info_section = NodeInfoComponent();
        auto add_node_section = AddNodeComponent();
        auto load_save_section = LoadSaveComponent();
        auto danger_section = DangerOperationsComponent();

        auto info_tab = Container::Tab(
            {
                Renderer([] {
                    return vbox(text(" MENU ") | bold,
                                separator(),
                                text("Click on node to select.")) | border;
                }),
                node_info_section,
            },
            &active_node_tab
        );

        auto container = Container::Vertical({
            info_tab,
            add_node_section,
            danger_section,
            load_save_section
        });

        auto menuRenderer = Renderer(
            container,
            [&, info_tab, add_node_section, danger_section, load_save_section] {
                active_node_tab = (selected_node != nullptr) ? 1 : 0;

                // Update height every frame
                menu_visible_height = Terminal::Size().dimy;

                Elements all_lines;
                all_lines.push_back(info_tab->Render());
                all_lines.push_back(separator());
                all_lines.push_back(add_node_section->Render() | border);
                all_lines.push_back(separator());
                all_lines.push_back(danger_section->Render());
                all_lines.push_back(separator());
                all_lines.push_back(load_save_section->Render());
                if (!status_msg.empty()) {
                    all_lines.push_back(separator());
                    all_lines.push_back(text(status_msg) | color(Color::Yellow));
                }
                all_lines.push_back(separator());
                all_lines.push_back(text("[Home] exit ToMode | [Alt+S] toggle ToMode") | dim);
                all_lines.push_back(text("[PgUp/PgDn] or scroll to scroll menu") | dim);
                all_lines.push_back(text("[Alt+R] to reset scroll") | dim);

                // THIS is the key: hard pixel height on the outer box
                // focusPosition tells yframe which Y pixel to center on
                // yframe then clips to the hard size() constraint
                return vbox(std::move(all_lines))
                       | focusPosition(0, menu_scroll_offset)
                       | yframe
                       | size(HEIGHT, EQUAL, menu_visible_height);
                // no vscroll_indicator - draw our own since we know the metrics
            });

        return CatchEvent(menuRenderer, [&](Event event) {
            menu_visible_height = Terminal::Size().dimy;

            // Page-style jumps work better than line-by-line since elements are tall
            if (event == Event::ArrowDown) {
                menu_scroll_offset += 3;
                return true;
            }
            if (event == Event::ArrowUp) {
                menu_scroll_offset = max(menu_scroll_offset-3,0);
                return true;
            }
            if (event == Event::PageDown) {
                menu_scroll_offset += menu_visible_height / 2;
                return true;
            }
            if (event == Event::PageUp) {
                menu_scroll_offset -= menu_visible_height / 2;
                return true;
            }

            if (event.is_mouse()) {
                auto &m = event.mouse();
                if (m.x > graph_panel_size) {
                    if (m.button == Mouse::WheelDown) {
                        menu_scroll_offset += 3;
                        return true;
                    }
                    if (m.button == Mouse::WheelUp) {
                        menu_scroll_offset = max(menu_scroll_offset-3,0);
                        return true;
                    }
                }
            }
            return false;
        });
    }

    Component MatrixComponent() {
        return Renderer([&] {
            Elements rows;
            rows.push_back(text(" Distance Matrix ") | bold | color(Color::Cyan));
            rows.push_back(separator());

            if (!controler->D()) {
                rows.push_back(text("No matrix available."));
                return vbox(std::move(rows)) | border;
            }

            // Header row: node names
            Elements header_cells;
            header_cells.push_back(text("     ") | size(WIDTH, EQUAL, 8));

            std::vector<std::vector<string> > tableData = std::vector<std::vector<string> >();

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
                    const float curWeigt = (*controler->D())[(*it)->id][(*it2)->id];
                    row.push_back(std::to_string(curWeigt));
                }
                tableData.push_back(row);
            }

            auto table = ftxui::Table(tableData);
            table.SelectRows(0, -1).SeparatorVertical(LIGHT);
            table.SelectColumns(0, -1).SeparatorHorizontal(LIGHT);

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
    gui(FStar<NodeType> *fstar, Controler *controler) : fstar(fstar), controler(controler) {
        this->transformer = new Transformer();
        *transformer += Transformer::Scale(&_canvas_zoom);
        *transformer += Transformer::Move(&_canvas_pan_x, &_canvas_pan_y);
        *transformer += Transformer::FlipY(0.0);
        this->rTransformer = new Transformer();
        *rTransformer += Transformer::rScale(&_canvas_zoom);
        *rTransformer += Transformer::rMove(&_canvas_pan_x, &_canvas_pan_y);
        *rTransformer += Transformer::rFlipY(0.0);

        this->edge_section = Container::Vertical({});

        // this->_canvas_pan_x = fstar->getMinX();
        // this->_canvas_pan_y = fstar->getMaxY();
    }

    ~gui() {
        delete transformer;
        delete rTransformer;

        for (string *s: edge_weight_buffers) {
            delete s;
        }
    }

    void run() {
        auto screen = ScreenInteractive::Fullscreen();

        auto graph_component = GraphComponent();
        auto menu_component = MenuComponent();
        auto matrix_component = MatrixComponent();

        // Tab bar //leftpanel + right pannel
        std::vector<std::string> tab_labels = {" Graph ", " Distance Matrix "};
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
                         ) | (is_select_node_to_mode ? color(Color::Red) : color(Color::Default));


        auto main_component = CatchEvent(full_view, [&](Event event) {
            // this->is_select_node_to_mode_key_pressed = event==Event::AltS;
            // if (this->is_select_node_to_mode_key_pressed != this->is_select_node_to_mode_key_lastState) {
            //     this->is_select_node_to_mode=this->is_select_node_to_mode_key_pressed;
            //
            //     this->is_select_node_to_mode_key_lastState = this->is_select_node_to_mode_key_pressed;
            // }
            if (event == Event::AltS) {
                this->is_select_node_to_mode = !this->is_select_node_to_mode;
                return true;
            }

            if (event == Event::Home) {
                this->is_select_node_to_mode = false;
                return true;
            }

            if (event == Event::AltR) {
                menu_scroll_offset = 0;
                return true;
            }


            // if (event == Event::Character('q')) {
            //     screen.ExitLoopClosure()();
            //     return true;
            // }
            // 'r' shortcut to recalculate matrix from anywhere
            // if (event == Event::Character('r')) {
            //     //RebuildDistanceMatrix();
            //     return true;
            // }
            return false;
        });
        screen.Loop(main_component);
    }
};
