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
#include <QNetworkAccessManager>
#include <QNetworkReply>
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
    void checkForUpdates();
    void onUpdateCheckFinished(QNetworkReply *reply);
    void openUpdateUrl();

private:
    void createTrayIcon();
    
    // Audio helpers
    void playFile(const QString &path);
    void playGrandfatherSequence();
    void playNextStrike();
    void playNotes(const QString &notes, float speed, float volume);
    
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *updateAction;
    QTimer *timer;
    QTimer *strikeTimer;
    SettingsDialog *settingsDialog;

    // Network
    QNetworkAccessManager *networkManager;
    QString latestVersionUrl;
    QString latestVersionStr;

    // Audio
    QList<QMediaPlayer*> voicePool;
    QList<QAudioOutput*> outputPool;
    QMediaPlayer* getFreePlayer();
    QMediaPlayer* preludePlayer;
    
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
