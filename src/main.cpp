#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <QApplication>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    // Enable debug output
    qDebug() << "Starting TextMaster application...";
    
    QApplication app(argc, argv);
    
    // Set application information
    app.setApplicationName("TextMaster");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("TextMaster");
    
    // Set application style
    app.setStyle("Fusion");
    
    // Create resource directories if they don't exist
    QDir appDir(QCoreApplication::applicationDirPath());
    if (!appDir.exists("resources")) {
        appDir.mkdir("resources");
    }
    
    qDebug() << "Creating main window...";
    MainWindow mainWindow;
    mainWindow.show();
    qDebug() << "Main window shown";
    
    return app.exec();
}
