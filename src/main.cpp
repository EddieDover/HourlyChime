#include "HourlyChime.h"
#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include "Config.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    qRegisterMetaType<Config::AppConfig>();

    QCoreApplication::setOrganizationName("EddieDover");
    QCoreApplication::setApplicationName("HourlyChime");
    // Helps Wayland compositors map the window to the .desktop file
    QGuiApplication::setDesktopFileName("hourly-chime");

    app.setWindowIcon(QIcon(":/images/icon.png"));

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }

    HourlyChime chimeApp;
    
    // Check for command line args to open settings immediately
    QStringList args = app.arguments();
    if (args.contains("--settings")) {
        chimeApp.showSettings();
    }

    return app.exec();
}
