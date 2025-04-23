#include "text_processor.h"
#include <stdexcept>
#include <string>

std::string processText(const std::string& inputText) {
    if (inputText.empty()) {
        return "Error: Input text cannot be empty";
    }

    // Call Python NLP processing
    return processPythonNLP(QString::fromStdString(inputText)).toStdString();
}

QString processPythonNLP(const QString& inputText) {
    QProcess process;
    QString pythonScript = QCoreApplication::applicationDirPath() + "/resources/nlp_processor.py";
    
    // Print the Python script path for debugging
    qDebug() << "Python script path:" << pythonScript;
    
    // Check if the script exists
    if (!QFile::exists(pythonScript)) {
        qDebug() << "Python script not found at:" << pythonScript;
        return "Error: Python script not found";
    }
    
    process.start("python", QStringList() << pythonScript << inputText);
    if (!process.waitForStarted()) {
        qDebug() << "Failed to start Python process";
        return "Error: Failed to start Python process";
    }
    
    if (!process.waitForFinished(30000)) { // 30 second timeout
        qDebug() << "Python process timed out";
        process.kill();
        return "Error: Python process timed out";
    }
    
    // Read the standard error output
    QString error = process.readAllStandardError();
    if (!error.isEmpty()) {
        qDebug() << "Python stderr output: " << error;
    }
    
    // Read the standard output
    QString output = process.readAllStandardOutput();
    qDebug() << "Raw Python output: " << output;
    
    if (output.isEmpty()) {
        return "Error: No output received from Python script";
    }
    
    return output;
} 