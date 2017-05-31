#ifndef APP_HPP
#define APP_HPP

#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/menubar.hpp>
#include "states/StatesManager.hpp"
#include "StreamManager.hpp"
#include "StationsManager.hpp"

class Application
{
public:
	Application();
private:
	void register_states();
	void init_menubar();
	nana::form window_;
	StatesManager states_manager_;
	nana::menubar menubar_;
	StreamManager stream_manager_;
	StationsManager stations_manager_;
};


#endif
