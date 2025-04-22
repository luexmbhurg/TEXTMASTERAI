
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <QApplication>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDebug>
#include "mainwindow.h"

// Function to call Python script for NLP processing
std::string processPythonNLP(const std::string& inputText, const std::string& mode) {
    QProcess process;
    QStringList arguments;
    
    // Get the executable directory path
    QString appDir = QCoreApplication::applicationDirPath();
    QString pythonScriptPath = appDir + "/resources/nlp_processor.py";
    
    // Set up Python environment
    process.setProgram("python");
    arguments << pythonScriptPath << QString::fromStdString(mode) << QString::fromStdString(inputText);
    process.setArguments(arguments);
    
    // Start the process
    process.start();
    process.waitForFinished(-1);
    
    // Read the output
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    
    if (!error.isEmpty()) {
        qDebug() << "Python Error: " << error;
        return "Error processing text. Please try again.";
    }
    
    return output.toStdString();
}

// C++ core processing function
std::string processText(const std::string& inputText, const std::string& mode) {
    // Pre-process text in C++ if needed
    std::string processedText = inputText;
    
    // Call Python NLP processing
    std::string result = processPythonNLP(processedText, mode);
    
    return result;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
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
