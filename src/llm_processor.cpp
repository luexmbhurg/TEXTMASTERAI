#include "llm_processor.h"
#include <QDebug>
#include <QCoreApplication>
#include <QMetaObject>
#include <QThread>
#include <QDir>
#include <QProcess>
#include <QFuture>
#include <QPromise>
#include <QFutureWatcher>
#include <QRunnable>
#include <QThreadPool>
#include <QtConcurrent>

// Include llama.cpp headers
#include "llama.h"
#include "ggml.h"

class LLMProcessor::LLMProcessorImpl {
public:
    LLMProcessorImpl() : context(nullptr) {}
    ~LLMProcessorImpl() {
        if (context) {
            llama_free(context);
            context = nullptr;
        }
        llama_backend_free();
    }

    // Model context
    llama_context* context;
    
    // Model parameters
    llama_model* model = nullptr;
    std::string modelPath;
};

LLMProcessor::LLMProcessor(QObject* parent)
    : QObject(parent)
    , m_impl(new LLMProcessorImpl())
{
    // Initialize llama.cpp backend
    llama_backend_init();
}

LLMProcessor::~LLMProcessor()
{
    // Cleanup happens in the impl destructor
}

bool LLMProcessor::initialize(const QString& modelPath)
{
    qDebug() << "Initializing LLM with model:" << modelPath;
    emit statusUpdate("Loading LLM model...");
    
    try {
        // Free any existing context
        if (m_impl->context) {
            llama_free(m_impl->context);
            m_impl->context = nullptr;
        }
        
        if (m_impl->model) {
            llama_free_model(m_impl->model);
            m_impl->model = nullptr;
        }
        
        // Initialize model params
        llama_model_params model_params = llama_model_default_params();
        
        // Load the model
        m_impl->modelPath = modelPath.toStdString();
        m_impl->model = llama_load_model_from_file(m_impl->modelPath.c_str(), model_params);
        
        if (!m_impl->model) {
            emit error("Failed to load LLM model");
            return false;
        }
        
        // Create context params
        llama_context_params ctx_params = llama_context_default_params();
        ctx_params.n_ctx = 4096;  // Increased context size for better responses
        
        // Create context
        m_impl->context = llama_new_context_with_model(m_impl->model, ctx_params);
        
        if (!m_impl->context) {
            emit error("Failed to create LLM context");
            return false;
        }
        
        emit statusUpdate("LLM model loaded successfully");
        return true;
    } catch (const std::exception& e) {
        emit error(QString("Error initializing LLM: %1").arg(e.what()));
        return false;
    }
}

QString LLMProcessor::formatStudyGuidePrompt(const QString& input)
{
    return QString(
        "Please analyze the following text and create a comprehensive study guide. "
        "Include key concepts, important details, and explanations. "
        "Format the output with clear sections and bullet points.\n\n"
        "Text to analyze:\n%1"
    ).arg(input);
}

QString LLMProcessor::formatQuizPrompt(const QString& input)
{
    return QString(
        "Please create a quiz based on the following text. "
        "Generate questions that test understanding of key concepts. "
        "Format each question-answer pair with Q: and A: prefixes.\n\n"
        "Text to analyze:\n%1"
    ).arg(input);
}

QString LLMProcessor::formatFlashcardsPrompt(const QString& input)
{
    return QString(
        "Please create flashcards based on the following text. "
        "Each flashcard should have a term and its definition. "
        "Format each flashcard with the term on one line and the definition on the next line.\n\n"
        "Text to analyze:\n%1"
    ).arg(input);
}

QString LLMProcessor::formatEnumerationsPrompt(const QString& input)
{
    return QString(
        "Please analyze the following text and create a list of key points or enumerations. "
        "Format each point on a new line, starting with a number or bullet point. "
        "Focus on the main ideas and important details.\n\n"
        "Text to analyze:\n%1"
    ).arg(input);
}

QString LLMProcessor::generateContent(const QString& prompt)
{
    if (!m_impl->context) {
        emit error("LLM not initialized. Call initialize() first.");
        return QString();
    }
    
    try {
        std::string promptStr = prompt.toStdString();
        
        // Convert to tokens
        std::vector<llama_token> tokens(llama_n_ctx(m_impl->context));
        const struct llama_vocab* vocab = llama_model_get_vocab(llama_get_model(m_impl->context));
        int n_tokens = llama_tokenize(vocab, promptStr.c_str(), promptStr.length(), tokens.data(), tokens.size(), true, false);
        if (n_tokens < 0) {
            emit error("Failed to tokenize input");
            return QString();
        }
        tokens.resize(n_tokens);
        
        // Process tokens
        llama_batch batch = llama_batch_init(tokens.size(), 0, 1);
        for (size_t i = 0; i < tokens.size(); i++) {
            batch.token[i] = tokens[i];
            batch.pos[i] = i;
            batch.seq_id[i] = 0;
            batch.logits[i] = false;
        }
        batch.logits[batch.n_tokens - 1] = true;
        
        if (llama_decode(m_impl->context, batch)) {
            emit error("Failed to process tokens");
            llama_batch_free(batch);
            return QString();
        }
        
        // Generate response
        std::string response;
        const int max_tokens = 2048;
        llama_token token = 0;
        
        for (int i = 0; i < max_tokens; i++) {
            // Get logits and sample token
            const float* logits = llama_get_logits(m_impl->context);
            token = llama_sampler_sample(llama_sampler_init_dist(0), m_impl->context, 0);
            
            if (token == llama_vocab_eos(vocab)) {
                break;  // End of sequence
            }
            
            // Convert token to text
            char piece[8];
            int n = llama_token_to_piece(vocab, token, piece, sizeof(piece), 0, false);
            if (n > 0) {
                response += std::string(piece, n);
            }
            
            // Process next token
            llama_batch batch_next = llama_batch_init(1, 0, 1);
            batch_next.token[0] = token;
            batch_next.pos[0] = tokens.size() + i;
            batch_next.seq_id[0] = 0;
            batch_next.logits[0] = true;
            
            if (llama_decode(m_impl->context, batch_next)) {
                emit error("Failed to process token");
                llama_batch_free(batch_next);
                break;
            }
            llama_batch_free(batch_next);
        }
        
        llama_batch_free(batch);
        return QString::fromStdString(response);
    } catch (const std::exception& e) {
        emit error(QString("Error generating content: %1").arg(e.what()));
        return QString();
    }
}

QString LLMProcessor::generateStudyGuide(const QString& inputText)
{
    emit statusUpdate("Generating study guide...");
    QString prompt = formatStudyGuidePrompt(inputText);
    QString result = generateContent(prompt);
    emit statusUpdate("Study guide generated");
    return result;
}

QString LLMProcessor::generateQuiz(const QString& inputText)
{
    emit statusUpdate("Generating quiz...");
    QString prompt = formatQuizPrompt(inputText);
    QString result = generateContent(prompt);
    emit statusUpdate("Quiz generated");
    return result;
}

QString LLMProcessor::generateFlashcards(const QString& inputText)
{
    emit statusUpdate("Generating flashcards...");
    QString prompt = formatFlashcardsPrompt(inputText);
    QString result = generateContent(prompt);
    emit statusUpdate("Flashcards generated");
    return result;
}

QFuture<QString> LLMProcessor::generateStudyGuideAsync(const QString& input)
{
    return QtConcurrent::run([this, input]() {
        QString prompt = formatStudyGuidePrompt(input);
        return generateContent(prompt);
    });
}

QFuture<QString> LLMProcessor::generateQuizAsync(const QString& input)
{
    return QtConcurrent::run([this, input]() {
        QString prompt = formatQuizPrompt(input);
        return generateContent(prompt);
    });
}

QFuture<QString> LLMProcessor::generateFlashcardsAsync(const QString& input)
{
    return QtConcurrent::run([this, input]() {
        QString prompt = formatFlashcardsPrompt(input);
        return generateContent(prompt);
    });
}

QFuture<QString> LLMProcessor::generateEnumerationsAsync(const QString& input)
{
    return QtConcurrent::run([this, input]() {
        QString prompt = formatEnumerationsPrompt(input);
        return generateContent(prompt);
    });
} 