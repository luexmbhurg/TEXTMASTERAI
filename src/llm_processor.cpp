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
    try {
        qDebug() << "Starting LLM initialization with model:" << modelPath;
        
        // Stage 1: Check model file and system resources
        if (!QFile::exists(modelPath)) {
            qDebug() << "Model file does not exist at path:" << modelPath;
            emit error("Model file not found");
            return false;
        }
        
        QFile modelFile(modelPath);
        qint64 modelSize = modelFile.size();
        qDebug() << "Model file size:" << modelSize << "bytes";
        
        // Get system memory info
        QProcess process;
        process.start("wmic", QStringList() << "OS" << "get" << "FreePhysicalMemory");
        process.waitForFinished();
        QString output = process.readAllStandardOutput();
        qDebug() << "System memory info:" << output;
        
        // Parse memory info
        bool ok;
        qint64 freeMemory = 0;
        QStringList lines = output.split("\n");
        for (const QString& line : lines) {
            freeMemory = line.trimmed().toLongLong(&ok);
            if (ok) break;
        }
        freeMemory *= 1024; // Convert KB to bytes
        qDebug() << "Free memory in bytes:" << freeMemory;
        
        // Stage 2: Load model with minimal settings
        qDebug() << "Stage 2: Loading model with minimal settings...";
        llama_model_params model_params = llama_model_default_params();
        model_params.n_gpu_layers = 0;      // No GPU
        model_params.use_mmap = false;      // Disable memory mapping
        model_params.use_mlock = false;     // Disable memory locking
        model_params.vocab_only = true;     // Only load vocabulary initially
        model_params.main_gpu = 0;          // CPU only
        model_params.check_tensors = false; // Disable tensor validation
        
        // Add memory buffer
        const float memoryBuffer = 1.2f;  // 20% buffer
        if (freeMemory < modelSize * memoryBuffer) {
            qDebug() << "Warning: Low memory available. Using ultra-conservative settings.";
            model_params.vocab_only = true;
            model_params.use_mmap = false;
        }
        
        qDebug() << "Loading model...";
        m_impl->model = llama_load_model_from_file(modelPath.toStdString().c_str(), model_params);
        if (!m_impl->model) {
            qDebug() << "Failed to load model";
            emit error("Failed to load model");
            return false;
        }
        qDebug() << "Model loaded successfully";
        
        // Stage 3: Create minimal context
        qDebug() << "Stage 3: Creating minimal context...";
        llama_context_params ctx_params = llama_context_default_params();
        ctx_params.n_ctx = 2048;         // Increased context size to handle longer inputs
        ctx_params.n_threads = 1;        // Single thread
        ctx_params.n_batch = 512;        // Increased batch size
        ctx_params.type_k = GGML_TYPE_F32;  // Use F32 for KV cache
        ctx_params.type_v = GGML_TYPE_F32;  // Use F32 for KV cache
        ctx_params.logits_all = false;
        ctx_params.flash_attn = false;   // Disable flash attention
        ctx_params.offload_kqv = false;  // Disable KQV offloading
        
        qDebug() << "Creating context...";
        m_impl->context = llama_new_context_with_model(m_impl->model, ctx_params);
        if (!m_impl->context) {
            qDebug() << "Failed to create context";
            emit error("Failed to create context");
            llama_free_model(m_impl->model);
            m_impl->model = nullptr;
            return false;
        }
        qDebug() << "Context created successfully";
        
        // Stage 4: Test minimal functionality
        qDebug() << "Stage 4: Testing minimal functionality...";
        try {
            const struct llama_vocab* vocab = llama_model_get_vocab(m_impl->model);
            if (!vocab) {
                qDebug() << "Failed to get vocabulary";
                emit error("Failed to get vocabulary");
                cleanup();
                return false;
            }

            // Test with a simple token
            llama_token test_token = llama_vocab_bos(vocab);
            if (test_token < 0) {
                qDebug() << "Failed to get BOS token";
                emit error("Failed to test vocabulary");
                cleanup();
                return false;
            }

            qDebug() << "Tokenization test successful";
        } catch (const std::exception& e) {
            qDebug() << "Exception during tokenization test:" << e.what();
            emit error(QString("Tokenization test failed: %1").arg(e.what()));
            cleanup();
            return false;
        }
        
        qDebug() << "LLM initialization completed successfully";
        emit statusUpdate("LLM initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        qDebug() << "Exception during LLM initialization:" << e.what();
        emit error(QString("LLM initialization failed: %1").arg(e.what()));
        cleanup();
        return false;
    } catch (...) {
        qDebug() << "Unknown exception during LLM initialization";
        emit error("LLM initialization failed with unknown error");
        cleanup();
        return false;
    }
}

void LLMProcessor::cleanup() {
    if (m_impl->context) {
        llama_free(m_impl->context);
        m_impl->context = nullptr;
    }
    if (m_impl->model) {
        llama_free_model(m_impl->model);
        m_impl->model = nullptr;
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
        
        // Get model and vocabulary
        const llama_model* model = llama_get_model(m_impl->context);
        if (!model) {
            emit error("Failed to get model");
            return QString();
        }
        
        const struct llama_vocab* vocab = llama_model_get_vocab(model);
        if (!vocab) {
            emit error("Failed to get vocabulary");
            return QString();
        }
        
        // Get context size
        const int ctx_size = llama_n_ctx(m_impl->context);
        qDebug() << "Context size:" << ctx_size;
        
        // Convert to tokens with error checking
        std::vector<llama_token> tokens;
        tokens.resize(ctx_size);  // Use full context size
        
        qDebug() << "Tokenizing input of length:" << promptStr.length();
        int n_tokens = llama_tokenize(vocab, promptStr.c_str(), promptStr.length(), tokens.data(), tokens.size(), true, false);
        
        if (n_tokens < 0 || n_tokens >= ctx_size) {
            qDebug() << "Tokenization failed - input too long. Tokens needed:" << -n_tokens << "Context size:" << ctx_size;
            emit error("Input text is too long for the current context size. Please reduce the input length.");
            return QString();
        }
        
        qDebug() << "Successfully tokenized input into" << n_tokens << "tokens";
        tokens.resize(n_tokens);
        
        // Process tokens in batches
        const int batch_size = 32;  // Process tokens in smaller batches
        for (int i = 0; i < n_tokens; i += batch_size) {
            int current_batch_size = std::min(batch_size, n_tokens - i);
            
            llama_batch batch = llama_batch_init(current_batch_size, 0, 1);
            if (!batch.token) {
                emit error("Failed to allocate batch");
                return QString();
            }
            
            for (int j = 0; j < current_batch_size; j++) {
                batch.token[j] = tokens[i + j];
                batch.pos[j] = i + j;
                batch.seq_id[j] = 0;
                batch.logits[j] = (j == current_batch_size - 1);  // Only need logits for last token
            }
            
            if (llama_decode(m_impl->context, batch)) {
                qDebug() << "Failed to decode batch at position" << i;
                emit error("Failed to process tokens");
                llama_batch_free(batch);
                return QString();
            }
            
            llama_batch_free(batch);
        }
        
        // Generate response
        std::string response;
        const int max_tokens = 512;  // Reduced from 2048
        llama_token token = 0;
        
        // Initialize a more conservative sampler
        llama_sampler* smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
        if (!smpl) {
            emit error("Failed to initialize sampler");
            return QString();
        }
        
        llama_sampler_chain_add(smpl, llama_sampler_init_min_p(0.1f, 1));  // Higher minimum probability
        llama_sampler_chain_add(smpl, llama_sampler_init_temp(0.7f));     // Lower temperature
        llama_sampler_chain_add(smpl, llama_sampler_init_dist(0));        // Deterministic sampling
        
        for (int i = 0; i < max_tokens; i++) {
            // Get logits and sample token
            const float* logits = llama_get_logits(m_impl->context);
            if (!logits) {
                emit error("Failed to get logits");
                llama_sampler_free(smpl);
                return QString();
            }
            
            token = llama_sampler_sample(smpl, m_impl->context, 0);
            
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
            llama_batch batch = llama_batch_init(1, 0, 1);
            if (!batch.token) {
                emit error("Failed to allocate batch");
                llama_sampler_free(smpl);
                return QString();
            }
            
            batch.token[0] = token;
            batch.pos[0] = n_tokens + i;
            batch.seq_id[0] = 0;
            batch.logits[0] = true;
            
            if (llama_decode(m_impl->context, batch)) {
                emit error("Failed to process token");
                llama_sampler_free(smpl);
                llama_batch_free(batch);
                return QString();
            }
            llama_batch_free(batch);
        }
        
        llama_sampler_free(smpl);
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