#include <imgui_app/ImGui_Application.hpp>

#include "implot.h"

class App : public ImGui_Application
{
public:

    bool init() override
    {
        if( !ImGui_Application::init() )
        {
            return false;
        }

        set_window_title("ImPlot Demo");

        ImPlot::CreateContext();

        return true;
    }

    virtual ~App()
    {
        ImPlot::DestroyContext();
    }

protected:

    void draw_ui() override
    {
        ImPlot::ShowDemoWindow();
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
