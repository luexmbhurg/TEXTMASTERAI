#include "mainwindow.h"
#include "llm_processor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QClipboard>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QMenuBar>
#include <QApplication>
#include <QRegularExpression>
#include <QDebug>
#include <QProcess>
#include <QCoreApplication>
#include <QFuture>
#include <QFutureWatcher>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include "ui_mainwindow.h"
#include "flashcards_page.h"
#include "quiz_page.h"
#include "enumerations_page.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_llmProcessor(new LLMProcessor(this))
    , m_studyGuideWatcher(new QFutureWatcher<QString>(this))
    , isProcessing(false)
    , resultsText(new QTextEdit(this))
    , currentInputText("")
{
    qDebug() << "Starting TextMaster application...";
    ui->setupUi(this);
    
    // Initialize state variables
    currentFlashcardIndex = 0;
    currentQuizQuestionIndex = 0;
    quizScore = 0;
    totalQuestions = 0;
    
    qDebug() << "Creating main window...";
    
    // Initialize UI components
    setupUI();
    setupMenuBar();
    
    // Create and add pages to stacked widget
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    
    // Create pages
    qDebug() << "Creating pages...";
    homePage = new HomePage(this);
    flashcardsPage = new FlashcardsPage(this);
    quizPage = new QuizPage(this);
    enumerationsPage = new EnumerationsPage(this);
    historyPage = new QWidget(this);
    
    // Create results page
    qDebug() << "Creating results page...";
    resultsPage = new ResultsPage(this);
    
    // Add pages to stacked widget
    qDebug() << "Adding pages to stacked widget...";
    stackedWidget->addWidget(homePage);
    stackedWidget->addWidget(flashcardsPage);
    stackedWidget->addWidget(quizPage);
    stackedWidget->addWidget(enumerationsPage);
    stackedWidget->addWidget(historyPage);
    stackedWidget->addWidget(resultsPage);
    
    // Create history page
    qDebug() << "Creating history page...";
    createHistoryPage();
    
    // Apply styles
    qDebug() << "Setting up styles...";
    setupStyles();
    
    // Connect signals
    qDebug() << "Connecting signals...";
    connectSignals();
    
    // Set initial state
    qDebug() << "Setting initial state...";
    stackedWidget->setCurrentWidget(homePage);
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Initializing...");
    
    // Initialize LLM
    qDebug() << "Initializing LLM...";
    if (initializeLLM()) {
        statusBar->showMessage("Ready");
    } else {
        statusBar->showMessage("Failed to initialize LLM");
    }
    
    qDebug() << "Main window initialization complete";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // Setup main window properties
    setWindowTitle("TextMaster");
    resize(800, 600);
    
    // Center window on screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *importAction = fileMenu->addAction("Import File");
    QAction *downloadAction = fileMenu->addAction("Download");
    QAction *copyAction = fileMenu->addAction("Copy to Clipboard");
    
    QMenu *helpMenu = menuBar->addMenu("Help");
    QAction *aboutAction = helpMenu->addAction("About");
    
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportFileClicked);
    connect(downloadAction, &QAction::triggered, this, &MainWindow::onDownloadClicked);
    connect(copyAction, &QAction::triggered, this, &MainWindow::onCopyClicked);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutAction);
}

void MainWindow::createHistoryPage()
{
    QVBoxLayout *layout = new QVBoxLayout(historyPage);
    
    historyList = new QListWidget(historyPage);
    historyList->setWordWrap(true);
    
    QPushButton *backButton = new QPushButton("Back to Home", historyPage);
    
    layout->addWidget(historyList);
    layout->addWidget(backButton);
    
    connect(historyList, &QListWidget::itemClicked, this, &MainWindow::onHistoryItemClicked);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::showHomePage);
}

void MainWindow::showHomePage()
{
    stackedWidget->setCurrentWidget(homePage);
}

void MainWindow::showResultsPage()
{
    stackedWidget->setCurrentWidget(resultsPage);
}

void MainWindow::showHistoryPage()
{
    stackedWidget->setCurrentWidget(historyPage);
}

void MainWindow::onImportFileClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import Text File", "", "Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            homePage->setInputText(content);
            file.close();
        } else {
            QMessageBox::critical(this, "Error", "Could not open file: " + fileName);
        }
    }
}

void MainWindow::onDownloadClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Study Guide", "", "Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << resultsText->toPlainText();
            file.close();
            statusBar->showMessage("File saved successfully", 3000);
        } else {
            QMessageBox::critical(this, "Error", "Could not save file: " + fileName);
        }
    }
}

void MainWindow::onCopyClicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(resultsText->toPlainText());
    statusBar->showMessage("Copied to clipboard", 3000);
}

void MainWindow::onAboutAction()
{
    QMessageBox::about(this, "About TextMaster",
        "TextMaster v1.0.0\n\n"
        "A desktop application that helps convert text into concise, well-organized notes using local LLM processing.\n\n"
        "© 2024 TextMaster");
}

void MainWindow::onHistoryItemClicked(QListWidgetItem* item)
{
    int index = historyList->row(item);
    if (index >= 0 && index < processingHistory.size()) {
        resultsText->setText(processingHistory[index].result);
        showResultsPage();
    }
}

void MainWindow::onAnalyzeTextClicked()
{
    if (!validateInputText()) {
        return;
    }
    
    currentInputText = homePage->getInputText();
    statusBar->showMessage("Generating study guide...");
    isProcessing = true;
    
    // Show loading indicator
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    QFuture<QString> future = m_llmProcessor->generateStudyGuideAsync(currentInputText);
    m_studyGuideWatcher->setFuture(future);
}

bool MainWindow::validateInputText()
{
    QString inputText = homePage->getInputText();
    if (inputText.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter some text to analyze.");
        return false;
    }
    
    // Count words
    QRegularExpression wordRegex("\\b\\w+\\b");
    QRegularExpressionMatchIterator i = wordRegex.globalMatch(inputText);
    int wordCount = 0;
    while (i.hasNext()) {
        i.next();
        wordCount++;
    }
    
    if (wordCount > 1000) {
        QMessageBox::warning(this, "Error", "Text is too long. Please limit input to 1000 words.");
        return false;
    }
    
    return true;
}

void MainWindow::handleLLMResponse(const QString& response)
{
    // Update the results page with the response
    if (resultsPage) {
        resultsPage->setResults(response);
    } else {
        qWarning() << "Results page is null";
    }
}

void MainWindow::handleLLMError(const QString& error)
{
    QApplication::restoreOverrideCursor();
    isProcessing = false;
    QMessageBox::critical(this, "Error", "LLM Error: " + error);
    statusBar->showMessage("Error: " + error);
}

void MainWindow::handleLLMStatus(const QString& status)
{
    statusBar->showMessage(status);
}

void MainWindow::onStudyGuideGenerated(const QString& result)
{
    QApplication::restoreOverrideCursor();
    isProcessing = false;
    
    if (!result.isEmpty()) {
        resultsPage->setResults(result);
        addToHistory(currentInputText, result);
        showResultsPage();
        statusBar->showMessage("Study guide generated successfully");
    } else {
        statusBar->showMessage("Failed to generate study guide");
    }
}

void MainWindow::addToHistory(const QString& input, const QString& result)
{
    ProcessingHistoryItem item;
    item.inputText = input;
    item.result = result;
    item.timestamp = QDateTime::currentDateTime();
    
    processingHistory.prepend(item);
    
    // Update history list widget
    historyList->clear();
    for (const auto& historyItem : processingHistory) {
        QString displayText = QString("%1\n%2")
            .arg(historyItem.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(historyItem.inputText.left(100) + "...");
        historyList->addItem(displayText);
    }
    
    saveHistory();
}

void MainWindow::loadHistory()
{
    QFile file("history.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open history file";
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray array = doc.array();
    
    processingHistory.clear();
    for (const QJsonValue& value : array) {
        QJsonObject obj = value.toObject();
        ProcessingHistoryItem item;
        item.inputText = obj["input"].toString();
        item.result = obj["result"].toString();
        item.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        processingHistory.append(item);
    }
    
    // Update history list widget
    historyList->clear();
    for (const auto& historyItem : processingHistory) {
        QString displayText = QString("%1\n%2")
            .arg(historyItem.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(historyItem.inputText.left(100) + "...");
        historyList->addItem(displayText);
    }
}

void MainWindow::saveHistory()
{
    QJsonArray array;
    for (const auto& item : processingHistory) {
        QJsonObject obj;
        obj["input"] = item.inputText;
        obj["result"] = item.result;
        obj["timestamp"] = item.timestamp.toString(Qt::ISODate);
        array.append(obj);
    }
    
    QJsonDocument doc(array);
    QFile file("history.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open history file for writing";
        return;
    }
    
    file.write(doc.toJson());
}

bool MainWindow::initializeLLM()
{
    QString modelPath = QCoreApplication::applicationDirPath() + "/models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf";
    qDebug() << "Initializing LLM with model:" << modelPath;
    
    if (!QFile::exists(modelPath)) {
        QMessageBox::critical(this, "Error",
            "Model file not found: " + modelPath + "\n\n"
            "Please download the model from:\n"
            "https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF\n"
            "and place it in the models directory.");
        return false;
    }
    
    return m_llmProcessor->initialize(modelPath);
}

void MainWindow::connectSignals()
{
    // Connect LLM signals
    connect(m_llmProcessor, &LLMProcessor::error, this, &MainWindow::handleLLMError);
    connect(m_llmProcessor, &LLMProcessor::statusUpdate, this, &MainWindow::handleLLMStatus);
    
    // Connect study guide watcher
    connect(m_studyGuideWatcher, &QFutureWatcher<QString>::finished, this, [this]() {
        QString result = m_studyGuideWatcher->result();
        if (!result.isEmpty()) {
            resultsPage->setResults(result);
            addToHistory(currentInputText, result);
            showResultsPage();
            statusBar->showMessage("Study guide generated successfully");
        } else {
            statusBar->showMessage("Failed to generate study guide");
        }
        QApplication::restoreOverrideCursor();
        isProcessing = false;
    });
    
    // Connect home page signals
    connect(homePage, &HomePage::analyzeTextClicked, this, &MainWindow::onAnalyzeTextClicked);
    
    // Load history
    loadHistory();
}

void MainWindow::setupStyles()
{
    // Load main style sheet
    QFile styleFile("resources/styles/main.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = QLatin1String(styleFile.readAll());
        qApp->setStyleSheet(style);
        styleFile.close();
    } else {
        qWarning() << "Failed to load main style sheet from" << styleFile.fileName();
    }
    
    // Load page-specific styles
    loadPageStyles();
}

void MainWindow::loadPageStyles()
{
    // Load styles for each page
    const QStringList pages = {"home", "flashcards", "quiz", "enumerations", "history", "results"};
    for (const QString& page : pages) {
        QFile styleFile(QString("resources/styles/%1.qss").arg(page));
        if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
            QString style = QLatin1String(styleFile.readAll());
            // Apply style to the corresponding page
            if (page == "home" && homePage) homePage->setStyleSheet(style);
            else if (page == "flashcards" && flashcardsPage) flashcardsPage->setStyleSheet(style);
            else if (page == "quiz" && quizPage) quizPage->setStyleSheet(style);
            else if (page == "enumerations" && enumerationsPage) enumerationsPage->setStyleSheet(style);
            else if (page == "history" && historyPage) historyPage->setStyleSheet(style);
            else if (page == "results" && resultsPage) resultsPage->setStyleSheet(style);
            styleFile.close();
        } else {
            qWarning() << "Failed to load style sheet for" << page << "from" << styleFile.fileName();
        }
    }
}
