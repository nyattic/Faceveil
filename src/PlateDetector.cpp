#include "redactly/PlateDetector.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace redactly
{
    namespace
    {
        constexpr int kChannels = 3;

        std::filesystem::path modelPathFromUtf8(const std::string &modelPath)
        {
            const std::u8string u8(modelPath.begin(), modelPath.end());
            return std::filesystem::path(u8);
        }
    }

    PlateDetector::PlateDetector(const std::string &modelPath)
        : inputWidth_(512),
          inputHeight_(512),
          env_(ORT_LOGGING_LEVEL_WARNING, "Redactly-Plate"),
          sessionOptions_(),
          session_(nullptr)
    {
        sessionOptions_.SetIntraOpNumThreads(1);
        sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        const std::filesystem::path modelFsPath = modelPathFromUtf8(modelPath);
        session_ = Ort::Session(env_, modelFsPath.c_str(), sessionOptions_);

        Ort::AllocatorWithDefaultOptions allocator;

        if (session_.GetInputCount() != 1 || session_.GetOutputCount() < 1)
        {
            throw std::runtime_error("The selected model does not look like a license-plate ONNX model.");
        }

        inputNames_.reserve(session_.GetInputCount());
        for (size_t i = 0; i < session_.GetInputCount(); ++i)
        {
            auto name = session_.GetInputNameAllocated(i, allocator);
            inputNames_.emplace_back(name.get());
        }
        outputNames_.reserve(session_.GetOutputCount());
        for (size_t i = 0; i < session_.GetOutputCount(); ++i)
        {
            auto name = session_.GetOutputNameAllocated(i, allocator);
            outputNames_.emplace_back(name.get());
        }

        const auto inputShape = session_.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
        if (inputShape.size() == 4)
        {
            if (inputShape[2] > 0)
            {
                inputHeight_ = static_cast<int>(inputShape[2]);
            }
            if (inputShape[3] > 0)
            {
                inputWidth_ = static_cast<int>(inputShape[3]);
            }
        }

        inputNamePtrs_.reserve(inputNames_.size());
        for (const auto &name: inputNames_)
        {
            inputNamePtrs_.push_back(name.c_str());
        }
        outputNamePtrs_.reserve(outputNames_.size());
        for (const auto &name: outputNames_)
        {
            outputNamePtrs_.push_back(name.c_str());
        }
    }

    FaceDetections PlateDetector::detect(const cv::Mat &bgrImage, float scoreThreshold, float /*nmsThreshold*/)
    {
        if (bgrImage.empty())
        {
            return {};
        }

        const float ratio = std::min(static_cast<float>(inputWidth_) / static_cast<float>(bgrImage.cols),
                                     static_cast<float>(inputHeight_) / static_cast<float>(bgrImage.rows));
        const int resizedWidth = std::max(1, static_cast<int>(std::round(bgrImage.cols * ratio)));
        const int resizedHeight = std::max(1, static_cast<int>(std::round(bgrImage.rows * ratio)));
        const float padX = (inputWidth_ - resizedWidth) / 2.0F;
        const float padY = (inputHeight_ - resizedHeight) / 2.0F;

        cv::Mat resized;
        cv::resize(bgrImage, resized, cv::Size(resizedWidth, resizedHeight), 0.0, 0.0, cv::INTER_LINEAR);

        cv::Mat canvas(inputHeight_, inputWidth_, CV_8UC3, cv::Scalar(114, 114, 114));
        const int top = static_cast<int>(std::round(padY - 0.1F));
        const int left = static_cast<int>(std::round(padX - 0.1F));
        resized.copyTo(canvas(cv::Rect(left, top, resized.cols, resized.rows)));

        std::vector<float> tensor(static_cast<size_t>(kChannels) * inputHeight_ * inputWidth_);
        const int planeSize = inputWidth_ * inputHeight_;
        for (int y = 0; y < inputHeight_; ++y)
        {
            const auto *row = canvas.ptr<cv::Vec3b>(y);
            for (int x = 0; x < inputWidth_; ++x)
            {
                const int offset = y * inputWidth_ + x;
                tensor[offset] = static_cast<float>(row[x][2]) / 255.0F;
                tensor[planeSize + offset] = static_cast<float>(row[x][1]) / 255.0F;
                tensor[2 * planeSize + offset] = static_cast<float>(row[x][0]) / 255.0F;
            }
        }

        std::array<int64_t, 4> inputShape = {1, kChannels, inputHeight_, inputWidth_};
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, tensor.data(), tensor.size(), inputShape.data(), inputShape.size());

        auto outputs = session_.Run(Ort::RunOptions{nullptr},
                                    inputNamePtrs_.data(),
                                    &inputTensor,
                                    1,
                                    outputNamePtrs_.data(),
                                    1);

        if (outputs.empty() || !outputs.front().IsTensor())
        {
            return {};
        }

        const auto outputShape = outputs.front().GetTensorTypeAndShapeInfo().GetShape();
        if (outputShape.size() != 2 || outputShape[1] < 7)
        {
            return {};
        }
        const auto rows = static_cast<size_t>(outputShape[0]);
        const auto stride = static_cast<size_t>(outputShape[1]);
        const float *data = outputs.front().GetTensorData<float>();

        FaceDetections detections;
        for (size_t i = 0; i < rows; ++i)
        {
            const float *row = data + i * stride;
            const float score = row[6];
            if (score < scoreThreshold)
            {
                continue;
            }

            const float x1 = (row[1] - padX) / ratio;
            const float y1 = (row[2] - padY) / ratio;
            const float x2 = (row[3] - padX) / ratio;
            const float y2 = (row[4] - padY) / ratio;

            const float boxLeft = std::clamp(std::min(x1, x2), 0.0F, static_cast<float>(bgrImage.cols));
            const float boxTop = std::clamp(std::min(y1, y2), 0.0F, static_cast<float>(bgrImage.rows));
            const float boxRight = std::clamp(std::max(x1, x2), 0.0F, static_cast<float>(bgrImage.cols));
            const float boxBottom = std::clamp(std::max(y1, y2), 0.0F, static_cast<float>(bgrImage.rows));
            if (boxRight - boxLeft < 1.0F || boxBottom - boxTop < 1.0F)
            {
                continue;
            }

            FaceDetection detection;
            detection.box = cv::Rect2f(boxLeft, boxTop, boxRight - boxLeft, boxBottom - boxTop);
            detection.score = score;
            detections.push_back(detection);
        }

        return detections;
    }
}
