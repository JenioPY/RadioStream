#ifndef EVENTS_HPP
#define EVENTS_HPP

namespace events
{
	enum class Event
	{
        //StreamObserver events
		StreamPause,
		StreamPlay,
		StreamSetNew,
		StreamMute,
		StreamVolumeChanged,
        //StatusObserver events
        LoadingStreamStatus,
        NormalStatus,
        SearchingStationsStatus,
        StreamPlayingStatus,
        StreamPausedStatus,
        //StationsDatabaseObserver events
        AddStation,
        DeleteStation,
		//TextLoxccalizerObserver events
		ChangeLanguage
	};
}

#endif