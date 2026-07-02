#pragma once

#include "redactly/Detector.hpp"
#include "redactly/FaceDetection.hpp"

#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>

#include <string>
#include <vector>

namespace redactly
{
    class PlateDetector final : public Detector
    {
    public:
        explicit PlateDetector(const std::string &modelPath);

        FaceDetections detect(const cv::Mat &bgrImage, float scoreThreshold, float nmsThreshold) override;

    private:
        int inputWidth_;
        int inputHeight_;
        Ort::Env env_;
        Ort::SessionOptions sessionOptions_;
        Ort::Session session_;
        std::vector<std::string> inputNames_;
        std::vector<std::string> outputNames_;
        std::vector<const char *> inputNamePtrs_;
        std::vector<const char *> outputNamePtrs_;
    };
}
