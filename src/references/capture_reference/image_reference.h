#pragma once
#include <libcamera/libcamera.h>

using namespace libcamera;

namespace io::naturesense {
    struct ImageReference {
        ImageReference();
        ImageReference(const FrameBuffer::Plane &plane);
        [[nodiscard]] std::pair<uint8_t*, uint32_t> map_data() const;
        void unmap_data(uint8_t* data, uint32_t len);
        void trace() const;

        int dma_buf_fd;
        unsigned int length;
        unsigned int offset;
    };

}
