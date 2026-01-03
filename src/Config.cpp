#include "Config.h"
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>

namespace Config {

QString getConfigPath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(path);
    if (!dir.exists("hourlychime")) {
        dir.mkdir("hourlychime");
    }
    return dir.filePath("hourlychime/config.json");
}

void ensureAssets() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(configPath);
    QString soundsDirPath = dir.filePath("hourlychime/sounds");
    
    if (!QDir(soundsDirPath).exists()) {
        dir.mkpath("hourlychime/sounds");
    }
    
    auto copyIfMissing = [&](const QString &resPath, const QString &fileName) {
        QString destPath = QDir(soundsDirPath).filePath(fileName);
        if (!QFile::exists(destPath)) {
            QFile::copy(resPath, destPath);
            QFile::setPermissions(destPath, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
        }
    };

    copyIfMissing(":/sounds/gc-chime.mp3", "gc-chime.mp3");
    copyIfMissing(":/sounds/gc-prelude.mp3", "gc-prelude.mp3");
}

AppConfig getDefaults() {
    AppConfig cfg;
    cfg.mode = "Notes";
    cfg.notes = "C E G C5";
    cfg.noteSpeed = 1.0f;
    cfg.strikeIntervalMs = 2000;
    cfg.volume = 1.0f;

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString soundsDir = QDir(configPath).filePath("hourlychime/sounds");
    
    cfg.audioFilePath = QDir(soundsDir).filePath("gc-chime.mp3");
    cfg.strikeFilePath = QDir(soundsDir).filePath("gc-chime.mp3");
    cfg.preludeFilePath = QDir(soundsDir).filePath("gc-prelude.mp3");

    return cfg;
}

AppConfig load() {
    ensureAssets();
    AppConfig cfg = getDefaults();

    QString configPath = getConfigPath();
    QFile file(configPath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject obj = doc.object();
        
        if (obj.contains("mode")) cfg.mode = obj["mode"].toString();
        if (obj.contains("notes")) cfg.notes = obj["notes"].toString();
        if (obj.contains("note_speed")) cfg.noteSpeed = obj["note_speed"].toDouble();
        
        if (obj.contains("audio_file_path") && !obj["audio_file_path"].isNull()) 
            cfg.audioFilePath = obj["audio_file_path"].toString();
            
        if (obj.contains("strike_file_path") && !obj["strike_file_path"].isNull()) 
            cfg.strikeFilePath = obj["strike_file_path"].toString();
            
        if (obj.contains("prelude_file_path") && !obj["prelude_file_path"].isNull()) 
            cfg.preludeFilePath = obj["prelude_file_path"].toString();
            
        if (obj.contains("strike_interval_ms")) cfg.strikeIntervalMs = obj["strike_interval_ms"].toInt();
        if (obj.contains("volume")) cfg.volume = obj["volume"].toDouble();
    }
    return cfg;
}

void save(const AppConfig &cfg) {
    QJsonObject obj;
    obj["mode"] = cfg.mode;
    obj["notes"] = cfg.notes;
    obj["note_speed"] = cfg.noteSpeed;
    
    if (!cfg.audioFilePath.isEmpty()) obj["audio_file_path"] = cfg.audioFilePath;
    else obj["audio_file_path"] = QJsonValue::Null;
    
    if (!cfg.strikeFilePath.isEmpty()) obj["strike_file_path"] = cfg.strikeFilePath;
    else obj["strike_file_path"] = QJsonValue::Null;

    if (!cfg.preludeFilePath.isEmpty()) obj["prelude_file_path"] = cfg.preludeFilePath;
    else obj["prelude_file_path"] = QJsonValue::Null;

    obj["strike_interval_ms"] = cfg.strikeIntervalMs;
    obj["volume"] = cfg.volume;

    QFile file(getConfigPath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    }
}

}
