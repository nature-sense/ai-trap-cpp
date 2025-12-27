#pragma once
#include <libcamera/libcamera.h>

using namespace libcamera;

namespace io::naturesense {
    class ImageReference {
    public:
        ImageReference() : dma_buf_fd(0), length(0), offset(0) {};
        ImageReference(const FrameBuffer::Plane &plane);
        [[nodiscard]]  std::pair<uint8_t*, uint32_t> map_data() const;
        static void unmap_data(uint8_t* data, uint32_t length);
        void trace() const;
    private:
        int dma_buf_fd;
        uint32_t length;
        uint32_t offset;
    };

}

