#pragma once

#include <boost/asio.hpp>
#include "recycle_reference.h"

namespace asio = boost::asio;
namespace io::naturesense {
    class RecycleReferenceMixin {
    public:
        RecycleReferenceMixin() = default;
        virtual ~RecycleReferenceMixin() {};
        virtual asio::awaitable<void> post_async(RecycleReference ref) = 0;
        virtual void post_sync(RecycleReference ref) = 0;
    };
}
