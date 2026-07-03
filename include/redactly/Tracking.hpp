#pragma once

#include "redactly/FaceDetection.hpp"

#include <opencv2/core.hpp>

#include <vector>

namespace redactly
{
    struct TrackedBox
    {
        int frame = 0;
        cv::Rect2f box;
        float score = 0.0F;
        bool interpolated = false;
    };

    struct Track
    {
        int id = 0;
        std::vector<TrackedBox> boxes;

        [[nodiscard]] int firstFrame() const;
        [[nodiscard]] int lastFrame() const;
        [[nodiscard]] const TrackedBox *boxAtFrame(int frame) const;
    };

    struct TrackerConfig
    {
        float highScoreThreshold = 0.5F;
        float lowScoreThreshold = 0.1F;
        float iouThreshold = 0.3F;
        int maxFramesLost = 30;
        float velocityBlend = 0.3F;
    };

    class ByteTracker
    {
    public:
        explicit ByteTracker(TrackerConfig config = {});

        void update(int frame, const FaceDetections &detections);

        [[nodiscard]] std::vector<Track> finish();

    private:
        struct ActiveTrack
        {
            Track track;
            cv::Point2f velocity{0.0F, 0.0F};
            int lastFrame = 0;

            [[nodiscard]] cv::Rect2f predictedBox(int frame) const;
        };

        void extendTrack(ActiveTrack &active, int frame, const FaceDetection &detection);

        TrackerConfig config_;
        std::vector<ActiveTrack> active_;
        std::vector<Track> finished_;
        int nextId_ = 1;
    };

    struct TrackPostProcessConfig
    {
        int maxInterpolationGap = 30;
        int smoothingRadius = 2;
        int extensionFrames = 3;
    };

    [[nodiscard]] std::vector<Track> buildTracks(const std::vector<FaceDetections> &frameDetections,
                                                 const TrackerConfig &config = {});

    [[nodiscard]] std::vector<Track> buildBidirectionalTracks(
        const std::vector<FaceDetections> &frameDetections,
        const TrackerConfig &config = {},
        float mergeIouThreshold = 0.5F);

    void interpolateGaps(Track &track, int maxGap);

    void smoothTrack(Track &track, int radius);

    void extendTrackEnds(Track &track, int frames, int frameCount);

    void postProcessTracks(std::vector<Track> &tracks, const TrackPostProcessConfig &config, int frameCount);

    [[nodiscard]] std::vector<cv::Rect2f> trackRegionsForFrame(const std::vector<Track> &tracks, int frame);
}
