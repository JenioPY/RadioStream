#include "../include/StationsDatabase.hpp"
#include "../include/Station.hpp"
#include "../include/Utilities.hpp"

StationsDatabase::StationsDatabase(const std::string& database_name)
    : database_("SQLite", database_name)
{
    Poco::Data::Statement select(database_);
    Station station;
    int id;
    select << "SELECT * FROM stations",
        Poco::Data::Keywords::into(id),
        Poco::Data::Keywords::into(station.name_),
        Poco::Data::Keywords::into(station.ip_),
        Poco::Data::Keywords::into(station.favorite_),
        Poco::Data::Keywords::range(0, 1);

    while(!select.done())
    {
        select.execute();
        cached_stations_.push_back(station);
    }
}

const std::vector<Station>& StationsDatabase::get_stations() const
{
    return cached_stations_;
}

void StationsDatabase::add_station(const Station& station)
{
    Poco::Data::Statement insert(database_);
    const auto favorite = static_cast<int>(station.favorite_);
    insert << "INSERT INTO stations (name, ip, favorite) VALUES(?, ?, ?)",
        Poco::Data::Keywords::bind(station.name_),
        Poco::Data::Keywords::bind(station.ip_),
        Poco::Data::Keywords::bind(favorite);
    insert.execute();
    cached_stations_.push_back(station);
}

void StationsDatabase::remove_station(const Station& station)
{
    Poco::Data::Statement delete_statement(database_);
    delete_statement << "DELETE FROM stations WHERE name = ? and ip = ?",
        Poco::Data::Keywords::bind(station.name_),
        Poco::Data::Keywords::bind(station.ip_);
    delete_statement.execute();
    cached_stations_.erase(std::find(cached_stations_.begin(), cached_stations_.end(), station));
}

void StationsDatabase::change_station_favorite_status(const Station& station)
{
    Poco::Data::Statement update(database_);
    update << "UPDATE stations SET favorite = ? WHERE name = ? AND ip = ?",
        Poco::Data::Keywords::bind(!station.favorite_),
        Poco::Data::Keywords::bind(station.name_),
        Poco::Data::Keywords::bind(station.ip_);
    update.execute();
    const auto station_found = std::find(cached_stations_.begin(), cached_stations_.end(), station);
    station_found->favorite_ = !station_found->favorite_;
}

std::vector<std::string> StationsDatabase::get_stations_names_with_substring(const std::string& substring) const
{
	std::vector<std::string> matching_station_names;
    const std::string string_to_look_for = string_to_lower(substring);
	for(const auto& station : cached_stations_)
	{
        std::string lower_cased_station_name = string_to_lower(station.name_);
		if (lower_cased_station_name.find(string_to_look_for) != std::string::npos)
		{
            matching_station_names.push_back(lower_cased_station_name);
		}
	}
	return matching_station_names;
}

std::string StationsDatabase::get_station_ip(const std::string& station_name) const
{
    const auto iterator = std::find_if(cached_stations_.cbegin(), cached_stations_.cend(), [&](const Station& station)
    {
        return station.name_ == station_name;
    });
    if(iterator == cached_stations_.cend())
    {
        throw;
    }
    return iterator->ip_;
}