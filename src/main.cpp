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
    
    // Check if Python script exists
    if (!QFile::exists(pythonScriptPath)) {
        qDebug() << "Python script not found at:" << pythonScriptPath;
        return "{\"error\": \"Python script not found\", \"success\": false}";
    }
    
    // Set up Python environment
    process.setProgram("py");
    arguments << "-3.11" << pythonScriptPath << QString::fromStdString(mode) << QString::fromStdString(inputText);
    process.setArguments(arguments);
    
    // Set up process environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    process.setProcessEnvironment(env);
    
    // Start the process
    process.start();
    if (!process.waitForStarted()) {
        qDebug() << "Failed to start Python process";
        return "{\"error\": \"Failed to start Python process\", \"success\": false}";
    }
    
    // Wait for completion with timeout
    if (!process.waitForFinished(30000)) { // 30 second timeout
        process.kill();
        qDebug() << "Python process timed out";
        return "{\"error\": \"Python process timed out\", \"success\": false}";
    }
    
    // Read the output
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    
    if (!error.isEmpty()) {
        qDebug() << "Python Error: " << error;
        return "{\"error\": \"Error processing text: " + error.toStdString() + "\", \"success\": false}";
    }
    
    // Validate JSON output
    QJsonParseError jsonError;
    QJsonDocument::fromJson(output.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "Invalid JSON output:" << jsonError.errorString();
        return "{\"error\": \"Invalid JSON output from Python\", \"success\": false}";
    }
    
    return output.toStdString();
}

// C++ core processing function
std::string processText(const std::string& inputText, const std::string& mode) {
    // Validate input length
    std::vector<std::string> words;
    std::string word;
    for (char c : inputText) {
        if (std::isspace(c)) {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    
    if (words.size() > 1000) {
        return "{\"error\": \"Input text exceeds 1000 words limit\", \"success\": false}";
    }
    
    // Validate mode
    std::vector<std::string> validModes = {"brief", "detailed", "bullet_points"};
    if (std::find(validModes.begin(), validModes.end(), mode) == validModes.end()) {
        return "{\"error\": \"Invalid mode specified\", \"success\": false}";
    }
    
    // Call Python NLP processing
    return processPythonNLP(inputText, mode);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application information
    app.setApplicationName("TextMaster");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("TextMaster");
    
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
