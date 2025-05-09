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
#include <QTimer>

// Include llama.cpp headers
#include "llama.h"
#include "ggml.h"

struct LLMProcessor::Impl {
    llama_context* context = nullptr;
    llama_model* model = nullptr;
};

LLMProcessor::LLMProcessor(QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>())
{
    // Initialize llama.cpp backend
    llama_backend_init();
}

LLMProcessor::~LLMProcessor()
{
    cleanup();
}

void LLMProcessor::cleanup()
{
    if (m_impl) {
        if (m_impl->context) {
            llama_free(m_impl->context);
            m_impl->context = nullptr;
        }
        if (m_impl->model) {
            llama_free_model(m_impl->model);
            m_impl->model = nullptr;
        }
    }
    llama_backend_free();
}

bool LLMProcessor::initialize(const QString& modelPath)
{
    if (!m_impl) {
        emit error("Implementation not initialized");
        return false;
    }

    try {
        // Stage 1: Load model
        qDebug() << "Starting LLM initialization with model:" << modelPath;
        
        // Get model file size
        QFile modelFile(modelPath);
        if (!modelFile.exists()) {
            emit error("Model file not found");
            return false;
        }
        qint64 modelSize = modelFile.size();
        qDebug() << "Model file size:" << modelSize << "bytes";

        // Check system memory
        QProcess process;
        process.start("wmic", QStringList() << "OS" << "get" << "FreePhysicalMemory");
        process.waitForFinished();
        QString memoryInfo = process.readAllStandardOutput();
        qDebug() << "System memory info:" << memoryInfo;
        
        // Parse memory info (rough estimate)
        qint64 freeMemoryKB = memoryInfo.split("\n")[1].trimmed().toLongLong();
        qint64 freeMemoryBytes = freeMemoryKB * 1024;
        qDebug() << "Free memory in bytes:" << freeMemoryBytes;

        // Stage 2: Load model with optimized settings
        qDebug() << "Stage 2: Loading model with optimized settings...";
        
        llama_model_params model_params = llama_model_default_params();
        model_params.n_gpu_layers = 0;  // CPU only for stability
        m_impl->model = llama_load_model_from_file(modelPath.toStdString().c_str(), model_params);
        
        if (!m_impl->model) {
            emit error("Failed to load model");
            return false;
        }
        qDebug() << "Model loaded successfully";
        
        // Stage 3: Create context with optimized parameters
        qDebug() << "Stage 3: Creating context...";
        llama_context_params ctx_params = llama_context_default_params();
        
        // Set context parameters for stability
        ctx_params.n_ctx = 2048;          // Increased context size to match model's training context
        ctx_params.n_batch = 2048;        // Match batch size to context
        ctx_params.n_threads = 4;        // Use multiple threads
        ctx_params.n_threads_batch = 4;  // Use multiple threads for batch
        
        // Memory and performance settings
        ctx_params.type_k = GGML_TYPE_F32;  // Use F32 for KV cache
        ctx_params.type_v = GGML_TYPE_F32;  // Use F32 for KV cache
        ctx_params.logits_all = false;      // Disable all logits
        ctx_params.embeddings = false;      // Disable embeddings
        ctx_params.offload_kqv = false;     // Disable KQV offloading
        
        // Calculate required memory
        qint64 requiredMemory = ctx_params.n_ctx * ctx_params.n_batch * sizeof(float) * 2;  // Rough estimate
        qDebug() << "Creating context with parameters:";
        qDebug() << "  n_ctx:" << ctx_params.n_ctx;
        qDebug() << "  n_batch:" << ctx_params.n_batch;
        qDebug() << "  n_threads:" << ctx_params.n_threads;
        qDebug() << "Estimated required memory:" << requiredMemory << "bytes";
        
        if (freeMemoryBytes < requiredMemory * 2) {  // Ensure we have at least 2x the required memory
            qDebug() << "Not enough memory available";
            emit error("Not enough memory available");
            cleanup();
            return false;
        }
        
        m_impl->context = llama_new_context_with_model(m_impl->model, ctx_params);
        if (!m_impl->context) {
            qDebug() << "Failed to create context";
            emit error("Failed to create context");
            llama_free_model(m_impl->model);
            m_impl->model = nullptr;
            return false;
        }
        
        // Verify context parameters
        int n_ctx = llama_n_ctx(m_impl->context);
        qDebug() << "Context created with size:" << n_ctx;
        if (n_ctx != 2048) {
            qDebug() << "Warning: Context size mismatch. Expected 2048, got" << n_ctx;
            // Try to recreate with correct size
            llama_free(m_impl->context);
            ctx_params.n_ctx = 2048;
            m_impl->context = llama_new_context_with_model(m_impl->model, ctx_params);
            if (!m_impl->context) {
                qDebug() << "Failed to recreate context with correct size";
                emit error("Failed to create context with correct size");
                cleanup();
                return false;
            }
            n_ctx = llama_n_ctx(m_impl->context);
            qDebug() << "Recreated context with size:" << n_ctx;
        }

        return true;
    } catch (const std::exception& e) {
        emit error(QString("Error initializing LLM: %1").arg(e.what()));
        cleanup();
        return false;
    }
}

QString LLMProcessor::processText(const QString& prompt)
{
    if (!m_impl->context || !m_impl->model) {
        emit error("LLM not initialized");
        return QString();
    }

    qDebug() << "Processing prompt:" << prompt;

    try {
        // Get vocab for tokenization
        const llama_vocab* vocab = llama_model_get_vocab(m_impl->model);
        if (!vocab) {
            qDebug() << "Failed to get vocab";
            emit error("Failed to get vocab");
            return QString();
        }

        // Convert QString to std::string
        std::string prompt_std = prompt.toStdString();

        // Tokenize the prompt
        std::vector<llama_token> tokens;
        tokens.resize(prompt_std.length() + 1);
        int n_tokens = llama_tokenize(vocab, prompt_std.c_str(), prompt_std.length(), tokens.data(), tokens.size(), true, false);
        
        if (n_tokens < 0) {
            qDebug() << "Failed to tokenize input";
            emit error("Failed to tokenize input");
            return QString();
        }
        
        tokens.resize(n_tokens);
        qDebug() << "Tokenized" << n_tokens << "tokens";

        // Process the text in chunks
        std::vector<llama_token> response_tokens;
        const int max_response_tokens = 2048;  // Maximum tokens in response
        
        // Create initial batch for the prompt
        llama_batch batch = llama_batch_init(tokens.size(), 0, 1);
        for (size_t i = 0; i < tokens.size(); i++) {
            batch.token[i] = tokens[i];
            batch.pos[i] = i;
            batch.n_seq_id[i] = 1;
            batch.seq_id[i][0] = 0;
            batch.logits[i] = false;  // Disable logits for all tokens initially
        }
        batch.n_tokens = tokens.size();
        
        // Process the prompt
        if (llama_decode(m_impl->context, batch) != 0) {
            qDebug() << "Failed to decode prompt";
            llama_batch_free(batch);
            return QString();
        }
        
        // Generate response tokens
        for (int i = 0; i < max_response_tokens; i++) {
            // Create a new batch for the next token
            llama_batch next_batch = llama_batch_init(1, 0, 1);
            next_batch.token[0] = (i == 0) ? tokens.back() : response_tokens.back();
            next_batch.pos[0] = tokens.size() + i;
            next_batch.n_seq_id[0] = 1;
            next_batch.seq_id[0][0] = 0;
            next_batch.logits[0] = true;  // Enable logits for the next token
            next_batch.n_tokens = 1;
            
            // Process next token
            if (llama_decode(m_impl->context, next_batch) != 0) {
                qDebug() << "Failed to decode token";
                llama_batch_free(next_batch);
                break;
            }
            
            // Get logits for the next token
            const float* logits = llama_get_logits_ith(m_impl->context, 0);
            if (!logits) {
                qDebug() << "Failed to get logits";
                llama_batch_free(next_batch);
                break;
            }
            
            // Find the token with the highest probability
            llama_token best_token = -1;
            float best_logit = -INFINITY;
            const int n_vocab = llama_vocab_n_tokens(vocab);
            
            for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
                float logit = logits[token_id];
                if (logit > best_logit) {
                    best_logit = logit;
                    best_token = token_id;
                }
            }
            
            if (best_token == -1) {
                qDebug() << "Failed to find best token";
                llama_batch_free(next_batch);
                break;
            }
            
            // Add token to response
            response_tokens.push_back(best_token);
            
            // Check for end of text
            if (best_token == llama_vocab_eos(vocab)) {
                llama_batch_free(next_batch);
                break;
            }
            
            llama_batch_free(next_batch);
        }
        
        // Convert tokens to text
        std::string response;
        char piece[8];
        for (const auto& token : response_tokens) {
            int length = llama_token_to_piece(vocab, token, piece, sizeof(piece), 0, false);
            if (length > 0) {
                response.append(piece, length);
            }
        }
        
        return QString::fromStdString(response);

    } catch (const std::exception& e) {
        emit error(QString("Error processing text: %1").arg(e.what()));
        return QString();
    }
}

QString LLMProcessor::generateStudyGuide(const QString& inputText)
{
    QString prompt = formatStudyGuidePrompt(inputText);
    return processText(prompt);
}

QString LLMProcessor::generateQuiz(const QString& inputText)
{
    QString prompt = formatQuizPrompt(inputText);
    return processText(prompt);
}

QString LLMProcessor::generateFlashcards(const QString& inputText)
{
    QString prompt = formatFlashcardsPrompt(inputText);
    return processText(prompt);
}

QString LLMProcessor::generateEnumerations(const QString& inputText)
{
    QString prompt = formatEnumerationsPrompt(inputText);
    return processText(prompt);
}

QFuture<QString> LLMProcessor::generateStudyGuideAsync(const QString& input)
{
    return QtConcurrent::run([this, input]() {
        return generateStudyGuide(input);
    });
}

QFuture<QString> LLMProcessor::generateQuizAsync(const QString& input)
{
    return QtConcurrent::run([this, input]() {
        return generateQuiz(input);
    });
}

QFuture<QString> LLMProcessor::generateFlashcardsAsync(const QString& input)
{
    return QtConcurrent::run([this, input]() {
        return generateFlashcards(input);
    });
}

QFuture<QString> LLMProcessor::generateEnumerationsAsync(const QString& input)
{
    return QtConcurrent::run([this, input]() {
        return generateEnumerations(input);
    });
}

QString LLMProcessor::formatStudyGuidePrompt(const QString& input)
{
    return QString("Create a comprehensive study guide for the following text:\n\n%1\n\n"
                  "Study Guide:").arg(input);
}

QString LLMProcessor::formatQuizPrompt(const QString& input)
{
    return QString("Create a quiz with multiple choice questions based on this text:\n\n%1\n\n"
                  "Quiz:").arg(input);
}

QString LLMProcessor::formatFlashcardsPrompt(const QString& input)
{
    return QString("Create flashcards (question on front, answer on back) based on this text:\n\n%1\n\n"
                  "Flashcards:").arg(input);
}

QString LLMProcessor::formatEnumerationsPrompt(const QString& input)
{
    return QString("Create a list of key points and enumerations from this text:\n\n%1\n\n"
                  "Key Points:").arg(input);
} 