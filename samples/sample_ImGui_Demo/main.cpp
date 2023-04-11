#include <imgui_app/ImGui_Application.hpp>

#include "imgui.h"

class App : public ImGui_Application
{
public:

    bool init() override
    {
        if( !ImGui_Application::init() )
        {
            return false;
        }

        set_window_title("ImGui Demo");

        return true;
    }

protected:

    void draw_ui() override
    {
        ImGui::ShowDemoWindow();
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
