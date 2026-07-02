#pragma once

#include <opencv2/core.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace faceveil
{
    bool metadataSupportAvailable();

    int readExifOrientation(const std::filesystem::path &source);

    void applyOrientation(cv::Mat &image, int orientation);

    cv::Mat toDetectionBgr(const cv::Mat &image);

    cv::Mat imreadUnicode(const std::filesystem::path &source, int flags);

    bool imwriteUnicode(const std::filesystem::path &destination, const cv::Mat &image,
                        const std::vector<int> &params);

    std::vector<int> encodeParamsForExtension(const std::string &extLower);

    bool copyMetadata(const std::filesystem::path &source,
                      const std::filesystem::path &destination,
                      bool normalizeOrientation);
}
