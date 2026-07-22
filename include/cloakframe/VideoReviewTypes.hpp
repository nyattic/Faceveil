#pragma once

#include <QMetaType>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QVector>

#include <optional>

#include "cloakframe/Tracking.hpp"

namespace cloakframe
{
    struct VideoReviewBox
    {
        int frame = 0;
        QRectF rect;
        bool interpolated = false;
    };

    struct VideoReviewTrack
    {
        int id = 0;
        QVector<VideoReviewBox> boxes;
    };

    struct VideoReviewManualTrack
    {
        int id = 0;
        int startFrame = 0;
        int endFrame = 0;
        QVector<VideoReviewBox> keyframes;
    };

    struct VideoReviewRequest
    {
        QString sourcePath;
        QString ffmpegPath;
        QString sourceName;
        QSize frameSize;
        double fps = 0.0;
        int frameCount = 0;
        QVector<VideoReviewTrack> tracks;
    };

    enum class VideoReviewDecision
    {
        Encode,
        CancelAll,
    };

    struct VideoReviewResult
    {
        VideoReviewDecision decision = VideoReviewDecision::Encode;
        QVector<int> excludedTrackIds;
        QVector<VideoReviewManualTrack> addedTracks;
    };

    [[nodiscard]] std::optional<QRectF> manualTrackRectAtFrame(
        const VideoReviewManualTrack &track, int frame);

    [[nodiscard]] std::optional<Track> materializeManualVideoTrack(
        const VideoReviewManualTrack &track, int frameCount,
        const QSize &frameSize, int assignedId);
}

Q_DECLARE_METATYPE(cloakframe::VideoReviewBox)
Q_DECLARE_METATYPE(cloakframe::VideoReviewTrack)
Q_DECLARE_METATYPE(cloakframe::VideoReviewManualTrack)
Q_DECLARE_METATYPE(cloakframe::VideoReviewRequest)
Q_DECLARE_METATYPE(cloakframe::VideoReviewResult)
