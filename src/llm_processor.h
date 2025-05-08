#ifndef LLMPROCESSOR_H
#define LLMPROCESSOR_H

#include <string>
#include <memory>
#include <QString>
#include <QObject>
#include <QFuture>
#include <QPromise>

class LLMProcessor : public QObject
{
    Q_OBJECT

public:
    explicit LLMProcessor(QObject* parent = nullptr);
    ~LLMProcessor();

    // Initialize the LLM model
    bool initialize(const QString& modelPath);

    // Different types of content generation
    QString generateStudyGuide(const QString& inputText);
    QString generateQuiz(const QString& inputText);
    QString generateFlashcards(const QString& inputText);
    
    // Async versions
    QFuture<QString> generateStudyGuideAsync(const QString& inputText);
    QFuture<QString> generateQuizAsync(const QString& inputText);
    QFuture<QString> generateFlashcardsAsync(const QString& inputText);
    QFuture<QString> generateEnumerationsAsync(const QString& inputText);

signals:
    void statusUpdate(const QString& status);
    void error(const QString& message);

private:
    // Pointer to the implementation (PIMPL pattern)
    class LLMProcessorImpl;
    std::unique_ptr<LLMProcessorImpl> m_impl;
    
    // Format prompts for different types
    QString formatStudyGuidePrompt(const QString& inputText);
    QString formatQuizPrompt(const QString& inputText);
    QString formatFlashcardsPrompt(const QString& inputText);
    QString formatEnumerationsPrompt(const QString& inputText);
    
    // Core generation function
    QString generateContent(const QString& prompt);

    void cleanup();
};

#endif // LLMPROCESSOR_H 