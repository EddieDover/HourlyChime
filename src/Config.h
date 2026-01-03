#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>

namespace Config {
    struct AppConfig {
        QString mode; // "Notes", "File", "GrandfatherClock"
        QString notes;
        float noteSpeed;
        QString audioFilePath;
        QString strikeFilePath;
        QString preludeFilePath;
        int strikeIntervalMs;
        float volume;
    };
}

Q_DECLARE_METATYPE(Config::AppConfig)

namespace Config {
    AppConfig load();
    void save(const AppConfig &config);
    QString getConfigPath();
    AppConfig getDefaults();
}

#endif
