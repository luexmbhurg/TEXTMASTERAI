#ifndef TEXT_PROCESSOR_H
#define TEXT_PROCESSOR_H

#include <string>
#include <QString>
#include <QProcess>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>

// Function to process text using NLP models
std::string processText(const std::string& inputText);

// Function to process text using NLP models
QString processPythonNLP(const QString& inputText);

#endif // TEXT_PROCESSOR_H 