//
// Created by steve on 12/12/2025.
#pragma once
#include "image_reference.hx"

namespace io::naturesense {
    struct CaptureReference {
        uint64_t id;
        uint64_t timestamp;
        ImageReference hi_res_image;
        ImageReference lo_res_image;

        CaptureReference() : id(0), timestamp(0.0), hi_res_image(ImageReference()), lo_res_image(ImageReference()) {}
        CaptureReference(int64_t id,
            uint64_t timestamp,
            ImageReference hi_res_image,
            ImageReference lo_res_image ) :
        id(id), timestamp(timestamp), hi_res_image(hi_res_image), lo_res_image(lo_res_image) {}
    };
}