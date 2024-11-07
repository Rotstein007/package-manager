#include <iostream>
#include <fstream>
#include <string>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/spinner.h>
#include <gtkmm/image.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>
#include <gtkmm/overlay.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <glibmm/spawn.h>
#include <glibmm/main.h>
#include <glibmm/iochannel.h>

class PackageManagerApp : public Gtk::Application {
protected:
    PackageManagerApp() : Gtk::Application("dev.rotstein.packagemanager") {}

    void on_startup() override {
        Gtk::Application::on_startup();
        auto* window = new Gtk::Window();
        window->set_default_size(800, 600);
        window->set_title("Package Manager");

        auto* vbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
        window->set_child(*vbox);

        auto* hbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
        vbox->append(*hbox);

        auto* button = Gtk::make_managed<Gtk::Button>("Select Package Manager");
        button->set_size_request(200, 50);
        button->set_margin_top(10);
        button->set_margin_start(10);
        button->signal_clicked().connect(sigc::mem_fun(*this, &PackageManagerApp::on_button_clicked));
        hbox->append(*button);

        auto* check_button = Gtk::make_managed<Gtk::Button>();
        check_button->set_size_request(50, 50);
        check_button->set_margin_top(10);
        check_button->set_margin_start(10);
        auto* check_image = Gtk::make_managed<Gtk::Image>("checkmark.png");
        check_button->set_child(*check_image);
        check_button->get_style_context()->add_class("green-check");
        check_button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &PackageManagerApp::on_check_button_clicked), check_button));
        hbox->append(*check_button);

        auto* terminal_button = Gtk::make_managed<Gtk::Button>("Show Terminal");
        terminal_button->set_halign(Gtk::Align::CENTER);
        terminal_button->signal_clicked().connect(sigc::mem_fun(*this, &PackageManagerApp::on_terminal_button_clicked));
        vbox->append(*terminal_button);

        terminal_area = Gtk::make_managed<Gtk::ScrolledWindow>();
        terminal_area->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
        terminal_area->set_visible(false);
        vbox->append(*terminal_area);

        terminal_view = Gtk::make_managed<Gtk::TextView>();
        terminal_view->set_editable(false);
        terminal_view->set_wrap_mode(Gtk::WrapMode::WORD);
        terminal_area->set_child(*terminal_view);

        add_window(*window);
        window->show();
    }

    void on_activate() override {
        std::cout << "Willkommen zum Package Manager!" << std::endl;
        std::cout << "Das Programm läuft korrekt." << std::endl;
    }

    void on_button_clicked() {
        auto* popup = new Gtk::Window();
        popup->set_default_size(400, 300);
        popup->set_title("");

        auto* vbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);
        popup->set_child(*vbox);

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

    static void on_check_button_clicked(Gtk::Button* button) {
        button->set_sensitive(false);

        auto* spinner = Gtk::make_managed<Gtk::Spinner>();
        button->set_child(*spinner);
        spinner->start();

        Glib::RefPtr<Glib::IOChannel> io_channel;
        Glib::Pid pid;
        std::vector<std::string> argv = {"paru"};
        Glib::spawn_async_with_pipes(
            ".", argv, Glib::SPAWN_DO_NOT_REAP_CHILD, sigc::slot<void>(), &pid, nullptr, nullptr, nullptr, &io_channel);

        io_channel->set_flags(Glib::IOChannel::Flags::NONBLOCK);
        Glib::signal_io().connect(sigc::bind(sigc::mem_fun(*this, &PackageManagerApp::on_paru_output), button, spinner, pid), io_channel->get_fd(), Glib::IOCondition(Glib::IOCondition::IN | Glib::IOCondition::HUP));
    }

    bool on_paru_output(Glib::IOCondition condition, Gtk::Button* button, Gtk::Spinner* spinner, Glib::Pid pid) {
        if (static_cast<bool>(condition & Glib::IOCondition::HUP)) {
            spinner->stop();
            button->set_sensitive(true);
            auto* check_image = Gtk::make_managed<Gtk::Image>("checkmark.png");
            button->set_child(*check_image);
            Glib::spawn_close_pid(pid);
            return false;
        }

        if (static_cast<bool>(condition & Glib::IOCondition::IN)) {
            char buffer[256];
            gsize bytes_read;
            io_channel->read(buffer, sizeof(buffer) - 1, bytes_read);
            buffer[bytes_read] = '\0';
            terminal_view->get_buffer()->insert_at_cursor(buffer);
        }

        return true;
    }

    void on_terminal_button_clicked() {
        terminal_area->set_visible(!terminal_area->get_visible());
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
    Gtk::ScrolledWindow* terminal_area{};
    Gtk::TextView* terminal_view{};
    Glib::RefPtr<Glib::IOChannel> io_channel;

public:
    static Glib::RefPtr<PackageManagerApp> create() {
        return Glib::RefPtr<PackageManagerApp>(new PackageManagerApp());
    }
};

int main(int argc, char* argv[]) {
    auto app = PackageManagerApp::create();
    return app->run(argc, argv);
}