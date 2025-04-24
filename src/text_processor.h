#ifndef TEXT_PROCESSOR_H
#define TEXT_PROCESSOR_H

#include <string>
#include <QString>
#include <QProcess>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>

// Function to process text using NLP models
std::string processText(const std::string& inputText);

// Function to process text using NLP models
QString processPythonNLP(const QString& inputText);

// Async version of processPythonNLP
QFuture<QString> processPythonNLPAsync(const QString& inputText);

// Cleanup function for Python process
void cleanupPythonProcess(QProcess* process);

#endif // TEXT_PROCESSOR_H 