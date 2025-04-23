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
#include "text_processor.h"

int main(int argc, char *argv[]) {
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
    
    // Extract Python script if it doesn't exist
    QString pythonScriptPath = appDir.absolutePath() + "/resources/nlp_processor.py";
    if (!QFile::exists(pythonScriptPath)) {
        QFile resourceFile(":/resources/nlp_processor.py");
        if (resourceFile.open(QIODevice::ReadOnly)) {
            QByteArray scriptContent = resourceFile.readAll();
            QFile outputFile(pythonScriptPath);
            if (outputFile.open(QIODevice::WriteOnly)) {
                outputFile.write(scriptContent);
                outputFile.close();
            }
            resourceFile.close();
        }
    }
    
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}
