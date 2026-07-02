#include "faceveil/ImageIo.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <vector>

#ifdef FACEVEIL_HAVE_EXIV2
#include <exiv2/exiv2.hpp>
#endif

namespace faceveil
{
    namespace
    {
        std::string toUtf8(const std::filesystem::path &value)
        {
            const auto utf8 = value.u8string();
            return std::string(utf8.begin(), utf8.end());
        }
    }

    bool metadataSupportAvailable()
    {
#ifdef FACEVEIL_HAVE_EXIV2
        return true;
#else
        return false;
#endif
    }

    int readExifOrientation(const std::filesystem::path &source)
    {
#ifdef FACEVEIL_HAVE_EXIV2
        try
        {
            auto image = Exiv2::ImageFactory::open(toUtf8(source));
            image->readMetadata();
            const Exiv2::ExifData &exif = image->exifData();
            const auto it = exif.findKey(Exiv2::ExifKey("Exif.Image.Orientation"));
            if (it != exif.end())
            {
                const auto value = static_cast<int>(it->toInt64());
                if (value >= 1 && value <= 8)
                {
                    return value;
                }
            }
        }
        catch (const Exiv2::Error &)
        {
        }
#else
        (void) source;
#endif
        return 1;
    }

    void applyOrientation(cv::Mat &image, int orientation)
    {
        switch (orientation)
        {
            case 2:
                cv::flip(image, image, 1);
                break;
            case 3:
                cv::rotate(image, image, cv::ROTATE_180);
                break;
            case 4:
                cv::flip(image, image, 0);
                break;
            case 5:
                cv::transpose(image, image);
                break;
            case 6:
                cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
                break;
            case 7:
                cv::transpose(image, image);
                cv::rotate(image, image, cv::ROTATE_180);
                break;
            case 8:
                cv::rotate(image, image, cv::ROTATE_90_COUNTERCLOCKWISE);
                break;
            default:
                break;
        }
    }

    cv::Mat toDetectionBgr(const cv::Mat &image)
    {
        cv::Mat eightBit;
        if (image.depth() == CV_16U)
        {
            image.convertTo(eightBit, CV_8U, 1.0 / 257.0);
        }
        else if (image.depth() != CV_8U)
        {
            double minVal = 0.0;
            double maxVal = 0.0;
            cv::minMaxLoc(image.reshape(1), &minVal, &maxVal);
            const double scale = (maxVal > minVal) ? 255.0 / (maxVal - minVal) : 1.0;
            image.convertTo(eightBit, CV_8U, scale, -minVal * scale);
        }
        else
        {
            eightBit = image;
        }

        cv::Mat bgr;
        if (eightBit.channels() == 1)
        {
            cv::cvtColor(eightBit, bgr, cv::COLOR_GRAY2BGR);
        }
        else if (eightBit.channels() == 4)
        {
            cv::cvtColor(eightBit, bgr, cv::COLOR_BGRA2BGR);
        }
        else
        {
            bgr = eightBit;
        }
        return bgr;
    }

    cv::Mat imreadUnicode(const std::filesystem::path &source, int flags)
    {
        std::ifstream file(source, std::ios::binary);
        if (!file)
        {
            return {};
        }
        std::vector<uchar> buffer((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
        if (buffer.empty())
        {
            return {};
        }
        return cv::imdecode(buffer, flags);
    }

    bool imwriteUnicode(const std::filesystem::path &destination, const cv::Mat &image,
                        const std::vector<int> &params)
    {
        const auto extension = destination.extension();
        if (extension.empty())
        {
            return false;
        }
        std::vector<uchar> buffer;
        if (!cv::imencode(extension.string(), image, buffer, params))
        {
            return false;
        }
        std::ofstream file(destination, std::ios::binary | std::ios::trunc);
        if (!file)
        {
            return false;
        }
        file.write(reinterpret_cast<const char *>(buffer.data()),
                   static_cast<std::streamsize>(buffer.size()));
        return file.good();
    }

    std::vector<int> encodeParamsForExtension(const std::string &extLower)
    {
        std::string ext = extLower;
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (!ext.empty() && ext.front() == '.')
        {
            ext.erase(ext.begin());
        }

        if (ext == "jpg" || ext == "jpeg")
        {
            return {cv::IMWRITE_JPEG_QUALITY, 100,
                    cv::IMWRITE_JPEG_SAMPLING_FACTOR, cv::IMWRITE_JPEG_SAMPLING_FACTOR_444,
                    cv::IMWRITE_JPEG_OPTIMIZE, 1};
        }
        if (ext == "webp")
        {
            return {cv::IMWRITE_WEBP_QUALITY, 101};
        }
        if (ext == "png")
        {
            return {cv::IMWRITE_PNG_COMPRESSION, 6};
        }
        if (ext == "tif" || ext == "tiff")
        {
            return {cv::IMWRITE_TIFF_COMPRESSION, 5};
        }
        return {};
    }

    bool copyMetadata(const std::filesystem::path &source,
                      const std::filesystem::path &destination,
                      bool normalizeOrientation)
    {
#ifdef FACEVEIL_HAVE_EXIV2
        try
        {
            auto src = Exiv2::ImageFactory::open(toUtf8(source));
            src->readMetadata();

            auto dst = Exiv2::ImageFactory::open(toUtf8(destination));
            dst->readMetadata();

            Exiv2::ExifData exif = src->exifData();
            if (normalizeOrientation && !exif.empty())
            {
                exif["Exif.Image.Orientation"] = static_cast<uint16_t>(1);
            }
            dst->setExifData(exif);
            dst->setIptcData(src->iptcData());
            dst->setXmpData(src->xmpData());

            const Exiv2::DataBuf &icc = src->iccProfile();
            if (icc.size() > 0)
            {
                dst->setIccProfile(Exiv2::DataBuf(icc.c_data(), icc.size()));
            }

            dst->writeMetadata();
            return true;
        }
        catch (const Exiv2::Error &)
        {
            return false;
        }
#else
        (void) source;
        (void) destination;
        (void) normalizeOrientation;
        return false;
#endif
    }
}
