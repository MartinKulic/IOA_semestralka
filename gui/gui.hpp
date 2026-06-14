#pragma once

#include <ftxui/component/component.hpp>          // Renderer, ResizableSplitLeft, CatchEvent, Maybe
#include <ftxui/component/screen_interactive.hpp> // ScreenInteractive
#include <ftxui/component/event.hpp>              // Event, Mouse
#include <ftxui/dom/table.hpp>
#include <memory>
#include <string>
#include <deque>
#include <vector>
#include <cmath>
#include <algorithm>

#include "controler.h"
#include "Transformer.hpp"
#include "../fStar/Alg.hpp"
#include "../fStar/fStar.hpp"

using namespace ftxui;
using namespace fStar;

class gui {
private:
    FStar     *fstar;
    Controler *controler;

    Canvas c;
    std::unique_ptr<Transformer> transformer;
    std::unique_ptr<Transformer> rTransformer;

    const coor graph_text_draw_offset{-4, -6};

    float _canvas_zoom  = 1.0f;
    float _canvas_pan_x = 10.0f;
    float _canvas_pan_y = -100.0f;
    bool  is_dragging   = false;

    int active_tab       = 0;
    int graph_panel_size = 70;

    fStar::Node *selected_node = nullptr;
    int   canvas_mouse_x    = 0;
    int   canvas_mouse_y    = 0;
    float last_click_node_x = 0.0f;
    float last_click_node_y = 0.0f;
    bool  draw_last_click   = true;

    // Node edit fields – kept in sync by SetSelectedNode()
    std::string edit_name;
    std::string edit_x;
    std::string edit_y;

    // Add-node fields – populated from canvas left-click
    std::string add_node_name = "Name";
    std::string add_node_x;
    std::string add_node_y;

    // Edge list state.
    // std::deque provides pointer stability on push_back, so raw pointers
    // passed to Input() remain valid for the lifetime of the deque entry.
    std::deque<std::unique_ptr<std::string>> edge_weight_buffers;
    fStar::Node *edge_section_built_for = nullptr; // which node the rows were built for
    Component    edge_section_container;           // Container::Vertical, rebuilt on demand

    std::string  add_edge_weight        = "10";
    fStar::Node *node_edge_to           = nullptr;
    bool         is_select_node_to_mode = false;

    std::string path_load_from = "../save";
    std::string path_save_to   = "../save";
    std::string status_msg     = "Status msg";

    int menu_scroll_offset = 0;

    // ─────────────────────────────────────────────────────────── drawing ─────

    void DrawNodes() {
        for (auto it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node *n   = *it;
            coor          cor = transformer->transform(n);
            int sx = cor.x * 2 + graph_text_draw_offset.x;
            int sy = cor.y * 2 + graph_text_draw_offset.y;
            if      (n == selected_node) c.DrawText(sx, sy, "< " + n->name + " >", Color::Green);
            else if (n == node_edge_to)  c.DrawText(sx, sy, "> " + n->name + " <");
            else                          c.DrawText(sx, sy, n->name);
        }
    }

    void DrawEdges() {
        for (auto it = fstar->begin_edges(); it != fstar->end_edges(); ++it) {
            fStar::Edge e  = *it;
            coor c1 = transformer->transform(e.from) * 2 + graph_text_draw_offset;
            coor c2 = transformer->transform(e.to)   * 2 + graph_text_draw_offset;
            c.DrawPointLine(c1.x, c1.y, c2.x, c2.y);
            c.DrawText((c1.x + c2.x) / 2, (c1.y + c2.y) / 2, std::to_string(e.weight));
        }
    }

    fStar::Node *FindClickedNode(int mx, int my) {
        if (mx >= graph_panel_size) return nullptr;
        int tx = mx * 2 - graph_text_draw_offset.x;
        int ty = my * 2 - graph_text_draw_offset.y - 5;
        constexpr int tol = 4;
        for (auto it = fstar->begin_nodes(); it != fstar->end_nodes(); ++it) {
            fStar::Node *n   = *it;
            coor          cor = transformer->transform(n);
            if (std::abs(cor.x * 2 - tx) <= tol &&
                std::abs(cor.y     - ty) <= tol)
                return n;
        }
        return nullptr;
    }

    void SetSelectedNode(fStar::Node *node) {
        // Always invalidate the edge list so pointer comparison in
        // RebuildEdgeSectionChildren() never touches a dangling node pointer.
        edge_section_built_for = nullptr;
        selected_node = node;
        if (node) {
            edit_name = node->name;
            edit_x    = std::to_string(node->x);
            edit_y    = std::to_string(node->y);
        } else {
            edit_name.clear();
            edit_x.clear();
            edit_y.clear();
        }
    }

    // ──────────────────────────────────────── edge-list rebuild ──────────────
    //
    // FTXUI Input components are stateful: they hold a raw pointer to an
    // external std::string.  For a variable-length list (one Input per
    // out-edge) the component subtree must be rebuilt whenever edges change.
    //
    // The idiomatic FTXUI mechanism:
    //   1. DetachAllChildren() – drops child shared_ptr refcounts to zero,
    //      destroying the old row components (and their captured closures).
    //   2. Clear string buffers – safe because the Inputs that referenced
    //      them are already destroyed after step 1.
    //   3. Build fresh rows and Add() them back to the same container.
    //
    // EdgeSectionComponent's Renderer triggers a rebuild whenever
    // edge_section_built_for != selected_node.

    void RebuildEdgeSectionChildren() {
        // 1. Destroy old row components (and their Input references).
        edge_section_container->DetachAllChildren();

        // 2. Free string buffers – safe because their Inputs are gone.
        edge_weight_buffers.clear();

        if (!selected_node) {
            edge_section_built_for = selected_node;
            return;
        }

        // 3. Build one interactive row per out-edge.
        for (auto it  = fstar->begin_out_edges(selected_node->id),
                  end = fstar->end_out_edges(selected_node->id);
             it != end; ++it)
        {
            fStar::Edge edge = *it;

            // Buffer lives in the deque; pointer is stable across push_back.
            edge_weight_buffers.push_back(
                std::make_unique<std::string>(std::to_string(edge.weight)));
            std::string *buf = edge_weight_buffers.back().get();

            auto w_in      = Input(buf, "w");
            auto del_btn   = Button("Delete", [this, edge] {
                status_msg             = controler->deleteEdge(edge);
                edge_section_built_for = nullptr; // request rebuild next frame
            });
            auto apply_btn = Button("Apply", [this, edge, buf] {
                status_msg             = controler->modifyEdge(edge, *buf);
                edge_section_built_for = nullptr;
            });

            auto row = Renderer(
                Container::Horizontal({w_in, apply_btn, del_btn}),
                [=] {
                    return hbox({
                        text(edge.from->name + " -[") | vcenter,
                        w_in->Render() | size(WIDTH, EQUAL, 9) | vcenter,
                        text("]-> " + edge.to->name + " ") | vcenter,
                        filler(),
                        apply_btn->Render() | vcenter,
                        del_btn->Render()   | color(Color::Red) | vcenter,
                    });
                });

            edge_section_container->Add(row);
        }

        edge_section_built_for = selected_node;
    }

    // ─────────────────────────────────────────────── component factories ─────

    Component EdgeSectionComponent() {
        auto in_weight = Input(&add_edge_weight, "Weight");
        auto add_btn   = Button("Add Edge", [&] {
            status_msg             = controler->addEdge(selected_node, node_edge_to, add_edge_weight);
            edge_section_built_for = nullptr;
            node_edge_to           = nullptr;
            is_select_node_to_mode = false;
        });
        auto sel_cb   = Checkbox("Select node to", &is_select_node_to_mode);
        auto calc_btn = Button("Calculate weight", [&] {
            status_msg = controler->calculateEuclideanDistance(
                selected_node, node_edge_to, &add_edge_weight);
        });

        auto container = Container::Vertical({
            edge_section_container, in_weight, add_btn, sel_cb, calc_btn,
        });

        return Renderer(container, [&, in_weight, add_btn, sel_cb, calc_btn] {
            // Rebuild the interactive row list whenever the edge set may have changed.
            if (edge_section_built_for != selected_node)
                RebuildEdgeSectionChildren();

            return vbox({
                edge_section_container->Render(),
                separatorDouble(),
                text("Add Edge") | color(Color::Green),
                separator(),
                hbox({
                    text(selected_node->name + " -[") | vcenter,
                    in_weight->Render() | size(WIDTH, EQUAL, 9) | vcenter,
                    text("]-> " + (node_edge_to ? node_edge_to->name
                                               : std::string("select node to"))) | vcenter,
                    filler(),
                    (is_select_node_to_mode
                       ? add_btn->Render() | vcenter | color(Color::BlueLight) | blink
                       : add_btn->Render() | vcenter | color(Color::BlueLight)),
                }),
                hbox({
                    calc_btn->Render() | vcenter,
                    filler(),
                    sel_cb->Render()   | vcenter,
                }),
            });
        });
    }

    Component AddNodeComponent() {
        auto name_in = Input(&add_node_name, "Name");
        auto x_in    = Input(&add_node_x,    "X");
        auto y_in    = Input(&add_node_y,    "Y");
        auto add_btn = Button("Add New Node", [&] {
            fStar::Node *n = nullptr;
            status_msg = controler->addNode(add_node_name, add_node_x, add_node_y, &n);
            SetSelectedNode(n);
        });
        auto prev_cb = Checkbox("Preview location", &draw_last_click);

        return Renderer(
            Container::Vertical({name_in, x_in, y_in, add_btn, prev_cb}),
            [name_in, x_in, y_in, add_btn, prev_cb] {
                return vbox({
                    text(" Add Node ") | bold | color(Color::Green),
                    separatorLight(),
                    hbox({text("Name: "), name_in->Render()}),
                    hbox({text("X: "),    x_in->Render()}),
                    hbox({text("Y: "),    y_in->Render()}),
                    prev_cb->Render(),
                    add_btn->Render() | color(Color::BlueLight),
                });
            });
    }

    Component NodeInfoComponent() {
        auto name_in   = Input(&edit_name, "node name");
        auto x_in      = Input(&edit_x,    "X coord");
        auto y_in      = Input(&edit_y,    "Y coord");
        auto apply_btn = Button("  Apply Node Changes  ", [&] {
            status_msg = controler->modifyNode(selected_node, edit_name, edit_x, edit_y);
        });
        auto del_btn   = Button("  Delete node  ", [&] {
            std::string name = selected_node->name;
            fstar->deleteNode(selected_node->id);
            SetSelectedNode(nullptr);
            status_msg = "Deleted node " + name;
        });
        auto edge_sec  = EdgeSectionComponent();

        return Renderer(
            Container::Vertical({name_in, x_in, y_in, apply_btn, del_btn, edge_sec}),
            [&, name_in, x_in, y_in, apply_btn, del_btn, edge_sec] {
                coor cor = transformer->transform(selected_node);
                return vbox({
                    text(" Edit Node ") | bold | color(Color::Green),
                    separator(),
                    text("ID: " + std::to_string(selected_node->id)),
                    hbox({text("Name : "),  name_in->Render()}),
                    hbox({text("Pos X : "), x_in->Render()}),
                    hbox({text("Pos Y : "), y_in->Render()}),
                    text("Drawn X: " + std::to_string(cor.x)),
                    text("Drawn Y: " + std::to_string(cor.y)),
                    separator(),
                    hbox({apply_btn->Render(), filler(), del_btn->Render() | color(Color::Red)}),
                    separatorDouble(),
                    text(" Out Edges ") | bold | color(Color::Green),
                    separator(),
                    edge_sec->Render(),
                }) | border;
            });
    }

    Component DangerOperationsComponent() {
        auto recalc_btn = Button("Recalculate All Distances", [&] {
            SetSelectedNode(nullptr);
            node_edge_to = nullptr;
            status_msg   = controler->recalculateAllDistances();
        });
        // Renderer(base, fn) uses base for focus/event routing and fn for
        // rendering only – the minimal wrapper to apply element decorators.
        auto colap = Collapsible("Danger", recalc_btn | color(Color::Orange1));
        return Renderer(colap, [colap] {
            return colap->Render() | border | color(Color::Red);
        });
    }

    Component LoadSaveComponent() {
        auto load_in  = Input(&path_load_from, "path to load from");
        auto load_btn = Button("Load", [&] { status_msg = controler->load(path_load_from); });
        auto save_in  = Input(&path_save_to,   "path to save to");
        auto save_btn = Button("Save", [&] { status_msg = controler->save(path_save_to); });

        auto inner = Container::Vertical({load_in, save_in, load_btn, save_btn});

        return Collapsible("Load Save",
            Renderer(inner, [load_in, load_btn, save_in, save_btn] {
                return vbox({
                    hbox({load_in->Render(), filler(), load_btn->Render() | color(Color::Orange3)}),
                    hbox({save_in->Render(), filler(), save_btn->Render() | color(Color::GreenLight)}),
                }) | color(Color::Default);
            })
        ) | color(Color::Green) | borderDashed | color(Color::CyanLight);
    }

    Component GraphComponent() {
        auto graph_rndr = Renderer([&] {
            int dyn_w = std::max(100, graph_panel_size * 2);
            int dyn_h = std::max(270, static_cast<int>(300 * _canvas_zoom));
            c = Canvas(dyn_w, dyn_h);
            DrawEdges();
            DrawNodes();

            if (draw_last_click && !selected_node) {
                coor cor = transformer->transform(coor{last_click_node_x, last_click_node_y});
                cor = cor * 2 + graph_text_draw_offset;
                c.DrawText(cor.x, cor.y, "x");
            }

            return vbox({
                canvas(&c) | flex,
                separator(),
                text("Zoom: "     + std::to_string(_canvas_zoom).substr(0, 4)         +
                     " | PanX: "  + std::to_string(static_cast<int>(_canvas_pan_x))   +
                     " | PanY: "  + std::to_string(static_cast<int>(_canvas_pan_y))   +
                     " | ClickX: "+ std::to_string(last_click_node_x)                 +
                     " | ClickY: "+ std::to_string(last_click_node_y)                 +
                     " | ToMode: "+ (is_select_node_to_mode ? "Y" : "N"))
                    | color(Color::Yellow),
            }) | border;
        });

        return CatchEvent(graph_rndr, [&](Event event) -> bool {
            if (!event.is_mouse()) return false;
            auto &m = event.mouse();
            if (m.x >= graph_panel_size) return false;

            int prev_x     = canvas_mouse_x;
            int prev_y     = canvas_mouse_y;
            canvas_mouse_x = m.x - 1;
            canvas_mouse_y = m.y - 1;

            if (m.button == Mouse::WheelUp) {
                _canvas_zoom = std::min(_canvas_zoom * 1.1f, 10.0f);
                return true;
            }
            if (m.button == Mouse::WheelDown) {
                _canvas_zoom = std::max(_canvas_zoom / 1.1f, 0.2f);
                return true;
            }
            if (m.button == Mouse::Left && m.motion == Mouse::Pressed) {
                fStar::Node *hit = FindClickedNode(canvas_mouse_x, canvas_mouse_y);
                if (is_select_node_to_mode) node_edge_to = hit;
                else                       SetSelectedNode(hit);

                coor wp = rTransformer->reverseTransform({float(m.x), float(m.y * 2)});
                last_click_node_x = wp.x;
                last_click_node_y = wp.y;
                add_node_x = std::to_string(wp.x);
                add_node_y = std::to_string(wp.y);
                return hit != nullptr;
            }
            if (m.button == Mouse::Right) {
                if (m.motion == Mouse::Pressed)  { is_dragging = true;  return true; }
                if (m.motion == Mouse::Released) { is_dragging = false; return true; }
            }
            if (m.motion == Mouse::Moved && is_dragging) {
                _canvas_pan_x += canvas_mouse_x - prev_x;
                _canvas_pan_y -= (canvas_mouse_y - prev_y) * 2;
                return true;
            }
            return false;
        });
    }

    Component MenuComponent() {
        // Maybe is the canonical FTXUI primitive for conditional visibility.
        // It also gates focus and event routing, replacing the Container::Tab
        // + active_node_tab integer workaround from the original code.
        auto no_sel_help = Maybe(
            Renderer([] {
                return vbox({
                    text(" MENU ") | bold,
                    separator(),
                    text("Click on node to select."),
                }) | border;
            }),
            [&] { return selected_node == nullptr; });

        auto node_info_maybe = Maybe(NodeInfoComponent(),
                                     [&] { return selected_node != nullptr; });

        auto add_node_section  = AddNodeComponent();
        auto danger_section    = DangerOperationsComponent();
        auto load_save_section = LoadSaveComponent();

        auto container = Container::Vertical({
            no_sel_help, node_info_maybe,
            add_node_section, danger_section, load_save_section,
        });

        // focusPosition + yframe is the correct FTXUI primitive for a panel
        // that must scroll independently of keyboard focus.  vscroll_indicator
        // renders a proportional scroll bar alongside the clipped content.
        auto menu_rndr = Renderer(container,
            [&, no_sel_help, node_info_maybe,
               add_node_section, danger_section, load_save_section]
        {
            Elements lines;
            lines.push_back(no_sel_help->Render());
            lines.push_back(node_info_maybe->Render());
            lines.push_back(separator());
            lines.push_back(add_node_section->Render()  | border);
            lines.push_back(separator());
            lines.push_back(danger_section->Render());
            lines.push_back(separator());
            lines.push_back(load_save_section->Render());
            if (!status_msg.empty()) {
                lines.push_back(separator());
                lines.push_back(text(status_msg) | color(Color::Yellow));
            }
            lines.push_back(separator());
            lines.push_back(text("[Home] exit ToMode | [Alt+S] toggle ToMode") | dim);
            lines.push_back(text("[PgUp/PgDn] or scroll to scroll menu")        | dim);
            lines.push_back(text("[Alt+R] reset scroll")                         | dim);

            return vbox(std::move(lines))
                   | focusPosition(0, menu_scroll_offset)
                   | yframe
                   | vscroll_indicator
                   | size(HEIGHT, EQUAL, Terminal::Size().dimy);
        });

        return CatchEvent(menu_rndr, [&](Event event) -> bool {
            const int step = 3;
            const int page = std::max(1, Terminal::Size().dimy / 2);

            if (event == Event::ArrowDown) { menu_scroll_offset += step;                            return true; }
            if (event == Event::ArrowUp)   { menu_scroll_offset  = std::max(0, menu_scroll_offset - step); return true; }
            if (event == Event::PageDown)  { menu_scroll_offset += page;                            return true; }
            if (event == Event::PageUp)    { menu_scroll_offset  = std::max(0, menu_scroll_offset - page); return true; }

            if (event.is_mouse() && event.mouse().x > graph_panel_size) {
                auto &m = event.mouse();
                if (m.button == Mouse::WheelDown) { menu_scroll_offset += step;                            return true; }
                if (m.button == Mouse::WheelUp)   { menu_scroll_offset  = std::max(0, menu_scroll_offset - step); return true; }
            }
            return false;
        });
    }

    Component MatrixComponent() {
        return Renderer([&] {
            if (!controler->D())
                return vbox({
                    text(" Distance Matrix ") | bold | color(Color::Cyan),
                    separator(),
                    text("No matrix available."),
                }) | border;

            auto end = fstar->end_nodes();
            std::vector<std::vector<std::string>> data;

            // Header row (first cell blank)
            {
                std::vector<std::string> hdr{""};
                for (auto it = fstar->begin_nodes(); it != end; ++it)
                    hdr.push_back((*it)->name);
                data.push_back(std::move(hdr));
            }
            for (auto it = fstar->begin_nodes(); it != end; ++it) {
                std::vector<std::string> row{(*it)->name};
                for (auto it2 = fstar->begin_nodes(); it2 != end; ++it2)
                    row.push_back(std::to_string((*controler->D())[(*it)->id][(*it2)->id]));
                data.push_back(std::move(row));
            }

            auto tbl = ftxui::Table(data);
            tbl.SelectAll()         .Border(LIGHT);
            tbl.SelectRows(0, -1)   .SeparatorVertical(LIGHT);
            tbl.SelectColumns(0, -1).SeparatorHorizontal(LIGHT);
            tbl.SelectRow(0)        .Decorate(bold);
            tbl.SelectRow(0)        .Border(HEAVY);
            tbl.SelectRow(0)        .DecorateCells(center);
            tbl.SelectColumn(0)     .Decorate(bold);
            tbl.SelectColumn(0)     .Border(HEAVY);
            tbl.SelectColumn(0)     .DecorateCells(center);

            return vbox({
                text(" Distance Matrix ") | bold | color(Color::Cyan),
                separator(),
                tbl.Render(),
            }) | border | flex;
        });
    }

public:
    gui(FStar *fstar, Controler *controler)
        : fstar(fstar)
        , controler(controler)
        , transformer(std::make_unique<Transformer>())
        , rTransformer(std::make_unique<Transformer>())
        , edge_section_container(Container::Vertical({}))
    {
        *transformer  += Transformer::Scale(&_canvas_zoom);
        *transformer  += Transformer::Move(&_canvas_pan_x, &_canvas_pan_y);
        *transformer  += Transformer::FlipY(0.0);

        *rTransformer += Transformer::rScale(&_canvas_zoom);
        *rTransformer += Transformer::rMove(&_canvas_pan_x, &_canvas_pan_y);
        *rTransformer += Transformer::rFlipY(0.0);
    }

    void run() {
        auto screen = ScreenInteractive::Fullscreen();

        std::vector<std::string> tab_labels = {" Graph ", " Distance Matrix "};
        auto tab_toggle  = Toggle(&tab_labels, &active_tab);
        auto tab_content = Container::Tab({GraphComponent(), MatrixComponent()}, &active_tab);

        auto left = Renderer(
            Container::Vertical({tab_toggle, tab_content}),
            [tab_toggle, tab_content] {
                return vbox({tab_toggle->Render(), tab_content->Render() | flex});
            });

        auto split = ResizableSplitLeft(left, MenuComponent(), &graph_panel_size);

        // Wrap in a Renderer so the select-to-mode tint is re-evaluated every
        // frame.  The original code applied the color once at construction via
        // the conditional operator, so the tint never actually activated.
        auto tinted = Renderer(split, [&, split] {
            Element e = split->Render();
            return is_select_node_to_mode ? e | color(Color::DarkOrange) : e;
        });

        auto main = CatchEvent(tinted, [&](Event event) -> bool {
            if (event == Event::AltS) { is_select_node_to_mode = !is_select_node_to_mode; return true; }
            if (event == Event::Home) { is_select_node_to_mode  = false;                  return true; }
            if (event == Event::AltR) { menu_scroll_offset      = 0;                      return true; }
            return false;
        });

        screen.Loop(main);
    }
};