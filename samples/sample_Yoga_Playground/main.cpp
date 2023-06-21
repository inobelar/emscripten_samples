#include <imgui_app/ImGui_Application.hpp>

#include <yoga/Yoga.h>

#define YOGA_VERSION_COMMIT_FULL  "d06f7b989e02a61d02aa6456b0b61f65b13961d0"
#define YOGA_VERSION_COMMIT_SHORT "d06f7b9"

#include <ImGui_Yoga/enums.hpp>
#include <ImGui_Yoga/YGValue_widgets.hpp>
#include <ImGui_Yoga/YGNode_draw_recursive.hpp>
#include <ImGui_Yoga/YGNode_reflect_web.hpp>
#include <ImGui_Yoga/YGNode_reflect_full.hpp>


// User Reference-to-pointer, to properly change it in recursive function
// Reference: https://stackoverflow.com/questions/46469803/pointer-change-its-address-after-exiting-a-recursive-function
void draw_nodes_tree(YGNodeRef &node, YGNodeRef &focused_node)
{
    // If passed node is null - provide ability to add node
    if(node == nullptr)
    {
        if( ImGui::Button("Add node") )
        {
            node = YGNodeNew();
        }

        return;
    }

    const bool is_root = (YGNodeGetParent(node) == nullptr);

    ImGuiTreeNodeFlags tree_node_flags =
        ImGuiTreeNodeFlags_OpenOnArrow;

    // Highlight 'focused' node
    if(node == focused_node)
    {
        tree_node_flags |= ImGuiTreeNodeFlags_Selected;
    }

    // We need to combine the next actions for each node leaf:
    //   - setting node as 'focused' - action: 'click'
    //   - opening node              - action: click over 'arrow'

    const bool tree_opened = ImGui::TreeNodeEx(node, tree_node_flags, "Node (%p)", node);
    if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        focused_node = node;
    }

    if( tree_opened )
    {
        // Buttons
        {
            if( ImGui::Button("[+] Add child node") )
            {
                // On Yoga playground website, for newly added child, size is
                // set to 100x100 for convenience, so we copy that behaviour.
                YGNodeRef new_child = YGNodeNew();
                YGNodeStyleSetWidth (new_child, 100.0f);
                YGNodeStyleSetHeight(new_child, 100.0f);

                const uint32_t index = YGNodeGetChildCount(node);
                YGNodeInsertChild(node, new_child, index);
            }

            ImGui::SameLine();

            if( ImGui::Button("[x] Remove node") )
            {
                // Important: if node-to-delete is currently 'focused' node,
                // we need to 'clear focus' before deletion.
                if(focused_node == node)
                {
                    focused_node = nullptr;
                }

                // `YGNodeRemoveChild()` call before `YGNodeFree()` needed to
                // mark 'owner' (parent) layout as 'dirty' (needed to update)
                // before actually remove it's child node.
                if(YGNodeRef parent = YGNodeGetParent(node))
                {
                    YGNodeRemoveChild(parent, node);
                }

                // Remove node (and it's childs recursively)
                YGNodeFree(node);
                node = nullptr; // Important - mark it as deleted
            }
        }

        // Process childs
        if(node != nullptr) // Additional check for case, when node being removed by user
        {
            const uint32_t child_count = YGNodeGetChildCount(node);
            for(uint32_t child_idx = 0; child_idx < child_count; child_idx++)
            {
                YGNodeRef child_node = YGNodeGetChild(node, child_idx);

                draw_nodes_tree(child_node, focused_node);
            }
        }

        ImGui::TreePop();
    }
}

void traverse_nodes_at_pos(
        const ImVec2& origin,
        YGNodeRef node,
        const ImVec2& pos,
        std::vector<YGNodeRef>& nodes)
{
    if(node == nullptr)
    {
        return;
    }

    const float left   = YGNodeLayoutGetLeft(node);
    const float top    = YGNodeLayoutGetTop(node);
//    const float right  = YGNodeLayoutGetRight(node);
//    const float bottom = YGNodeLayoutGetBottom(node);

    const float width  = YGNodeLayoutGetWidth(node);
    const float height = YGNodeLayoutGetHeight(node);

    const ImVec2 node_rect__top_left  = {origin.x + left,         origin.y + top};
    const ImVec2 node_rect__bot_right = {origin.x + left + width, origin.y + top + height};

    if( !YGFloatIsUndefined(width) && !YGFloatIsUndefined(height) )
    {
        // Is 'node rect' contains 'pos'
        if( ((node_rect__top_left.x <= pos.x) && (node_rect__bot_right.x >= pos.x)) &&
            ((node_rect__top_left.y <= pos.y) && (node_rect__bot_right.y >= pos.y)) )
        {
            nodes.push_back(node);
        }
    }

    const uint32_t child_count = YGNodeGetChildCount(node);
    for(uint32_t child_idx = 0; child_idx < child_count; ++child_idx)
    {
        YGNodeRef child = YGNodeGetChild(node, child_idx);

        traverse_nodes_at_pos(
                    node_rect__top_left,
                    child,
                    pos,
                    nodes
        );
    }
}

std::vector<YGNodeRef> get_nodes_at_pos(const ImVec2& origin, YGNodeRef node, const ImVec2& pos)
{
    std::vector<YGNodeRef> nodes;
    traverse_nodes_at_pos(origin, node, pos, nodes);
    return nodes;
}

// TODO: const ref
bool draw_nodes(YGNodeRef node,
                YGNode* &focused_node,
                const ImVec2& rect_size,
                const ImGui_Yoga::YGNode_Draw_Style* draw_style)
{
    if(node == nullptr)
    {
        return false;
    }


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
        static std::vector<YGNodeRef> nodes_at_right_click_pos;

        if( (is_hovered && is_active) )
        {
            if( ImGui::IsMouseClicked(ImGuiMouseButton_Left) )
            {
                const ImVec2 mouse_pos = ImGui::GetMousePos();
                const ImVec2 mouse_pos_nodes_space = {mouse_pos.x - top_left.x, mouse_pos.y - top_left.y};

                 const auto nodes_at_left_click_pos =
                    get_nodes_at_pos(ImVec2(0, 0), node, mouse_pos_nodes_space);

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
                    get_nodes_at_pos(ImVec2(0, 0), node, mouse_pos_nodes_space);

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
            for(const YGNodeRef n : nodes_at_right_click_pos)
            {
                char text_buf[64];
                const int text_size = snprintf(text_buf, 64, "Node (%p)", n);

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
        ImGui_Yoga::YGNode_draw_recursive(node, draw_list, top_left, focused_node, draw_style);
    }
    draw_list->PopClipRect();

    return focused_node_changed;
}

void set_next_window_initial_pos_and_size(float pos_x, float pos_y, float size_x, float size_y)
{
    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to this application a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + pos_x, main_viewport->WorkPos.y + pos_y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(size_x, size_y), ImGuiCond_FirstUseEver);
}

class App : public ImGui_Application
{
    bool show_nodes_view { true };
    bool show_nodes_tree { true };
    bool show_node_properties { true };

    bool properties_show_full { false }; // 'false' means 'web' style

    ImGui_Yoga::YGNode_Draw_Style draw_style {};

    YGNodeRef root_node { nullptr };
    bool auto_update_layout { true };
    bool apply_size_constraints { true };

    YGNode* focused_node { nullptr }; // non-owning (weak) ptr

public:

    bool init() override
    {
        if( !ImGui_Application::init() )
        {
            return false;
        }

        set_window_title("Yoga Playground");


        YGConfigSetUseWebDefaults(YGConfigGetDefault(), true);

        root_node = YGNodeNew();
        {
            // Set 'Flex' params
            YGNodeStyleSetDirection(root_node, YGDirection::YGDirectionLTR);

            // Set 'Alignment' params
            YGNodeStyleSetAlignItems(root_node, YGAlign::YGAlignFlexStart);

            // Set 'Layout' params
            YGNodeStyleSetWidth (root_node, 500);
            YGNodeStyleSetHeight(root_node, 500);
            YGNodeStyleSetPadding(root_node, YGEdge::YGEdgeAll, 20);
        }

        {
            YGNodeRef node0 = YGNodeNew();
            YGNodeInsertChild(root_node, node0, YGNodeGetChildCount(root_node));

            // Set 'Flex' params
            YGNodeStyleSetDirection(node0, YGDirection::YGDirectionLTR);

            // Set 'Layout' params
            YGNodeStyleSetWidth (node0, 100);
            YGNodeStyleSetHeight(node0, 100);
        }

        {
            YGNodeRef node1 = YGNodeNew();
            YGNodeInsertChild(root_node, node1, YGNodeGetChildCount(root_node));

            // Set 'Flex' params
            YGNodeStyleSetDirection(node1, YGDirection::YGDirectionLTR);
            YGNodeStyleSetFlexGrow(node1, 1.0f);

            // Set 'Layout' params
            YGNodeStyleSetWidth (node1, 100);
            YGNodeStyleSetHeight(node1, 100);
            YGNodeStyleSetMargin(node1, YGEdge::YGEdgeLeft,  20);
            YGNodeStyleSetMargin(node1, YGEdge::YGEdgeRight, 20);
        }

        {
            YGNodeRef node2 = YGNodeNew();
            YGNodeInsertChild(root_node, node2, YGNodeGetChildCount(root_node));

            // Set 'Flex' params
            YGNodeStyleSetDirection(node2, YGDirection::YGDirectionLTR);

            // Set 'Layout' params
            YGNodeStyleSetWidth (node2, 100);
            YGNodeStyleSetHeight(node2, 100);
        }

        // Currently focused node is 'root node'
        focused_node = root_node;


        // Default style is too dark (since designed to display over light
        // background) but here we use dark theme, so it's reasonable to tweak
        // non-focused color a bit, to be more visible.
        draw_style.node_nonfocused_color = IM_COL32(128, 128, 128, 128);

        return true;
    }

    virtual ~App()
    {
        if(root_node != nullptr)
        {
            YGNodeFree(root_node);
            root_node = nullptr;
        }

        focused_node = nullptr;
    }

protected:

    void draw_ui() override
    {
        // Menu Bar
        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("Windows"))
            {
                ImGui::MenuItem("Show Yoga nodes view", nullptr, &show_nodes_view);
                ImGui::MenuItem("Show Yoga nodes tree", nullptr, &show_nodes_tree);
                ImGui::MenuItem("Show Yoga node properties", nullptr, &show_node_properties);

                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("About"))
            {
                ImGui::Text("Dear ImGui version: %s", IMGUI_VERSION);
                ImGui::Text("Yoga version (commit): %s", YOGA_VERSION_COMMIT_SHORT);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if(show_nodes_view)
        {
            set_next_window_initial_pos_and_size(15, 30, 525, 585);
            if( ImGui::Begin("Yoga nodes view", &show_nodes_view, ImGuiWindowFlags_MenuBar) )
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

                if(root_node != nullptr)
                {
                    const ImVec2 content_max = ImGui::GetWindowContentRegionMax();
                    const ImVec2 content_min = ImGui::GetWindowContentRegionMin();

                    const ImVec2 content_size = {
                        (content_max.x - content_min.x),
                        (content_max.y - content_min.y) - 24 // Add space for buttons
                    };

                    draw_nodes(root_node, focused_node, content_size, &draw_style);

                    // ---------------------------------------------------------

                    ImGui::Checkbox("Auto update layout", &auto_update_layout);

                    if(auto_update_layout)
                    {
                        const float available_space_x = apply_size_constraints ? content_size.x : YGUndefined;
                        const float available_space_y = apply_size_constraints ? content_size.y : YGUndefined;

                        YGNodeCalculateLayout(root_node, available_space_x, available_space_y, YGDirection::YGDirectionLTR);
                    }
                    else
                    {
                        ImGui::SameLine();

                        if( ImGui::Button("Calculate layout") )
                        {
                            const float available_space_x = apply_size_constraints ? content_size.x : YGUndefined;
                            const float available_space_y = apply_size_constraints ? content_size.y : YGUndefined;

                            YGNodeCalculateLayout(root_node, available_space_x, available_space_y, YGDirection::YGDirectionLTR);
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
            if( ImGui::Begin("Yoga nodes tree", &show_nodes_tree) )
            {
                draw_nodes_tree(root_node, focused_node);
            }
            ImGui::End();
        }

        if(show_node_properties)
        {
            set_next_window_initial_pos_and_size(850, 30, 400, 585);
            if( ImGui::Begin("Yoga node properties", &show_node_properties, ImGuiWindowFlags_MenuBar) )
            {
                if(ImGui::BeginMenuBar())
                {
                    if(ImGui::BeginMenu("Display"))
                    {
                        if( ImGui::RadioButton("Full", properties_show_full) )
                        {
                            properties_show_full = true;
                        }

                        ImGui::SameLine();

                        if( ImGui::RadioButton("Web", !properties_show_full) )
                        {
                            properties_show_full = false;
                        }

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenuBar();
                }

                if(properties_show_full)
                {
                    ImGui_Yoga::YGNode_reflect_full(focused_node);
                }
                else
                {
                    ImGui_Yoga::YGNode_reflect_web(focused_node);
                }
            }
            ImGui::End();
        }
    }
};

int main()
{
    App app;
    if(!app.init())
        return 1;

    app.run_main_loop();

    return 0;
}
