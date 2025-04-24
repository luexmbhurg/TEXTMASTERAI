#include "text_processor.h"
#include <stdexcept>
#include <string>
#include <QThread>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

// Global mutex for process synchronization
static QMutex processMutex;
static QWaitCondition processCondition;

void cleanupPythonProcess(QProcess* process) {
    if (process) {
        if (process->state() == QProcess::Running) {
            process->terminate();
            if (!process->waitForFinished(5000)) {
                process->kill();
            }
        }
        process->deleteLater();
    }
}

std::string processText(const std::string& inputText) {
    if (inputText.empty()) {
        return "Error: Input text cannot be empty";
    }

    // Call Python NLP processing
    return processPythonNLP(QString::fromStdString(inputText)).toStdString();
}

QString processPythonNLP(const QString& inputText) {
    QProcess* process = new QProcess();
    QString pythonScript = QCoreApplication::applicationDirPath() + "/resources/nlp_processor.py";
    
    // Print the Python script path for debugging
    qDebug() << "Python script path:" << pythonScript;
    
    // Check if the script exists
    if (!QFile::exists(pythonScript)) {
        qDebug() << "Python script not found at:" << pythonScript;
        cleanupPythonProcess(process);
        return "Error: Python script not found";
    }
    
    // Set environment variables for Python
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    process->setProcessEnvironment(env);
    
    // Start the process
    QStringList arguments;
    arguments << pythonScript;
    process->start("python", arguments);
    
    if (!process->waitForStarted()) {
        qDebug() << "Failed to start Python process";
        cleanupPythonProcess(process);
        return "Error: Failed to start Python process";
    }
    
    // Write input with proper encoding
    QByteArray utf8Text = inputText.toUtf8();
    process->write(utf8Text);
    process->closeWriteChannel();
    
    // Wait for completion with timeout
    if (!process->waitForFinished(30000)) { // 30 second timeout
        qDebug() << "Python process timed out";
        cleanupPythonProcess(process);
        return "Error: Python process timed out";
    }
    
    // Read the standard error output
    QString error = process->readAllStandardError();
    if (!error.isEmpty()) {
        qDebug() << "Python stderr output: " << error;
    }
    
    // Read the standard output
    QString output = process->readAllStandardOutput();
    qDebug() << "Raw Python output: " << output;
    
    cleanupPythonProcess(process);
    
    if (output.isEmpty()) {
        qDebug() << "No output received from Python script";
        return "Error: No output received from Python script";
    }
    
    // Try to parse the output as JSON
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    if (doc.isNull()) {
        qDebug() << "Failed to parse JSON output";
        return "Error: Invalid JSON output from Python script";
    }
    
    return output;
}

QFuture<QString> processPythonNLPAsync(const QString& inputText) {
    return QtConcurrent::run([inputText]() {
        QMutexLocker locker(&processMutex);
        return processPythonNLP(inputText);
    });
} 