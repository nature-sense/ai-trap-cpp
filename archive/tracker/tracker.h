#pragma once
#include <boost/asio/awaitable.hpp>
#include "../references/capture_reference/capture_reference.h"
#include "channels/channel.h"
namespace io::naturesense {
    class Tracker {

    public:
        static asio::awaitable<void> actor(
            Channel<CaptureReference> *tracking_in_chan,
            Channel<CaptureReference> *extractor_in_chan
        );

    private:
        Tracker(
            Channel<CaptureReference> *tracking_in_chan,
            Channel<CaptureReference> *extractor_in_chan
            );
    };
}
