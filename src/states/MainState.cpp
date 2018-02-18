#include "../../include/states/MainState.hpp"
#include "../../include/observers/StreamObserver.hpp"
#include "../../include/observers/StationsDatabaseObserver.hpp"
#include "../../include/observers/StatusObserver.hpp"
#include "../../include/Constants.hpp"
#include "../../include/Utilities.hpp"
#include <nana/gui/widgets/menubar.hpp>

using namespace constants;

MainState::MainState(StatesManager& manager, Context& context)
	: State(manager, context)
	, container_(context.window)
	, current_song_label_(context.window, context.localizer.get_localized_text("no song is playing"))
	, current_station_label_(context.window, context.localizer.get_localized_text("no station is playing"))
	, play_button_(context.window)
	, pause_button_(context.window)
	, mute_button_(context.window)
	, search_textbox_(context.window)
	, stations_listbox_(context.window)
	, volume_slider_(context.window)
    , song_label_menu_()
    , listbox_item_menu_()
{
    add_observers();
	build_interface();
    init_contextual_menus();
	init_listbox();
    run_concurrent_song_name_updater();
}

void MainState::change_visibility(bool visible)
{
	container_.field_display("content", visible);
	context_.menubar.show();
}

void MainState::set_station_name(const std::string& name)
{
    current_station_label_.caption(name);
}

void MainState::refresh_listbox()
{
    populate_listbox();
}

void MainState::select_row_without_unselect_feature(const nana::arg_listbox& selected_row)
{
    auto& arg_item = selected_row.item;
    if (stations_listbox_.selected().empty())
    {
        arg_item.select(true);
    }
}

bool MainState::check_if_row_was_right_clicked(const nana::arg_mouse& arg) const
{
    return !arg.is_left_button()
        && !stations_listbox_.selected().empty()
        && !stations_listbox_.cast(arg.pos).empty()
        && !stations_listbox_.cast(arg.pos).is_category();
}

void MainState::add_observers()
{
    subject_.attach(std::make_unique<StreamObserver>());
    subject_.attach(std::make_unique<StationsDatabaseObserver>());
    subject_.attach(std::make_unique<StatusObserver>());
}

void MainState::run_concurrent_song_name_updater()
{
    song_title_updater_ = std::thread(&MainState::update_titles, this);
    song_title_updater_.detach();
}

void MainState::build_interface()
{
    current_song_label_.events().mouse_up([this](const nana::arg_mouse& arg)
    {
        if (!arg.is_left_button())
        {
            pop_song_title_menu();
        }
    });
	play_button_.caption(context_.localizer.get_localized_text("Play"));
    play_button_.events().click([this]()
    {
        subject_.notify(std::make_any<bool>(true), context_, events::Event::StreamPlay);
        subject_.notify(Observer::placeholder, context_, events::Event::StreamPlayingStatus);
    });
	pause_button_.caption(context_.localizer.get_localized_text("Pause"));
	pause_button_.events().click([this]()
	{
		subject_.notify(std::make_any<bool>(true), context_, events::Event::StreamPause);
        subject_.notify(Observer::placeholder, context_, events::Event::StreamPausedStatus);
	});
	mute_button_.caption(context_.localizer.get_localized_text("Mute"));
    mute_button_.enable_pushed(true);
    mute_button_.events().mouse_up([this]()
    {
        if(mute_button_.pushed())
        {
            subject_.notify(std::make_any<unsigned int>(volume_slider_.value()), context_, events::Event::StreamMute);
        }
        else
        {
            subject_.notify(std::make_any<unsigned int>(volume_slider_.value()), context_, events::Event::StreamVolumeChanged);
        }

    });
	volume_slider_.scheme().color_vernier = VERNIER_COLOR;
	volume_slider_.maximum(100);
	volume_slider_.value(volume_float_to_int(context_.stream_manager.get_current_volume()));
	volume_slider_.vernier([](unsigned int maximum, unsigned int cursor_value)
	{
		return std::string(std::to_string(cursor_value) + "/" + std::to_string(maximum));
	});
	volume_slider_.events().value_changed([this]()
	{
        if (!mute_button_.pushed())
        {
            subject_.notify(std::make_any<unsigned int>(volume_slider_.value()), context_, events::Event::StreamVolumeChanged);
        }    
	});
	search_textbox_.line_wrapped(true).multi_lines(false).tip_string("Search...");
	search_textbox_.events().text_changed([this]()
	{
        search_stations();
	});
	container_.div(
		"<content vertical margin=[5%,0,0,0]" 
			"<buttons weight=12% arrange=[10%,10%,10%,10%] gap=1% margin=1%>"
			"<labels weight=10% arrange=[49%,48%] gap=1% margin=1% >"
			"<misc weight=8% arrange=[25%,72%] gap=1% margin=1%>"
			"<listbox margin=[1%,1%,7%,1%]>"
		">");
	container_.field("buttons") << play_button_ << pause_button_ << mute_button_ ;
	container_.field("labels") << current_station_label_ << current_song_label_;
	container_.field("misc") << search_textbox_ << volume_slider_;
	container_.field("listbox") << stations_listbox_;
	container_.collocate();
}

void MainState::init_contextual_menus()
{
    song_label_menu_.append(context_.localizer.get_localized_text("Copy title to clipboard."), [this](auto&)
    {
        std::string title = current_song_label_.caption();
        copy_to_clipboard(title);
    });
    listbox_item_menu_.append(context_.localizer.get_localized_text("Play"), [this](auto&)
    {
        set_new_stream();
    });
    listbox_item_menu_.append(context_.localizer.get_localized_text("Subscribe"), [this](auto&)
    {
        subscribe_to_station();
    });
    listbox_item_menu_.append(context_.localizer.get_localized_text("Delete from list"), [this](auto&)
    {
        delete_station();
    });
}

void MainState::init_listbox()
{
//	stations_listbox_.append(context_.localizer.get_localized_text("User stations"));
//	stations_listbox_.append(context_.localizer.get_localized_text("Default stations"));
	stations_listbox_.append_header(context_.localizer.get_localized_text("Station's name"));
    stations_listbox_.append_header(context_.localizer.get_localized_text("Ip"));
	stations_listbox_.append_header(context_.localizer.get_localized_text("Favorite"));
	stations_listbox_.append_header(context_.localizer.get_localized_text("User defined"));
	stations_listbox_.column_at(static_cast<std::size_t>(StationListboxColumns::Name)).width(300u);
    stations_listbox_.column_at(static_cast<std::size_t>(StationListboxColumns::Ip)).width(200u);
	stations_listbox_.column_at(static_cast<std::size_t>(StationListboxColumns::Favorite)).width(100u);
//	stations_listbox_.column_at(static_cast<std::size_t>(StationListboxColumns::UserDefined)).width(0u); // 0u to make this column invisibe to the user and so that we can check easly whether it is user defined etc.
	stations_listbox_.enable_single(true, false);
    populate_listbox();
	stations_listbox_.sort_col(static_cast<std::size_t>(StationListboxColumns::Favorite), true);
   
    stations_listbox_.events().selected([this](const nana::arg_listbox& arg)
    {
        select_row_without_unselect_feature(arg);
    });
    stations_listbox_.events().mouse_down([this](const nana::arg_mouse& arg)
    {

        if (check_if_row_was_right_clicked(arg))
        {
            pop_stations_listbox_menu();
        }
    });
	stations_listbox_.events().dbl_click([this](const nana::arg_mouse& arg)
	{
		if(!stations_listbox_.cast(arg.pos).is_category() && arg.is_left_button()) // this condition must be fulfilled because when we click category it selects the last item in it so when we dbl_click category it works just as we would click last item in it
		{
            set_new_stream();
            update_station_label();
            update_song_label();
		}
	});
    stations_listbox_.auto_draw(true);
}

void MainState::update_titles()
{
    update_song_label();
	while(true)
	{
		std::this_thread::sleep_for(TIME_TO_CHECK_IF_SONG_CHANGED);
		std::lock_guard<std::mutex>{song_title_mutex_};
        update_song_label();
	}
}

void MainState::update_song_label()
{
    current_song_label_.caption(context_.stream_manager.get_song_title());
}

void MainState::update_station_label()
{
    if (!stations_listbox_.selected().empty())
    {
        const auto selected_item = stations_listbox_.selected().front();
            const auto category_index = static_cast<std::size_t>(StationListboxCategories::NanaDefault);
            const auto column_index = static_cast<std::size_t>(StationListboxColumns::Name);
            const auto station_category = stations_listbox_.at(category_index);
            const auto station_name = station_category.at(selected_item.item).text(column_index);
            current_station_label_.caption(station_name);
    }
}

/**
 * \brief Changes favorite value in both listbox and StationsManager::stations_.
 */
void MainState::subscribe_to_station()
{
    const auto selected_item = stations_listbox_.selected().front();
    const auto category_index = static_cast<std::size_t>(StationListboxCategories::NanaDefault);
    const auto name_column_index = static_cast<std::size_t>(StationListboxColumns::Name);
    const auto ip_column_index = static_cast<std::size_t>(StationListboxColumns::Ip);
    const auto favorite_column_index = static_cast<std::size_t>(StationListboxColumns::Favorite);
    const auto station_category = stations_listbox_.at(category_index);
    const auto station_name = station_category.at(selected_item.item).text(name_column_index);
    const auto station_ip = station_category.at(selected_item.item).text(ip_column_index);
    const auto station_favorite = str_to_bool(station_category.at(selected_item.item).text(favorite_column_index));
    const auto station = Station( station_name, station_ip, station_favorite);
    context_.stations_database.change_station_favorite_status(station);
    populate_listbox();
}

void MainState::populate_listbox()
{
    stations_listbox_.auto_draw(false);
    stations_listbox_.clear();
    for (const auto& station : context_.stations_database.get_stations())
    {
        const auto category_index = static_cast<std::size_t>(StationListboxCategories::NanaDefault);
        stations_listbox_.at(category_index).append(station);
    }
    stations_listbox_.sort_col(static_cast<std::size_t>(StationListboxColumns::Favorite), true);
    stations_listbox_.auto_draw(true);
}

void MainState::search_stations()
{
    subject_.notify(Observer::placeholder, context_, events::Event::SearchingStationsStatus);
    std::string string_to_find{};
    search_textbox_.getline(0, string_to_find);
    stations_listbox_.auto_draw(false);
    if (!string_to_find.empty())
    {
        stations_listbox_.clear();
        auto station_names_containing_substring = context_.stations_database.get_stations_names_with_substring(string_to_find);
        const auto& all_stations = context_.stations_database.get_stations();
        for (const auto& station : all_stations)
        {
            for (const auto& name : station_names_containing_substring)
            {
                if (string_to_lower(station.name_) == name)
                {
                    const auto category_index = static_cast<std::size_t>(StationListboxCategories::NanaDefault);
                    stations_listbox_.at(category_index).append(station);
                }
            }
        }
    }
    else
    {
        stations_listbox_.clear();
        populate_listbox();
    }
    stations_listbox_.auto_draw(true);
    subject_.notify(Observer::placeholder, context_, events::Event::NormalStatus);
}

void MainState::pop_song_title_menu()
{
    auto position = nana::API::cursor_position();
    nana::API::calc_window_point(context_.window, position);
    song_label_menu_.popup(context_.window, position.x, position.y);
}

void MainState::pop_stations_listbox_menu()
{
    auto position = nana::API::cursor_position();
    nana::API::calc_window_point(context_.window, position);
    listbox_item_menu_.popup(context_.window, position.x, position.y);
}

void MainState::set_new_stream()
{
    
    std::thread thread{[&]()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        subject_.notify(Observer::placeholder, context_, events::Event::LoadingStreamStatus);
        if (!stations_listbox_.selected().empty())
        {
            auto selected_item = stations_listbox_.selected().front();
            std::string station_name;
            auto category_index = static_cast<std::size_t>(StationListboxCategories::NanaDefault);
            auto column_index = static_cast<std::size_t>(StationListboxColumns::Name);
            auto station_category = stations_listbox_.at(category_index);
            station_name = station_category.at(selected_item.item).text(column_index);
            subject_.notify(station_name, context_, events::Event::StreamSetNew);
            subject_.notify(Observer::placeholder, context_, events::Event::StreamPlayingStatus);
        }
    } };
    thread.detach();
}

void MainState::delete_station()
{
    Station station{};
    const auto index = stations_listbox_.selected().front();
    stations_listbox_.at(index.cat).at(index.item).resolve_to(station);
    subject_.notify(std::make_any<Station>(station), context_, events::Event::DeleteStation);
    populate_listbox();
}

Station MainState::get_station_from_listbox(unsigned long long category_index, unsigned long long row_index) const
{
        const auto name_column_index = static_cast<std::size_t>(StationListboxColumns::Name);
        const auto ip_column_index = static_cast<std::size_t>(StationListboxColumns::Ip);
        const auto favorite_column_index = static_cast<std::size_t>(StationListboxColumns::Favorite);
        const auto station_category = stations_listbox_.at(category_index);
        const auto station_name = station_category.at(row_index).text(name_column_index);
        const auto station_ip = station_category.at(row_index).text(ip_column_index);
        const auto station_favorite = str_to_bool(station_category.at(row_index).text(favorite_column_index));
        return Station{station_name, station_ip, station_favorite};
}
