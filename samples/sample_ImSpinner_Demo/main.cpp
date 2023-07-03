#include <imgui_app/ImGui_Application.hpp>

#include <cctype> // Needed, since in ImSpinner used 'isalpha', but related file not included. TODO: Must be fixed later

#define IMSPINNER_DEMO
#include <imspinner.h>

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
public:

    bool init() override
    {
        if( !ImGui_Application::init() )
        {
            return false;
        }

        set_window_title("ImSpinner Demo");

        return true;
    }

protected:

    void draw_ui() override
    {
        set_next_window_initial_pos_and_size(10, 10, 1000, 585);
        if( ImGui::Begin("ImSpinner Demo") )
        {
            ImSpinner::demoSpinners();
        }
        ImGui::End();
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
