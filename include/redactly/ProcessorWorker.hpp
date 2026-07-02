#pragma once

#include "redactly/Mosaic.hpp"

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>

#include <atomic>
#include <memory>

namespace redactly
{
    class ScrfdFaceDetector;
    class PlateDetector;

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
                        bool preserveMetadata,
                        bool reviewEnabled,
                        QObject *reviewReceiver,
                        std::shared_ptr<ScrfdFaceDetector> cachedDetector = {},
                        bool detectFaces = true,
                        bool detectPlates = false,
                        QString plateModelPath = {},
                        std::shared_ptr<PlateDetector> cachedPlateDetector = {});

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

        void finished(bool cancelled);

    private:
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
        bool preserveMetadata_;
        bool reviewEnabled_;
        QPointer<QObject> reviewReceiver_;
        bool detectFaces_;
        bool detectPlates_;
        QString plateModelPath_;
        std::atomic<bool> cancelled_{false};
        std::shared_ptr<ScrfdFaceDetector> detector_;
        std::shared_ptr<PlateDetector> plateDetector_;
    };
}
