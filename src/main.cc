#include <gtkmm.h>
#include "core/config.h"
#include "gui/window.h"

int main(int argc, char **argv)
{
    core::Config::load();

    auto app = Gtk::Application::create("org.hander.cppcord");
    auto win = new gui::Window();

    app->register_application();
    app->signal_activate().connect([&]
    { 
        win->show();
    });

    app->add_window(*win);
    return app->run(argc, argv);
}
