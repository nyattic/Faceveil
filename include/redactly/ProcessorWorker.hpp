#pragma once

#include "redactly/ImageScanner.hpp"
#include "redactly/Mosaic.hpp"

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>

namespace redactly
{
    class ScrfdFaceDetector;
    class PlateDetector;

    enum class RunOutcome
    {
        Completed,
        Cancelled,
        Failed,
    };

    class ProcessorWorker final : public QObject
    {
        Q_OBJECT

    public:
        ProcessorWorker(QString modelPath,
                        QStringList inputs,
                        QString outputDirectory,
                        bool recursive,
                        float scoreThreshold,
                        float nmsThreshold,
                        int mosaicBlockSize,
                        float paddingRatio,
                        AnonymizationMethod method,
                        MaskShape shape,
                        bool softEdges,
                        bool preserveMetadata,
                        bool reviewEnabled,
                        QObject *reviewReceiver,
                        std::shared_ptr<ScrfdFaceDetector> cachedDetector = {},
                        bool detectFaces = true,
                        bool detectPlates = false,
                        QString plateModelPath = {},
                        std::shared_ptr<PlateDetector> cachedPlateDetector = {},
                        bool gpuAcceleration = false);

        ~ProcessorWorker() override;

        [[nodiscard]] std::shared_ptr<ScrfdFaceDetector> takeDetector();

        [[nodiscard]] std::shared_ptr<PlateDetector> takePlateDetector();

    public slots:
        void process();

        void cancel();

    signals:
        void progressChanged(int completed, int total);

        void stageChanged(int index, int total, const QString &stage, const QString &fileName);

        void logMessage(const QString &message);

        void finished(redactly::RunOutcome outcome);

    private:
        struct ItemOutcome;

        ItemOutcome processItem(const ScanResult &item,
                                const std::filesystem::path &safeRoot,
                                int index,
                                int total,
                                bool allowReview);

        QString modelPath_;
        QStringList inputs_;
        QString outputDirectory_;
        bool recursive_;
        float scoreThreshold_;
        float nmsThreshold_;
        int mosaicBlockSize_;
        float paddingRatio_;
        AnonymizationMethod method_;
        MaskShape shape_;
        bool softEdges_;
        bool preserveMetadata_;
        bool reviewEnabled_;
        QPointer<QObject> reviewReceiver_;
        bool detectFaces_;
        bool detectPlates_;
        QString plateModelPath_;
        bool gpuAcceleration_;
        std::atomic<bool> cancelled_{false};
        std::mutex detectMutex_;
        std::shared_ptr<ScrfdFaceDetector> detector_;
        std::shared_ptr<PlateDetector> plateDetector_;
    };
}

Q_DECLARE_METATYPE(redactly::RunOutcome)
