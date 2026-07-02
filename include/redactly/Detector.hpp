#pragma once

#include "redactly/FaceDetection.hpp"

#include <opencv2/core.hpp>

namespace redactly
{
    class Detector
    {
    public:
        virtual ~Detector() = default;

        virtual FaceDetections detect(const cv::Mat &bgrImage, float scoreThreshold, float nmsThreshold) = 0;
    };
}
