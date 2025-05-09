#ifndef LLMPROCESSOR_H
#define LLMPROCESSOR_H

#include <QObject>
#include <QString>
#include <QFuture>
#include <memory>

class LLMProcessor : public QObject
{
    Q_OBJECT

public:
    explicit LLMProcessor(QObject* parent = nullptr);
    ~LLMProcessor();

    // Initialize the LLM model
    bool initialize(const QString& modelPath);
    void cleanup();

    // Text processing functions
    QString generateStudyGuide(const QString& inputText);
    QString generateQuiz(const QString& inputText);
    QString generateFlashcards(const QString& inputText);
    QString generateEnumerations(const QString& inputText);
    
    // Async versions
    QFuture<QString> generateStudyGuideAsync(const QString& input);
    QFuture<QString> generateQuizAsync(const QString& input);
    QFuture<QString> generateFlashcardsAsync(const QString& input);
    QFuture<QString> generateEnumerationsAsync(const QString& input);

signals:
    void error(const QString& message);
    void statusUpdate(const QString& status);

private:
    // Helper functions
    QString processText(const QString& prompt);
    QString formatStudyGuidePrompt(const QString& input);
    QString formatQuizPrompt(const QString& input);
    QString formatFlashcardsPrompt(const QString& input);
    QString formatEnumerationsPrompt(const QString& input);

    // Implementation details
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

#endif // LLMPROCESSOR_H 