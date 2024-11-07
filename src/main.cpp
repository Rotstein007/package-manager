#include <gtkmm.h>
#include <glibmm.h>
#include <iostream>
#include <fstream>
#include <sstream>

class PackageManagerApp : public Gtk::Application {
protected:
    PackageManagerApp() : Gtk::Application("dev.rotstein.packagemanager") {}

    void on_startup() override {
        Gtk::Application::on_startup();

        // Hauptfenster
        auto* window = new Gtk::Window();
        window->set_default_size(800, 600);
        window->set_title("Package Manager");

        auto* vbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
        window->set_child(*vbox);

        // Horizontale Box für Buttons
        auto* hbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
        vbox->append(*hbox);

        // Auswahl-Button für Paketmanager
        auto* button = Gtk::make_managed<Gtk::Button>("Select Package Manager");
        button->set_size_request(200, 50);
        button->set_margin_top(10);
        button->set_margin_start(10);
        button->signal_clicked().connect(sigc::mem_fun(*this, &PackageManagerApp::on_button_clicked));
        hbox->append(*button);

        // Terminal-Button zum Zeigen des Terminal-Bereichs
        auto* terminal_button = Gtk::make_managed<Gtk::Button>("Show Terminal");
        terminal_button->set_halign(Gtk::Align::CENTER);
        terminal_button->signal_clicked().connect(sigc::mem_fun(*this, &PackageManagerApp::on_terminal_button_clicked));
        vbox->append(*terminal_button);

        // Textbereich für Terminal-Ausgabe
        terminal_area = Gtk::make_managed<Gtk::TextView>();
        terminal_area->set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
        terminal_area->set_visible(false);
        vbox->append(*terminal_area);

        add_window(*window);
        window->show();
    }

    void on_activate() override {
        std::cout << "Willkommen zum Package Manager!" << std::endl;
        std::cout << "Das Programm läuft korrekt." << std::endl;
    }

    void on_button_clicked() {
        // Popup-Fenster für Paketmanager-Auswahl
        auto* popup = new Gtk::Window();
        popup->set_default_size(400, 300);
        popup->set_title("Select Package Manager");

        auto* vbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
        popup->set_child(*vbox);

        // Checkboxen für verschiedene Paketmanager
        auto* checkbox1 = Gtk::make_managed<Gtk::CheckButton>("paru");
        auto* checkbox2 = Gtk::make_managed<Gtk::CheckButton>("yay");
        auto* checkbox3 = Gtk::make_managed<Gtk::CheckButton>("flatpak");
        auto* checkbox4 = Gtk::make_managed<Gtk::CheckButton>("snap");
        vbox->append(*checkbox1);
        vbox->append(*checkbox2);
        vbox->append(*checkbox3);
        vbox->append(*checkbox4);

        load_state(checkbox1, checkbox2, checkbox3, checkbox4);

        auto* hbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
        vbox->append(*hbox);

        // Apply- und Cancel-Buttons für Auswahl
        auto* apply_button = Gtk::make_managed<Gtk::Button>("Apply");
        apply_button->get_style_context()->add_class("suggested-action");
        apply_button->signal_clicked().connect([this, checkbox1, checkbox2, checkbox3, checkbox4]() {
            save_state(checkbox1, checkbox2, checkbox3, checkbox4);
        });
        hbox->append(*apply_button);

        auto* cancel_button = Gtk::make_managed<Gtk::Button>("Cancel");
        cancel_button->signal_clicked().connect([popup]() {
            popup->close();
        });
        hbox->append(*cancel_button);

        popup->show();
    }

    void on_terminal_button_clicked() {
        // Terminalbereich sichtbar/unsichtbar schalten
        terminal_area->set_visible(!terminal_area->get_visible());
    }

    void run_terminal_command(const std::string& command) {
        // Führen Sie den Befehl aus und fangen Sie sowohl stdout als auch stderr ein
        Glib::RefPtr<Glib::IOChannel> stdout_channel, stderr_channel;

        // Starten Sie den Subprozess mit Umleitung der Ausgaben
        Glib::ChildProcess child_process(command, Glib::SpawnFlags::STDOUT_PIPE | Glib::SpawnFlags::STDERR_PIPE);
        pid_t pid = child_process.get_pid();

        // Holen Sie sich die Kanäle für stdout und stderr
        stdout_channel = Glib::IOChannel::create_from_fd(child_process.get_stdout_fd());
        stderr_channel = Glib::IOChannel::create_from_fd(child_process.get_stderr_fd());

        // Setze Watcher für die Kanäle, um den Output zu lesen
        stdout_channel->add_watch(Glib::IOCondition::READABLE, sigc::mem_fun(*this, &PackageManagerApp::on_subprocess_output));
        stderr_channel->add_watch(Glib::IOCondition::READABLE, sigc::mem_fun(*this, &PackageManagerApp::on_subprocess_output));

        child_process.spawn();
    }

    void on_subprocess_output(Glib::IOChannel* channel, Glib::IOCondition cond) {
        if (cond & Glib::IOCondition::READABLE) {
            std::string output;
            channel->read_line(output);
            terminal_area->get_buffer()->insert_at_cursor(output);
        }
    }

    void save_state(Gtk::CheckButton* checkbox1, Gtk::CheckButton* checkbox2, Gtk::CheckButton* checkbox3, Gtk::CheckButton* checkbox4) {
        std::ofstream file("state.txt");
        if (file.is_open()) {
            file << checkbox1->get_active() << std::endl;
            file << checkbox2->get_active() << std::endl;
            file << checkbox3->get_active() << std::endl;
            file << checkbox4->get_active() << std::endl;
            file.close();
        }
    }

    void load_state(Gtk::CheckButton* checkbox1, Gtk::CheckButton* checkbox2, Gtk::CheckButton* checkbox3, Gtk::CheckButton* checkbox4) {
        std::ifstream file("state.txt");
        if (file.is_open()) {
            bool state;
            file >> state;
            checkbox1->set_active(state);
            file >> state;
            checkbox2->set_active(state);
            file >> state;
            checkbox3->set_active(state);
            file >> state;
            checkbox4->set_active(state);
            file.close();
        }
    }

private:
    Gtk::TextView* terminal_area{};  // Terminal ersetzt durch TextView

public:
    static Glib::RefPtr<PackageManagerApp> create() {
        return Glib::RefPtr<PackageManagerApp>(new PackageManagerApp());
    }
};

int main(int argc, char* argv[]) {
    auto app = PackageManagerApp::create();
    return app->run(argc, argv);
}
