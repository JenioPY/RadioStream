#include "../../include/controllers/StationsDatabaseController.hpp"

StationsDatabaseController::StationsDatabaseController(StatesManager& manager, State::Context context)
    : Controller(manager, context)
{
}

void StationsDatabaseController::process_event_command(const radiostream::Event e, std::any data)
{
    switch (e)
    {
    case radiostream::Event::AddStationToDatabase:
    {
        const auto station = std::any_cast<Station>(data);
        context_.stations_database.add_station(station);
    }
    break;

    case radiostream::Event::DeleteStationFromDatabase:
    {
        const auto station = std::any_cast<Station>(data);
        context_.stations_database.remove_station(station);
    }
    break;

    }
}