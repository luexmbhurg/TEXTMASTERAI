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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include "ui_mainwindow.h"
#include "flashcardspage.h"
#include "quizpage.h"
#include "enumerationspage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , llmProcessor(new LLMProcessor(this))
    , currentFlashcardIndex(0)
    , currentQuizQuestionIndex(0)
    , quizScore(0)
    , totalQuestions(0)
{
    ui->setupUi(this);
    
    // Initialize UI components
    setupUI();
    setupMenuBar();
    setupStyles();
    
    // Create and add pages to stacked widget
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    
    // Create pages
    homePage = new HomePage(this);
    flashcardsPage = new FlashcardsPage(this);
    quizPage = new QuizPage(this);
    enumerationsPage = new EnumerationsPage(this);
    historyPage = new QWidget(this);
    
    // Add pages to stacked widget
    stackedWidget->addWidget(homePage);
    stackedWidget->addWidget(flashcardsPage);
    stackedWidget->addWidget(quizPage);
    stackedWidget->addWidget(enumerationsPage);
    stackedWidget->addWidget(historyPage);
    
    // Create history page
    createHistoryPage();
    
    // Connect signals
    connect(homePage, &HomePage::importFileClicked, this, &MainWindow::onImportFileClicked);
    connect(homePage, &HomePage::analyzeTextClicked, this, &MainWindow::onAnalyzeTextClicked);
    connect(homePage, &HomePage::flashcardsClicked, this, &MainWindow::onFlashcardsClicked);
    connect(homePage, &HomePage::quizClicked, this, &MainWindow::onQuizClicked);
    connect(homePage, &HomePage::enumerationsClicked, this, &MainWindow::onEnumerationsClicked);
    
    connect(flashcardsPage, &FlashcardsPage::backToHome, this, &MainWindow::showHomePage);
    connect(quizPage, &QuizPage::backToHome, this, &MainWindow::showHomePage);
    connect(enumerationsPage, &EnumerationsPage::backToHome, this, &MainWindow::showHomePage);
    
    // Connect LLM processor signals
    connect(llmProcessor, &LLMProcessor::statusUpdate, this, &MainWindow::onLLMStatusUpdate);
    connect(llmProcessor, &LLMProcessor::error, this, &MainWindow::onLLMError);
    
    // Initialize LLM
    initializeLLM();
    
    // Set initial state
    stackedWidget->setCurrentWidget(homePage);
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Ready");
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
            QMessageBox::critical(this, "Error", "Could not open file for reading.");
        }
    }
}

void MainWindow::onDownloadClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Results", "", "Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << resultsText->toPlainText();
            file.close();
        } else {
            QMessageBox::critical(this, "Error", "Could not open file for writing.");
        }
    }
}

void MainWindow::onCopyClicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(resultsText->toPlainText());
    statusBar->showMessage("Results copied to clipboard", 3000);
}

void MainWindow::onAboutAction()
{
    QMessageBox::about(this, "About TextMaster",
        "TextMaster is a text analysis and study aid application.\n\n"
        "Version 1.0\n"
        "Copyright © 2024");
}

void MainWindow::onHistoryItemClicked(QListWidgetItem* item)
{
    int index = historyList->row(item);
    if (index >= 0 && index < processingHistory.size()) {
        const ProcessingHistoryItem& historyItem = processingHistory[index];
        homePage->setInputText(historyItem.inputText);
        resultsText->setPlainText(historyItem.result);
        showResultsPage();
    }
}

bool MainWindow::validateInputText()
{
    QString text = homePage->getInputText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter some text to analyze.");
        return false;
    }
    return true;
}

void MainWindow::addToHistory(const QString& inputText, const QString& result)
{
    ProcessingHistoryItem item;
    item.inputText = inputText;
    item.result = result;
    item.timestamp = QDateTime::currentDateTime();
    
    processingHistory.prepend(item);
    if (processingHistory.size() > 50) {
        processingHistory.removeLast();
    }
    
    // Update history list
    historyList->clear();
    for (const ProcessingHistoryItem& historyItem : processingHistory) {
        QString displayText = QString("%1 - %2")
            .arg(historyItem.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(historyItem.inputText.left(50) + "...");
        historyList->addItem(displayText);
    }
    
    saveHistory();
}

void MainWindow::loadHistory()
{
    QFile file("history.json");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            for (const QJsonValue& value : array) {
                if (value.isObject()) {
                    QJsonObject obj = value.toObject();
                    ProcessingHistoryItem item;
                    item.inputText = obj["inputText"].toString();
                    item.result = obj["result"].toString();
                    item.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
                    processingHistory.append(item);
                }
            }
        }
        file.close();
    }
}

void MainWindow::saveHistory()
{
    QJsonArray array;
    for (const ProcessingHistoryItem& item : processingHistory) {
        QJsonObject obj;
        obj["inputText"] = item.inputText;
        obj["result"] = item.result;
        obj["timestamp"] = item.timestamp.toString(Qt::ISODate);
        array.append(obj);
    }
    
    QJsonDocument doc(array);
    QFile file("history.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson());
        file.close();
    }
}

bool MainWindow::initializeLLM()
{
    QString modelPath = "models/mistral-7b-instruct-v0.1.Q4_K_M.gguf";
    return llmProcessor->initialize(modelPath);
}

void MainWindow::onAnalyzeTextClicked()
{
    if (!validateInputText()) {
        return;
    }
    
    QString inputText = homePage->getInputText();
    statusBar->showMessage("Generating study guide...");
    
    QFuture<QString> future = llmProcessor->generateStudyGuideAsync(inputText);
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, [this, watcher, inputText]() {
        QString result = watcher->result();
        onStudyGuideGenerated(result);
        addToHistory(inputText, result);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void MainWindow::onFlashcardsClicked()
{
    if (!validateInputText()) {
        return;
    }
    
    QString inputText = homePage->getInputText();
    statusBar->showMessage("Generating flashcards...");
    
    QFuture<QString> future = llmProcessor->generateFlashcardsAsync(inputText);
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, [this, watcher, inputText]() {
        QString result = watcher->result();
        onFlashcardsGenerated(result);
        addToHistory(inputText, result);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void MainWindow::onQuizClicked()
{
    if (!validateInputText()) {
        return;
    }
    
    QString inputText = homePage->getInputText();
    statusBar->showMessage("Generating quiz...");
    
    QFuture<QString> future = llmProcessor->generateQuizAsync(inputText);
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, [this, watcher, inputText]() {
        QString result = watcher->result();
        onQuizGenerated(result);
        addToHistory(inputText, result);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void MainWindow::onEnumerationsClicked()
{
    if (!validateInputText()) {
        return;
    }
    
    QString inputText = homePage->getInputText();
    statusBar->showMessage("Generating enumerations...");
    
    QFuture<QString> future = llmProcessor->generateEnumerationsAsync(inputText);
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, [this, watcher, inputText]() {
        QString result = watcher->result();
        onEnumerationsGenerated(result);
        addToHistory(inputText, result);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

void MainWindow::onStudyGuideGenerated(const QString& result)
{
    resultsText->setPlainText(result);
    showResultsPage();
    statusBar->showMessage("Study guide generated", 3000);
}

void MainWindow::onFlashcardsGenerated(const QString& result)
{
    processFlashcardsResponse(result);
    stackedWidget->setCurrentWidget(flashcardsPage);
    statusBar->showMessage("Flashcards generated", 3000);
}

void MainWindow::onQuizGenerated(const QString& result)
{
    processQuizResponse(result);
    stackedWidget->setCurrentWidget(quizPage);
    statusBar->showMessage("Quiz generated", 3000);
}

void MainWindow::onEnumerationsGenerated(const QString& result)
{
    processEnumerationsResponse(result);
    stackedWidget->setCurrentWidget(enumerationsPage);
    statusBar->showMessage("Enumerations generated", 3000);
}

void MainWindow::onLLMStatusUpdate(const QString& status)
{
    statusBar->showMessage(status);
}

void MainWindow::onLLMError(const QString& error)
{
    QMessageBox::critical(this, "Error", error);
    statusBar->showMessage("Error: " + error, 5000);
}

void MainWindow::processFlashcardsResponse(const QString& response)
{
    try {
        QVector<QPair<QString, QString>> cards;
        
        // Split into individual flashcards
        QStringList cardBlocks = response.split("\n\n", Qt::SkipEmptyParts);
        
        for (const QString& block : cardBlocks) {
            QStringList lines = block.split("\n", Qt::SkipEmptyParts);
            if (lines.size() >= 2) {
                QString term = lines[0];
                QString definition = lines[1];
                
                // Remove any "Q:" or "A:" prefixes if present
                QString cleanTerm = term;
                cleanTerm.replace(QRegularExpression("^[QA]:\\s*"), "");
                
                QString cleanDefinition = definition;
                cleanDefinition.replace(QRegularExpression("^[QA]:\\s*"), "");
                
                cards.append(qMakePair(cleanTerm.trimmed(), cleanDefinition.trimmed()));
            }
        }
        
        flashcardsPage->setFlashcards(cards);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Error processing flashcards: %1").arg(e.what()));
        statusBar->showMessage("Error processing flashcards");
    }
}

void MainWindow::processQuizResponse(const QString& response)
{
    try {
        QVector<QPair<QString, QString>> questions;
        
        // Split into individual questions
        QStringList questionBlocks = response.split("\n\n", Qt::SkipEmptyParts);
        
        for (const QString& block : questionBlocks) {
            QStringList lines = block.split("\n", Qt::SkipEmptyParts);
            if (lines.size() >= 2) {
                QString question = lines[0];
                QString answer = lines[1];
                
                // Remove any "Q:" or "A:" prefixes if present
                QString cleanQuestion = question;
                cleanQuestion.replace(QRegularExpression("^[QA]:\\s*"), "");
                
                QString cleanAnswer = answer;
                cleanAnswer.replace(QRegularExpression("^[QA]:\\s*"), "");
                
                questions.append(qMakePair(cleanQuestion.trimmed(), cleanAnswer.trimmed()));
            }
        }
        
        quizPage->setQuestions(questions);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Error processing quiz: %1").arg(e.what()));
        statusBar->showMessage("Error processing quiz");
    }
}

void MainWindow::processEnumerationsResponse(const QString& response)
{
    try {
        QStringList items;
        
        // Split into individual items
        QStringList lines = response.split("\n", Qt::SkipEmptyParts);
        
        for (const QString& line : lines) {
            // Remove any numbering or bullet points
            QString cleanLine = line;
            cleanLine.replace(QRegularExpression("^[0-9]+[.)]\\s*|^[-*]\\s*"), "");
            items.append(cleanLine.trimmed());
        }
        
        enumerationsPage->setEnumerations(items);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Error processing enumerations: %1").arg(e.what()));
        statusBar->showMessage("Error processing enumerations");
    }
}

QString MainWindow::getMainStyleSheet()
{
    return R"(
        QMainWindow {
            background-color: #f0f0f0;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
    )";
}

QString MainWindow::getHeaderStyleSheet()
{
    return R"(
        QLabel {
            font-size: 24px;
            font-weight: bold;
            color: #333;
        }
    )";
}

QString MainWindow::getHomePageStyleSheet()
{
    return R"(
        QTextEdit {
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 8px;
            background-color: white;
        }
        QLabel {
            color: #666;
        }
    )";
}

QString MainWindow::getResultsPageStyleSheet()
{
    return R"(
        QTextEdit {
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 8px;
            background-color: white;
        }
        QLabel {
            color: #333;
        }
    )";
}

QString MainWindow::getHistoryPageStyleSheet()
{
    return R"(
        QListWidget {
            border: 1px solid #ccc;
            border-radius: 4px;
            background-color: white;
        }
        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #eee;
        }
        QListWidget::item:selected {
            background-color: #e0e0e0;
        }
    )";
}
