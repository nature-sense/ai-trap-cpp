#pragma once
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>

namespace io::naturesense {
    struct DetectionReference {

        DetectionReference(
            cv::Rect bounding_box,
            int clazz,
            float score
            ) : bounding_box(bounding_box),
                clazz(clazz),
                score(score),
                track_id(-1) {};

        void set_track_id(int track_id);

        cv::Rect bounding_box;
        int clazz;
        float score;
        int track_id;
    };
}