#include "cloakframe/VideoReviewTypes.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace cloakframe
{
    namespace
    {
        bool validRect(const QRectF &rect)
        {
            return std::isfinite(rect.x()) && std::isfinite(rect.y()) &&
                   std::isfinite(rect.width()) && std::isfinite(rect.height()) &&
                   rect.width() > 0.0 && rect.height() > 0.0;
        }

        QRectF interpolateRect(const QRectF &first, const QRectF &second,
                               const double amount)
        {
            return {
                first.x() + (second.x() - first.x()) * amount,
                first.y() + (second.y() - first.y()) * amount,
                first.width() + (second.width() - first.width()) * amount,
                first.height() + (second.height() - first.height()) * amount,
            };
        }

        bool validManualTrack(const VideoReviewManualTrack &track)
        {
            if (track.startFrame < 0 || track.endFrame < track.startFrame ||
                track.keyframes.isEmpty())
            {
                return false;
            }
            int previousFrame = -1;
            for (const auto &keyframe: track.keyframes)
            {
                if (keyframe.frame <= previousFrame ||
                    keyframe.frame < track.startFrame ||
                    keyframe.frame > track.endFrame || !validRect(keyframe.rect))
                {
                    return false;
                }
                previousFrame = keyframe.frame;
            }
            return true;
        }

        QRectF rectAtFrame(const VideoReviewManualTrack &track, const int frame)
        {
            const auto next = std::lower_bound(
                track.keyframes.cbegin(), track.keyframes.cend(), frame,
                [](const VideoReviewBox &box, const int value)
                {
                    return box.frame < value;
                });
            if (next == track.keyframes.cbegin())
            {
                return next->rect;
            }
            if (next == track.keyframes.cend())
            {
                return track.keyframes.back().rect;
            }
            if (next->frame == frame)
            {
                return next->rect;
            }

            const auto previous = std::prev(next);
            const double amount = static_cast<double>(frame - previous->frame) /
                                  static_cast<double>(next->frame - previous->frame);
            return interpolateRect(previous->rect, next->rect, amount);
        }
    }

    std::optional<QRectF> manualTrackRectAtFrame(
        const VideoReviewManualTrack &track, const int frame)
    {
        if (!validManualTrack(track) || frame < track.startFrame ||
            frame > track.endFrame)
        {
            return std::nullopt;
        }

        return rectAtFrame(track, frame);
    }

    std::optional<Track> materializeManualVideoTrack(
        const VideoReviewManualTrack &track, const int frameCount,
        const QSize &frameSize, const int assignedId)
    {
        if (!validManualTrack(track) || frameCount <= 0 || !frameSize.isValid() ||
            track.endFrame >= frameCount)
        {
            return std::nullopt;
        }

        Track result;
        result.id = assignedId;
        result.boxes.reserve(static_cast<std::size_t>(track.endFrame -
                                                       track.startFrame + 1));
        const QRectF frameBounds(QPointF(0.0, 0.0), QSizeF(frameSize));
        for (int frame = track.startFrame; frame <= track.endFrame; ++frame)
        {
            const QRectF interpolated = rectAtFrame(track, frame);
            const QRectF clipped = interpolated.normalized().intersected(frameBounds);
            if (!validRect(clipped))
            {
                return std::nullopt;
            }
            const auto keyframe = std::lower_bound(
                track.keyframes.cbegin(), track.keyframes.cend(), frame,
                [](const VideoReviewBox &box, const int value)
                {
                    return box.frame < value;
                });
            const bool exactKeyframe = keyframe != track.keyframes.cend() &&
                                       keyframe->frame == frame;
            result.boxes.push_back({
                frame,
                cv::Rect2f(static_cast<float>(clipped.x()),
                           static_cast<float>(clipped.y()),
                           static_cast<float>(clipped.width()),
                           static_cast<float>(clipped.height())),
                1.0F,
                !exactKeyframe,
            });
        }
        return result;
    }
}
