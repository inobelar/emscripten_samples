#include <imgui_app/ImGui_Application.hpp>

#include <imgui.h>

#include <taffy/tree/taffy_tree/tree/Taffy.hpp>
#include <taffy/style/mod/StyleBuilder.hpp>

#include <ImGui_Taffy/show_descriptions.hpp>
#include <ImGui_Taffy/open_link_func.hpp>
#include <ImGui_Taffy/types/style/mod/Style.hpp>
#include <ImGui_Taffy/node_draw_recursive.hpp>

// -----------------------------------------------------------------------------

// TODO: due to current limitation of Taffy API, we cannot get parrent from node,
// so we need to pass it explicetely. When we be able to do it - we need to
// remove 'parent' argument here.
void draw_nodes_tree(
    taffy::Taffy* tree,
    const taffy::NodeId parent,
    const taffy::NodeId node,

    taffy::Option<taffy::NodeId>& focused_node)
{
    const bool is_root = (parent == node);
    const bool is_focused = (focused_node.is_some() && focused_node.value() == node);

    ImGuiTreeNodeFlags tree_node_flags
        = ImGuiTreeNodeFlags_OpenOnArrow;

    if(is_focused) {
        tree_node_flags |= ImGuiTreeNodeFlags_Selected;
    }

    const uint64_t node_id = static_cast<uint64_t>(node);

    char id_buffer[128];
    snprintf(id_buffer, 128, "node_%llu", node_id);

    // We need to combine the next actions for each node leaf:
    //   - setting node as 'focused' - action: 'click'
    //   - opening node              - action: click over 'arrow'

    const bool tree_opened = ImGui::TreeNodeEx(id_buffer, tree_node_flags, "NodeId (%llu)", node_id);
    if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        focused_node = taffy::Option<taffy::NodeId>(node_id);
    }

    if( tree->dirty(node).unwrap() )
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", "Dirty");
    }

    if(tree_opened)
    {
        bool removed = false;

        // Buttons
        {
            if( ImGui::Button("[+] Add child node") )
            {
                const auto leaf = tree->new_leaf(taffy::StyleBuilder([](taffy::Style& s)
                {
                    // For convenience, in this demo, created leaf have size 100x100 (for easier selection)
                    s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Length(100.0f), taffy::Dimension::Length(100.0f) };
                })).unwrap();

                tree->add_child(node, leaf).unwrap();
            }


            if(!is_root)
            {
                ImGui::SameLine();
                if( ImGui::Button("[x] Remove node") )
                {
                    if(focused_node.is_some()) {
                        focused_node = taffy::Option<taffy::NodeId>{}; // None
                    }

                    auto key = tree->remove_child(parent, node).unwrap();
                    tree->remove(key).unwrap();

                    removed = true;
                }
            }
        }

        if(!removed)
        {
            const auto childs = tree->Children(node).unwrap();
            for(const auto& child : childs)
            {
                draw_nodes_tree(tree, node, child, focused_node);
            }
        }

        ImGui::TreePop();
    }
}

// -----------------------------------------------------------------------------

void set_next_window_initial_pos_and_size(float pos_x, float pos_y, float size_x, float size_y)
{
    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to this application a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + pos_x, main_viewport->WorkPos.y + pos_y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(size_x, size_y), ImGuiCond_FirstUseEver);
}

// -----------------------------------------------------------------------------

void traverse_nodes_at_pos(
    const ImVec2& origin,

    const taffy::LayoutTree& tree,
    const taffy::NodeId node,

    const ImVec2& pos,
    std::vector<taffy::NodeId>& nodes)
{
    const taffy::Layout& layout = tree.impl_layout(node);

    const float left = layout.location.x;
    const float top  = layout.location.y;

    const float width  = layout.size.width;
    const float height = layout.size.height;

    const ImVec2 node_rect__top_left  = {origin.x + left,         origin.y + top};
    const ImVec2 node_rect__bot_right = {origin.x + left + width, origin.y + top + height};

    // Is 'node rect' contains 'pos'
    if( ((node_rect__top_left.x <= pos.x) && (node_rect__bot_right.x >= pos.x)) &&
        ((node_rect__top_left.y <= pos.y) && (node_rect__bot_right.y >= pos.y)) )
    {
        nodes.push_back(node);
    }

    const auto childs = tree.impl_children(node);
    for(const auto& child : childs)
    {
        traverse_nodes_at_pos(
            node_rect__top_left,

            tree,
            child,

            pos,
            nodes
        );
    }
}

std::vector<taffy::NodeId> get_nodes_at_pos(
    const ImVec2& origin,

    const taffy::LayoutTree& tree,
    const taffy::NodeId node,
    const ImVec2& pos)
{
    std::vector<taffy::NodeId> nodes;
    traverse_nodes_at_pos(origin, tree, node, pos, nodes);
    return nodes;
}

// -----------------------------------------------------------------------------

bool draw_nodes(
    const taffy::LayoutTree& tree,
    const taffy::NodeId node,
    taffy::Option<taffy::NodeId> &focused_node,
    const ImVec2& rect_size,
    const ImGui_Taffy::Node_Draw_Style* draw_style)
{
    bool focused_node_changed = false;

    // Get widget's rectangle dimentions
    const ImVec2 top_left  = ImGui::GetCursorScreenPos();
    const ImVec2 bot_right = ImVec2(top_left.x + rect_size.x, top_left.y + rect_size.y);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->PushClipRect(top_left, bot_right, true);
    {
        // This will catch our interactions
        constexpr ImGuiButtonFlags BUTTON_FLAGS =
            ImGuiButtonFlags_MouseButtonLeft |
            ImGuiButtonFlags_MouseButtonRight;
        ImGui::InvisibleButton("##placeholder", rect_size, BUTTON_FLAGS);

        const bool is_hovered = ImGui::IsItemHovered(); // Hovered
        const bool is_active  = ImGui::IsItemActive();  // Held

        // It must be 'static', since we access to it in 'popup', which lifetime
        // is greater then this scope.
        // NOTE: this is bad design, but suitable for this example
        static std::vector<taffy::NodeId> nodes_at_right_click_pos;

        if( (is_hovered && is_active) )
        {
            if( ImGui::IsMouseClicked(ImGuiMouseButton_Left) )
            {
                const ImVec2 mouse_pos = ImGui::GetMousePos();
                const ImVec2 mouse_pos_nodes_space = {mouse_pos.x - top_left.x, mouse_pos.y - top_left.y};

                 const auto nodes_at_left_click_pos =
                    get_nodes_at_pos(ImVec2(0, 0), tree, node, mouse_pos_nodes_space);

                if(!nodes_at_left_click_pos.empty())
                {
                    focused_node = nodes_at_left_click_pos.back(); // Pickup last node (deepest)
                    focused_node_changed = true;
                }
            }

            if( ImGui::IsMouseClicked(ImGuiMouseButton_Right) )
            {
                const ImVec2 mouse_pos = ImGui::GetMousePos();
                const ImVec2 mouse_pos_nodes_space = {mouse_pos.x - top_left.x, mouse_pos.y - top_left.y};

                nodes_at_right_click_pos =
                    get_nodes_at_pos(ImVec2(0, 0), tree, node, mouse_pos_nodes_space);

                if(!nodes_at_right_click_pos.empty())
                {
                    if(nodes_at_right_click_pos.size() == 1) // Single node found - immediately select it
                    {
                        focused_node = nodes_at_right_click_pos[0];
                        focused_node_changed = true;
                    }
                    else // Multiple nodes found - show popup to select
                    {
                        ImGui::OpenPopup("popup__focus_node");
                    }
                }
            }

        }

        // In case of 'multiple nodes' select - show popup
        if( ImGui::BeginPopup("popup__focus_node") )
        {
            for(const taffy::NodeId& n : nodes_at_right_click_pos)
            {
                char text_buf[64];
                snprintf(text_buf, 64, "NodeId (%llu)", static_cast<uint64_t>(n));

                const bool is_selected = (n == focused_node);

                if( ImGui::Selectable(text_buf, is_selected) )
                {
                    focused_node = n;
                    focused_node_changed = true;
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndPopup();
        }

        // ---------------------------------------------------------------------

        // Draw border around widget
        constexpr ImU32 BORDER_COLOR = IM_COL32_BLACK;
        draw_list->AddRect(top_left, bot_right, BORDER_COLOR);

        // Draw nodes
        ImGui_Taffy::node_draw_recursive(tree, node, draw_list, top_left, focused_node, draw_style);
    }
    draw_list->PopClipRect();

    return focused_node_changed;
}

// -----------------------------------------------------------------------------

// Notice, in the next examples, slightly modified 'root' node - changed
// size from fixed `::Length` 'width x height' into `::Percent` '100% x 100%'

taffy::NodeId set_example_basic(taffy::Taffy& tree)
{
    tree.clear();

    const auto child = tree.new_leaf(taffy::StyleBuilder([](taffy::Style& s) {
        s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Percent(0.5f), taffy::Dimension::Auto() };
    })).unwrap();

    const auto node = tree.new_with_children(
        taffy::StyleBuilder([](taffy::Style& s) {
            s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Percent(1.0f), taffy::Dimension::Percent(1.0f) };
            s.justify_content = taffy::Some(taffy::JustifyContent::Center);
        }),
        taffy::Vec<taffy::NodeId>{child}
    ).unwrap();

    return node;
}

taffy::NodeId set_example_flexbox_gap(taffy::Taffy& tree)
{
    tree.clear();

    const auto child_style = taffy::StyleBuilder([](taffy::Style& s) { s.size = taffy::Size<taffy::Dimension> { taffy::length<taffy::Dimension>(20.0f), taffy::length<taffy::Dimension>(20.0f) }; });
    const auto child0 = tree.new_leaf(child_style).unwrap();
    const auto child1 = tree.new_leaf(child_style).unwrap();
    const auto child2 = tree.new_leaf(child_style).unwrap();

    const auto root = tree.new_with_children(
        taffy::StyleBuilder([](taffy::Style& s) {
            s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Percent(1.0f), taffy::Dimension::Percent(1.0f) };
            s.gap = taffy::Size<taffy::LengthPercentage> { taffy::length<taffy::LengthPercentage>(10.0f), taffy::zero<taffy::LengthPercentage>() };
        }),
        taffy::Vec<taffy::NodeId>{child0, child1, child2}
    ).unwrap();

    return root;
}

#if defined(TAFFY_FEATURE_GRID)
taffy::NodeId set_example_holy_grail(taffy::Taffy& tree)
{
    tree.clear();

    // Setup the grid
    const auto root_style = taffy::StyleBuilder([](taffy::Style& s) {
        s.display = taffy::Display::Grid();
        s.size = taffy::Size<taffy::Dimension>{ taffy::percent<taffy::Dimension>(1.0f), taffy::percent<taffy::Dimension>(1.0f) };
        s.grid_template_columns = taffy::GridTrackVec<taffy::TrackSizingFunction>{ taffy::length<taffy::TrackSizingFunction>(250.0f), taffy::fr<taffy::TrackSizingFunction>(1.0f), taffy::length<taffy::TrackSizingFunction>(250.0f) };
        s.grid_template_rows = taffy::GridTrackVec<taffy::TrackSizingFunction>{ taffy::length<taffy::TrackSizingFunction>(150.0f), taffy::fr<taffy::TrackSizingFunction>(1.0f), taffy::length<taffy::TrackSizingFunction>(150.0f) };
    });

    // Define the child nodes
    const auto header = tree.new_leaf(taffy::StyleBuilder([](taffy::Style& s) { s.grid_row = taffy::line<taffy::Line<taffy::GridPlacement>>(1); s.grid_column = taffy::span<taffy::Line<taffy::GridPlacement>>(3); })).unwrap();
    const auto left_sidebar = tree.new_leaf(taffy::StyleBuilder([](taffy::Style& s) { s.grid_row = taffy::line<taffy::Line<taffy::GridPlacement>>(2); s.grid_column = taffy::line<taffy::Line<taffy::GridPlacement>>(1); })).unwrap();
    const auto content_area = tree.new_leaf(taffy::StyleBuilder([](taffy::Style& s) { s.grid_row = taffy::line<taffy::Line<taffy::GridPlacement>>(2); s.grid_column = taffy::line<taffy::Line<taffy::GridPlacement>>(2); })).unwrap();
    const auto right_sidebar = tree.new_leaf(taffy::StyleBuilder([](taffy::Style& s) { s.grid_row = taffy::line<taffy::Line<taffy::GridPlacement>>(2); s.grid_column = taffy::line<taffy::Line<taffy::GridPlacement>>(3); })).unwrap();
    const auto footer = tree.new_leaf(taffy::StyleBuilder([](taffy::Style& s) { s.grid_row = taffy::line<taffy::Line<taffy::GridPlacement>>(3); s.grid_column = taffy::span<taffy::Line<taffy::GridPlacement>>(3); })).unwrap();

    // Create the container with the children
    const auto root = tree.new_with_children(root_style, taffy::Vec<taffy::NodeId>{header, left_sidebar, content_area, right_sidebar, footer}).unwrap();

    return root;
}
#endif // TAFFY_FEATURE_GRID

taffy::NodeId set_example_nested(taffy::Taffy& tree)
{
    tree.clear();

    // left
    const auto child_t1 = tree.new_leaf(taffy::StyleBuilder([](taffy::Style& s) {
        s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Length(50.0f), taffy::Dimension::Length(50.0f) }; // Slightly enlarged size: 5x5 -> 50x50
    })).unwrap();

    const auto div1 = tree.new_with_children(
        taffy::StyleBuilder([](taffy::Style& s) {
            s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Percent(0.5f), taffy::Dimension::Percent(1.0f) };
            // s.justify_content = JustifyContent::Center;
        }),
        taffy::Vec<taffy::NodeId>{child_t1}
    ).unwrap();

    // right
    const auto child_t2 = tree.new_leaf(taffy::StyleBuilder([](taffy::Style& s) {
        s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Length(50.0f), taffy::Dimension::Length(50.0f) }; // Slightly enlarged size: 5x5 -> 50x50
    })).unwrap();

    const auto div2 = tree.new_with_children(
        taffy::StyleBuilder([](taffy::Style& s) {
            s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Percent(0.5f), taffy::Dimension::Percent(1.0f) };
            // s.justify_content = taffy::JustifyContent::Center;
        }),
        taffy::Vec<taffy::NodeId>{child_t2}
    ).unwrap();

    const auto container = tree.new_with_children(
        taffy::StyleBuilder([](taffy::Style& s) { s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Percent(1.0f), taffy::Dimension::Percent(1.0f) }; }),
        taffy::Vec<taffy::NodeId>{div1, div2}
    ).unwrap();

    return container;
}

// -----------------------------------------------------------------------------

// NOTE: currently unused
template <typename T>
inline void show_SlotMap(
    const taffy::SlotMap<T>& slot_map,
    void (*show_value_func)(const T& value)
)
{
    ImGui::PushID(&slot_map);
    {
        constexpr ImGuiTableFlags TABLE_FLAGS =
            ImGuiTableFlags_Resizable      |
            ImGuiTableFlags_BordersInnerV  |
          //ImGuiTableFlags_SizingFixedFit |
            ImGuiTableFlags_NoSavedSettings;
        constexpr char TABLE_ID[] = "taffy_slot_map_table";

        if( ImGui::BeginTable(TABLE_ID, 2, TABLE_FLAGS) )
        {
            // Table columns specification, needed to set 'fixed' & 'stretch' flags
            ImGui::TableSetupColumn("Key",   ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow(); // Show table header

            // -----------------------------------------------------------------

            slot_map.for_each([show_value_func](const taffy::DefaultKey& key, const T& value)
            {
                ImGui::PushID(&value);
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("(%llu, %llu)", key.first, key.second);

                    ImGui::TableNextColumn();
                    show_value_func(value);
                }
                ImGui::PopID();
            });

            ImGui::EndTable();
        }
    }
    ImGui::PopID();
}

// -----------------------------------------------------------------------------

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>

/*
    Usage:
        - Open in a new tab: open_link_by_window_open("https://example.com/", "_blank");
        - Open in this tab:  open_link_by_window_open("https://example.com/", "_self");
 */
static bool open_link_by_window_open(const char* url, const char* target)
{
    const int result = EM_ASM_INT({
        const url    = UTF8ToString($0);
        const target = UTF8ToString($1);

        var openedWin = window.open(url, target);

        // Return status, is window being opened. May be 'null', if it was
        // blocked  by "popup blocker"
        return (openedWin == null) ? 0 : 1;
    }, url, target);

    return (result != 0);
}

#endif // __EMSCRIPTEN__

bool open_link_in_new_tab(const char *url)
{
#if defined(__EMSCRIPTEN__)

    return open_link_by_window_open(url, "_blank");

#else // ! __EMSCRIPTEN__

    // TODO: add ability to open links in Linux/Windows/Mac

    (void)url; // Unused
    return false;

#endif
}

// -----------------------------------------------------------------------------

class MyApp : public ImGui_Application
{
    bool show_nodes_view { true };
    bool show_nodes_tree { true };
    bool show_node_style_properties { true };

    ImGui_Taffy::Node_Draw_Style draw_style {};

    std::unique_ptr<taffy::Taffy> tree;
    taffy::NodeId root_node { 0 };

    bool auto_update_layout { true };
    bool apply_size_constraints { true };

    taffy::Option<taffy::NodeId> focused_node;
    taffy::Style editing_style;

public:

    bool init() override
    {
        if( !ImGui_Application::init() )
        {
            return false;
        }

        set_window_title("Taffy_cpp Playground");

        tree = std::unique_ptr<taffy::Taffy>( new taffy::Taffy() );
        root_node = tree->new_leaf(taffy::StyleBuilder([](taffy::Style& s)
        {
            s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Percent(1.0f), taffy::Dimension::Percent(1.0f) };
        })).unwrap();

        for(int i = 0; i < 3; ++i)
        {
            const auto child = tree->new_leaf(taffy::StyleBuilder([](taffy::Style& s)
            {
                s.size = taffy::Size<taffy::Dimension> { taffy::Dimension::Length(100.0f), taffy::Dimension::Length(100.0f) };
            })).unwrap();

            tree->add_child(root_node, child).unwrap();
        }

        // Currently focused node is 'root node'
        focused_node = taffy::Option<taffy::NodeId>{root_node};

        // Show descriptions by default
        ImGui_Taffy::set_show_descriptions(true);

        // Set callback for opening links
        ImGui_Taffy::set_open_link_func([](const char* url) {
            open_link_in_new_tab(url);
        });

        // Default style is too dark (since designed to display over light
        // background) but here we use dark theme, so it's reasonable to tweak
        // non-focused color a bit, to be more visible.
        draw_style.node_nonfocused_color = IM_COL32(128, 128, 128, 128);

        return true;
    }

protected:

    void draw_ui() override
    {
        // Menu Bar
        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("Windows"))
            {
                ImGui::MenuItem("Show Taffy nodes view", nullptr, &show_nodes_view);
                ImGui::MenuItem("Show Taffy nodes tree", nullptr, &show_nodes_tree);
                ImGui::MenuItem("Show Taffy node properties", nullptr, &show_node_style_properties);

                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Display"))
            {
                bool show_descriptions = ImGui_Taffy::is_show_descriptions();
                if( ImGui::MenuItem("Show descriptions", nullptr, &show_descriptions) )
                {
                    ImGui_Taffy::set_show_descriptions(show_descriptions);
                }

                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Examples"))
            {
                if(ImGui::MenuItem("Basic"))
                {
                    root_node = set_example_basic(*tree.get());
                    focused_node = taffy::Option<taffy::NodeId>{root_node};
                }

                if(ImGui::MenuItem("Flexbox gap"))
                {
                    root_node = set_example_flexbox_gap(*tree.get());
                    focused_node = taffy::Option<taffy::NodeId>{root_node};
                }

                #if defined(TAFFY_FEATURE_GRID)
                if(ImGui::MenuItem("Holy grail"))
                {
                    root_node = set_example_holy_grail(*tree.get());
                    focused_node = taffy::Option<taffy::NodeId>{root_node};
                }
                #endif // TAFFY_FEATURE_GRID

                if(ImGui::MenuItem("Nested"))
                {
                    root_node = set_example_nested(*tree.get());
                    focused_node = taffy::Option<taffy::NodeId>{root_node};
                }

                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("About"))
            {
                ImGui::Text("Dear ImGui version: %s", IMGUI_VERSION);
                // ImGui::Text("Taffy version (commit): %s", TAFFY_VERSION_COMMIT_SHORT); // TODO: add somehow taffy version

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if(show_nodes_view)
        {
            set_next_window_initial_pos_and_size(15, 30, 525, 585);
            if( ImGui::Begin("Taffy nodes view", &show_nodes_view, ImGuiWindowFlags_MenuBar) )
            {
                if(ImGui::BeginMenuBar())
                {
                    if(ImGui::BeginMenu("Draw style"))
                    {
                        static const auto ColorEdit_ImU32 = [](const char* label, ImU32& color) -> bool
                        {
                            // NOTE: Due to copying float[4] <--> ImVec4 few times, this
                            // function not too optimal :/

                            const ImVec4 float_color = ImColor(color);
                            float float_array_color[4] {float_color.x, float_color.y, float_color.z, float_color.w };

                            const bool changed = ImGui::ColorEdit4(label, float_array_color);

                            if(changed)
                            {
                                color = ImColor(float_array_color[0], float_array_color[1], float_array_color[2], float_array_color[3]);
                            }

                            return changed;
                        };

                        ColorEdit_ImU32("Node non-focused color", draw_style.node_nonfocused_color);
                        ColorEdit_ImU32("Node focused color", draw_style.node_focused_color);

                        ImGui::DragFloat("Node non-focused thickness", &draw_style.node_nonfocused_thickness);
                        ImGui::DragFloat("Node focused thickness", &draw_style.node_focused_thickness);

                        ColorEdit_ImU32("Position offset color", draw_style.position_offset_color);
                        ColorEdit_ImU32("Margin color", draw_style.margin_color);
                        ColorEdit_ImU32("Border color", draw_style.border_color);
                        ColorEdit_ImU32("Padding color", draw_style.padding_color);

                        ColorEdit_ImU32("Values color", draw_style.values_color);

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenuBar();
                }

                if(true /*root_node*/) // TODO: check is root_node exists
                {
                    const ImVec2 content_max = ImGui::GetWindowContentRegionMax();
                    const ImVec2 content_min = ImGui::GetWindowContentRegionMin();

                    const ImVec2 content_size = {
                        (content_max.x - content_min.x),
                        (content_max.y - content_min.y) - 24 // Add space for buttons
                    };

                    draw_nodes(*tree.get(), root_node, focused_node, content_size, &draw_style);

                    // ---------------------------------------------------------

                    ImGui::Checkbox("Auto update layout", &auto_update_layout);

                    if(auto_update_layout)
                    {
                        const taffy::AvailableSpace available_space_x = apply_size_constraints ? taffy::AvailableSpace::Definite(content_size.x) : taffy::AvailableSpace::MaxContent();
                        const taffy::AvailableSpace available_space_y = apply_size_constraints ? taffy::AvailableSpace::Definite(content_size.y) : taffy::AvailableSpace::MaxContent();

                        tree->compute_layout(root_node, taffy::Size<taffy::AvailableSpace>{available_space_x, available_space_y}).unwrap();
                    }
                    else
                    {
                        ImGui::SameLine();

                        if( ImGui::Button("Calculate layout") )
                        {
                            const taffy::AvailableSpace available_space_x = apply_size_constraints ? taffy::AvailableSpace::Definite(content_size.x) : taffy::AvailableSpace::MaxContent();
                            const taffy::AvailableSpace available_space_y = apply_size_constraints ? taffy::AvailableSpace::Definite(content_size.y) : taffy::AvailableSpace::MaxContent();

                            tree->compute_layout(root_node, taffy::Size<taffy::AvailableSpace>{available_space_x, available_space_y}).unwrap();
                        }
                    }

                    ImGui::SameLine();

                    ImGui::Checkbox("Apply size constraints", &apply_size_constraints);
                }
            }
            ImGui::End();
        }

        if(show_nodes_tree)
        {
            set_next_window_initial_pos_and_size(550, 30, 290, 585);
            if( ImGui::Begin("Taffy nodes tree", &show_nodes_tree) )
            {
                draw_nodes_tree(tree.get(), root_node, root_node, focused_node);
            }
            ImGui::End();
        }

        if(show_node_style_properties)
        {
            set_next_window_initial_pos_and_size(850, 30, 400, 585);
            if( ImGui::Begin("Taffy node Style", &show_node_style_properties) )
            {
                if(focused_node.is_some())
                {
                    const taffy::Style& style = tree->style(focused_node.value()).unwrap().get();
                    editing_style = style;

                    if( ImGui_Taffy::edit_Style(editing_style) )
                    {
                        tree->set_style(focused_node.value(), editing_style).unwrap();
                    }
                }
            }
            ImGui::End();
        }

    }
};

int main()
{
    MyApp app;
    if(!app.init())
        return 1;

    app.run_main_loop();

    return 0;
}

