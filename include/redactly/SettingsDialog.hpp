#pragma once

#include "redactly/Theme.hpp"

#include <QDialog>
#include <QString>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;

namespace redactly
{
    class SettingsDialog final : public QDialog
    {
        Q_OBJECT

    public:
        SettingsDialog(ThemeMode theme, const QString &language, bool checkForUpdates,
                       bool fileLogging, QWidget *parent = nullptr);

    signals:
        void themeChanged(redactly::ThemeMode mode);

        void languageChanged(const QString &language);

        void checkForUpdatesChanged(bool enabled);

        void fileLoggingChanged(bool enabled);

    protected:
        void changeEvent(QEvent *event) override;

    private:
        void retranslate();

        QLabel *themeLabel_ = nullptr;
        QLabel *languageLabel_ = nullptr;
        QComboBox *themeCombo_ = nullptr;
        QComboBox *languageCombo_ = nullptr;
        QCheckBox *updateCheck_ = nullptr;
        QCheckBox *logCheck_ = nullptr;
        QPushButton *closeButton_ = nullptr;
    };
}
