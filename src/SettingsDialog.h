#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "Config.h"

class QComboBox;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QPushButton;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

signals:
    void configChanged();
    void testRequested(const Config::AppConfig &config);
    void stopTestRequested();

public slots:
    void onTestFinished();

private slots:
    void saveSettings();
    void testSettings();
    void browseAudioFile();
    void browseStrikeFile();
    void browsePreludeFile();
    void updateUiState();
    void resetDefaults();

private:
    QComboBox *modeCombo;
    QLineEdit *notesEdit;
    QDoubleSpinBox *noteSpeedSpin;
    QLineEdit *audioFileEdit;
    QLineEdit *strikeFileEdit;
    QLineEdit *preludeFileEdit;
    QSpinBox *strikeIntervalSpin;
    QDoubleSpinBox *volumeSpin;
    
    QPushButton *browseAudioBtn;
    QPushButton *browseStrikeBtn;
    QPushButton *browsePreludeBtn;
    QPushButton *testBtn;
};

#endif // SETTINGSDIALOG_H
