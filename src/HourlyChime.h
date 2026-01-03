#ifndef HOURLYCHIME_H
#define HOURLYCHIME_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSettings>
#include <QDateTime>
#include <QAudioSink>
#include "Config.h"
#include "SynthGenerator.h"

class SettingsDialog;

class HourlyChime : public QObject
{
    Q_OBJECT

public:
    explicit HourlyChime(QObject *parent = nullptr);
    ~HourlyChime();

    void showSettings();

signals:
    void testFinished();

public slots:
    void testSound(const Config::AppConfig &config);
    void stopTest();

private slots:
    void checkTime();
    void playChime();
    void onMediaPlayerStateChanged(QMediaPlayer::PlaybackState state);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void reloadConfig();
    void showAbout();

private:
    void createTrayIcon();
    
    // Audio helpers
    void playFile(const QString &path);
    void playGrandfatherSequence();
    void playNextStrike();
    void playNotes(const QString &notes, float speed, float volume);
    
    // State
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QTimer *timer;
    QTimer *strikeTimer;
    SettingsDialog *settingsDialog;

    // Audio
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    
    // Synth
    QAudioSink *synthSink;
    SynthGenerator *synthGenerator;
    
    // Grandfather clock state
    int strikesLeft;
    bool isPlayingPrelude;

    // Config cache
    Config::AppConfig currentConfig;
};

#endif // HOURLYCHIME_H
