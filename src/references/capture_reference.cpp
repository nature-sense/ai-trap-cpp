#include "capture_reference.h"

namespace io::naturesense {

    uint64_t CaptureReference::get_id() const {
        return this->id;
    }
    uint64_t CaptureReference::get_timestamp() const {
        return this->timestamp;
    }

    ImageReference CaptureReference::get_hi_res_image() const {
        return this-> hi_res_image;
    }
    ImageReference CaptureReference::get_lo_res_image() const {
        return this-> lo_res_image;
    }

    std::vector<DetectionReference> CaptureReference::get_detections() const {
        return this->detections;
    }

    void CaptureReference::add_detection(DetectionReference detection) {
        this->detections.push_back(detection);
    }

}