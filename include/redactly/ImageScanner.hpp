#pragma once

#include <QString>
#include <QStringList>

#include <filesystem>
#include <vector>

namespace redactly
{
    struct ScanResult
    {
        std::filesystem::path sourcePath;
        std::filesystem::path relativePath;
    };

    std::vector<ScanResult> scanImages(const QStringList &inputs, bool recursive);

    bool isSupportedImage(const std::filesystem::path &path);
}
