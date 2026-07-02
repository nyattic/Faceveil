#pragma once

#include <opencv2/core.hpp>

#include <vector>

namespace redactly
{
    struct FaceDetection
    {
        cv::Rect2f box;
        float score = 0.0F;
    };

    using FaceDetections = std::vector<FaceDetection>;
}
