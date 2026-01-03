#include "HourlyChime.h"
#include "SettingsDialog.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QAction>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QMessageBox>

HourlyChime::HourlyChime(QObject *parent)
    : QObject(parent)
    , trayIcon(nullptr)
    , trayIconMenu(nullptr)
    , timer(new QTimer(this))
    , strikeTimer(new QTimer(this))
    , settingsDialog(nullptr)
    , player(new QMediaPlayer(this))
    , audioOutput(new QAudioOutput(this))
    , synthSink(nullptr)
    , synthGenerator(nullptr)
    , strikesLeft(0)
    , isPlayingPrelude(false)
{
    player->setAudioOutput(audioOutput);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &HourlyChime::onMediaPlayerStateChanged);

    strikeTimer->setSingleShot(true);
    connect(strikeTimer, &QTimer::timeout, this, &HourlyChime::playNextStrike);

    // Setup Synth
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);
    
    synthGenerator = new SynthGenerator(format, this);
    synthSink = new QAudioSink(format, this);
    synthSink->setVolume(1.0f);
    
    connect(synthSink, &QAudioSink::stateChanged, this, [this](QAudio::State state){
        if (state == QAudio::IdleState) {
            emit testFinished();
        } else if (state == QAudio::StoppedState) {
            if (this->synthSink->error() != QAudio::NoError) {
                 qWarning() << "AudioSink error:" << this->synthSink->error();
            }
            emit testFinished();
        }
    });

    createTrayIcon();
    
    connect(timer, &QTimer::timeout, this, &HourlyChime::checkTime);
    timer->start(1000);

    reloadConfig();
}

HourlyChime::~HourlyChime()
{
    if (settingsDialog) delete settingsDialog;
}

void HourlyChime::createTrayIcon()
{
    trayIconMenu = new QMenu();
    
    QAction *settingsAction = new QAction(tr("Settings"), this);
    connect(settingsAction, &QAction::triggered, this, &HourlyChime::showSettings);
    trayIconMenu->addAction(settingsAction);

    QAction *aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, &QAction::triggered, this, &HourlyChime::showAbout);
    trayIconMenu->addAction(aboutAction);

    QAction *quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    QIcon icon(":/images/icon.png"); 
    if (icon.isNull()) {
        icon = QIcon::fromTheme("audio-volume-high");
    }
    trayIcon->setIcon(icon);
    trayIcon->setToolTip(tr("Hourly Chime"));
    
    connect(trayIcon, &QSystemTrayIcon::activated, this, &HourlyChime::iconActivated);
    
    trayIcon->show();
}

void HourlyChime::showAbout()
{
    QMessageBox::about(nullptr, tr("About Hourly Chime"), 
        tr("<h3>Hourly Chime</h3>"
           "<p>v1.0.0</p>"
           "<p>Author: Eddie Dover</p>"
           "<p><a href='https://www.github.com/EddieDover/HourlyChime'>https://www.github.com/EddieDover/HourlyChime</a></p>"
           "<br>"
           "<h4>Attributions</h4>"
           "<p><b>Images:</b><br>"
           "Grandfather Clock Icon - Iconic Panda - Flaticon</p>"
           "<p><b>Sounds:</b><br>"
           "Default Prelude and Chime - Grandfather clock strikes ten - Pixabay</p>"));
}

void HourlyChime::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        showSettings();
    }
}

void HourlyChime::showSettings()
{
    if (!settingsDialog) {
        settingsDialog = new SettingsDialog();
        connect(settingsDialog, &SettingsDialog::configChanged, this, &HourlyChime::reloadConfig);
        connect(settingsDialog, &SettingsDialog::testRequested, this, &HourlyChime::testSound);
        connect(settingsDialog, &SettingsDialog::stopTestRequested, this, &HourlyChime::stopTest);
        connect(this, &HourlyChime::testFinished, settingsDialog, &SettingsDialog::onTestFinished);
    }
    settingsDialog->show();
    settingsDialog->raise();
    settingsDialog->activateWindow();
}

void HourlyChime::reloadConfig()
{
    currentConfig = Config::load();
    audioOutput->setVolume(currentConfig.volume);
}

void HourlyChime::checkTime()
{
    static int lastHour = -1;
    QDateTime now = QDateTime::currentDateTime();
    int currentHour = now.time().hour();

    if (lastHour != -1 && currentHour != lastHour) {
        playChime();
    }
    lastHour = currentHour;
}

void HourlyChime::playChime()
{
    reloadConfig();

    switch (currentConfig.mode == "File" ? 1 : (currentConfig.mode == "GrandfatherClock" ? 2 : 0)) {
        case 1:
            if (!currentConfig.audioFilePath.isEmpty()) {
                playFile(currentConfig.audioFilePath);
            }
            break;
        case 2:
            playGrandfatherSequence();
            break;
        default:
            playNotes(currentConfig.notes, currentConfig.noteSpeed, currentConfig.volume);
            break;
    }
}

void HourlyChime::testSound(const Config::AppConfig &config)
{
    audioOutput->setVolume(config.volume);

    if (config.mode == "File") {
        if (!config.audioFilePath.isEmpty()) {
            playFile(config.audioFilePath);
        }
    } else if (config.mode == "GrandfatherClock") {
        playGrandfatherSequence();
    } else if (config.mode == "Notes") {
        playNotes(config.notes, config.noteSpeed, config.volume);
    }
}

#include <iostream>

void HourlyChime::playNotes(const QString &notes, float speed, float volume)
{
    qDebug() << "Playing notes:" << notes << "Speed:" << speed << "Volume:" << volume;
    synthSink->stop();
    synthGenerator->setSequence(notes, speed, volume);
    synthGenerator->start();
    synthSink->start(synthGenerator);
}

void HourlyChime::playFile(const QString &path)
{
    player->setSource(QUrl::fromLocalFile(path));
    player->play();
}

void HourlyChime::playGrandfatherSequence()
{
    QDateTime now = QDateTime::currentDateTime();
    int hour = now.time().hour();
    if (hour == 0) hour = 12;
    if (hour > 12) hour -= 12;

    strikesLeft = hour;
    isPlayingPrelude = true;

    if (!currentConfig.preludeFilePath.isEmpty()) {
        playFile(currentConfig.preludeFilePath);
    } else {
        isPlayingPrelude = false;
        playNextStrike();
    }
}

void HourlyChime::stopTest()
{
    strikesLeft = 0;
    isPlayingPrelude = false;
    strikeTimer->stop();
    player->stop();
    if (synthSink) synthSink->stop();
    emit testFinished();
}

void HourlyChime::onMediaPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    if (state == QMediaPlayer::StoppedState) {
        if (currentConfig.mode == "GrandfatherClock") {
            if (isPlayingPrelude) {
                isPlayingPrelude = false;
                playNextStrike();
            } else if (strikesLeft > 1) {
                strikesLeft--;
                strikeTimer->start(currentConfig.strikeIntervalMs);
            } else {
                strikesLeft = 0;
                emit testFinished();
            }
        } else {
            emit testFinished();
        }
    }
}

void HourlyChime::playNextStrike()
{
    if (!currentConfig.strikeFilePath.isEmpty()) {
        playFile(currentConfig.strikeFilePath);
    }
}
