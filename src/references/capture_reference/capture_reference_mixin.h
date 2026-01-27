#pragma once

#include <boost/asio.hpp>
#include "capture_reference.h"
namespace asio = boost::asio;

namespace io::naturesense {
    class CaptureReferenceMixin {
    public:
        CaptureReferenceMixin() = default;
        virtual ~CaptureReferenceMixin(){};
        virtual asio::awaitable<void> post_async(CaptureReference ref) = 0;
        virtual void post_sync(CaptureReference ref) = 0;
    };
}
