#include <iostream>
#include <string>
#include <gtkmm.h>

class PackageManagerApp : public Gtk::Application {
protected:
    PackageManagerApp() : Gtk::Application("dev.rotstein.packagemanager") {}

    void on_startup() override {
        Gtk::Application::on_startup();
        // GUI Beispiel: Ein einfaches Fenster öffnen
        auto* window = new Gtk::Window();
        window->set_default_size(800, 600);
        window->set_title("Package Manager");
        add_window(*window);
        window->show();
    }

    void on_activate() override {
        // Eine einfache Konsolenausgabe zu Beginn
        std::cout << "Willkommen zum Package Manager!" << std::endl;
        std::cout << "Das Programm läuft korrekt." << std::endl;
    }

public:
    static Glib::RefPtr<PackageManagerApp> create() {
        return Glib::RefPtr<PackageManagerApp>(new PackageManagerApp());
    }
};

int main() {
    auto app = PackageManagerApp::create();
    return app->run();
}