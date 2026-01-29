//
// Created by steve on 12/12/2025.
#pragma once
#include "image_reference.h"
#include "detection_reference.h"

namespace io::naturesense {
    struct CaptureReference {
        CaptureReference() : id(0), timestamp(0.0), hi_res_image(ImageReference()), lo_res_image(ImageReference()) {}
        CaptureReference(uint64_t id,
            uint64_t timestamp,
            ImageReference hi_res_image,
            ImageReference lo_res_image ) :
        id(id), timestamp(timestamp), hi_res_image(hi_res_image), lo_res_image(lo_res_image) {}
        const uint64_t id;
        const uint64_t timestamp;
        ImageReference hi_res_image;
        ImageReference lo_res_image;
        std::vector<DetectionReference> detections;
    };
}