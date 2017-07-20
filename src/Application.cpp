#include "../include/Application.hpp"
#include "../include/observers/StationsObserver.hpp"

Application::Application()
	: window_(nana::API::make_center(600, 500), nana::appear::decorate<nana::appear::minimize, nana::appear::sizable, nana::appear::maximize, nana::appear::taskbar>())
	, menubar_(window_)
	, stream_manager_()
	, stations_manager_()
    , states_manager_(State::Context{ window_, menubar_, stream_manager_, stations_manager_ })
{
    window_.caption("Radio Client");
	init_menubar();
	register_states();
	states_manager_.switch_state(States::ID::Main);
	window_.show();
}

void Application::register_states()
{
	states_manager_.register_state<MainState>(States::ID::Main);
}

void Application::init_menubar()
{
	enum
	{
		FILE,
		SETTINGS
	};
	menubar_.push_back("File: ");
	menubar_.push_back("Settings: ");
	menubar_.at(FILE).append("Open URL", [this](nana::menu::item_proxy&)
	{
		nana::inputbox::text url("URL");
		nana::inputbox inbox(window_, "Please write correct URL.\n", "Open URL");
		if (inbox.show(url))
		{
			stream_manager_.set_new_stream(url.value());
			stream_manager_.play();
		}
	});
    menubar_.at(FILE).append("Add Station", [this](nana::menu::item_proxy&)
    {   
        nana::inputbox::text station_name("Station name:");
        nana::inputbox::text url("URL:");	
        nana::inputbox inbox(window_, "Please write correct URL.", "Add station");
        if (inbox.show(station_name, url))
        {
            stations_manager_.add_station(Station{ station_name.value(), url.value(), false, true });
        }
    });
	menubar_.hide();
}