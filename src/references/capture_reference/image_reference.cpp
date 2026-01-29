//
// Created by steve on 12/12/2025.
//

#include "image_reference.h"
#include <sys/mman.h>
#include <iostream>

using namespace libcamera;

namespace io::naturesense {

    ImageReference::ImageReference() {
        dma_buf_fd = 0;
        length = 0;
        offset = 0;
    }
    ImageReference::ImageReference(const FrameBuffer::Plane &plane) {
        dma_buf_fd = plane.fd.get();
        length = plane.length;
        offset = plane.offset;
    }

    std::pair<uint8_t*, uint32_t> ImageReference::map_data() const {
        auto *data = static_cast<uint8_t*>(mmap(
            nullptr,
            length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            dma_buf_fd,
            offset
            ));
        return std::make_pair(data, length);
    }

    void ImageReference::unmap_data(uint8_t* data, uint32_t len) {
        munmap(data, len);
    }

    void ImageReference::trace() const {
        std::cout << "DMA id : " << dma_buf_fd << " length : " << length << " offset : " << offset <<std::endl;
    }


}
