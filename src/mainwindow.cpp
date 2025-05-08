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
    , llmProcessor(new LLMProcessor(this))
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
    resultsPage = new QWidget(this);
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsPage);
    resultsText = new QTextEdit(resultsPage);
    resultsText->setReadOnly(true);
    QPushButton* backToHomeButton = new QPushButton("Back to Home", resultsPage);
    resultsLayout->addWidget(resultsText);
    resultsLayout->addWidget(backToHomeButton);
    connect(backToHomeButton, &QPushButton::clicked, this, &MainWindow::showHomePage);
    
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
    connect(homePage, &HomePage::importFileClicked, this, &MainWindow::onImportFileClicked);
    connect(homePage, &HomePage::analyzeTextClicked, this, &MainWindow::onAnalyzeTextClicked);
    
    connect(flashcardsPage, &FlashcardsPage::backToHome, this, &MainWindow::showHomePage);
    connect(quizPage, &QuizPage::backToHome, this, &MainWindow::showHomePage);
    connect(enumerationsPage, &EnumerationsPage::backToHome, this, &MainWindow::showHomePage);
    
    // Connect LLM processor signals
    connect(llmProcessor, &LLMProcessor::statusUpdate, this, &MainWindow::onLLMStatusUpdate);
    connect(llmProcessor, &LLMProcessor::error, this, &MainWindow::onLLMError);
    
    // Set initial state
    qDebug() << "Setting initial state...";
    stackedWidget->setCurrentWidget(homePage);
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Initializing...");
    
    // Initialize LLM in a separate thread
    QTimer::singleShot(0, this, [this]() {
        qDebug() << "Initializing LLM...";
        if (!initializeLLM()) {
            qDebug() << "Failed to initialize LLM";
            statusBar->showMessage("LLM initialization failed. Some features may be limited.");
            QMessageBox::warning(this, "Warning", "Failed to initialize LLM. Some features may be limited.");
        } else {
            statusBar->showMessage("Ready");
        }
    });
    
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

void MainWindow::onStudyGuideGenerated(const QString& result)
{
    resultsText->setText(result);
    showResultsPage();
    statusBar->showMessage("Study guide generated", 3000);
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

void MainWindow::setupStyles()
{
    setStyleSheet(getMainStyleSheet());
}

QString MainWindow::getMainStyleSheet()
{
    return QString(
        "QMainWindow {"
        "    background-color: #f5f5f5;"
        "}"
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #BDBDBD;"
        "}"
        "QTextEdit {"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    background-color: white;"
        "    color: #212121;"
        "}"
        "QLabel {"
        "    color: #212121;"
        "    font-size: 14px;"
        "}"
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    padding: 16px;"
        "}"
        "QListWidget {"
        "    background-color: white;"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 4px;"
        "}"
    );
}

QString MainWindow::getHeaderStyleSheet()
{
    return QString(
        "QWidget {"
        "    background-color: #2196F3;"
        "}"
        "QLabel {"
        "    color: white;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "}"
    );
}

QString MainWindow::getHomePageStyleSheet()
{
    return QString(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    padding: 16px;"
        "}"
        "QLabel {"
        "    color: #212121;"
        "    font-size: 14px;"
        "}"
        "QTextEdit {"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    background-color: white;"
        "    color: #212121;"
        "}"
    );
}

QString MainWindow::getResultsPageStyleSheet()
{
    return QString(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    padding: 16px;"
        "}"
        "QLabel {"
        "    color: #212121;"
        "    font-size: 14px;"
        "}"
        "QTextEdit {"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    background-color: white;"
        "    color: #212121;"
        "}"
    );
}

QString MainWindow::getHistoryPageStyleSheet()
{
    return QString(
        "QListWidget {"
        "    background-color: white;"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 4px;"
        "}"
        "QLabel {"
        "    color: #212121;"
        "    font-size: 14px;"
        "}"
    );
}

bool MainWindow::validateInputText()
{
    QString text = homePage->getInputText().trimmed();
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
    
    processingHistory.append(item);
    
    QString displayText = QString("%1\n%2")
        .arg(item.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
        .arg(item.inputText.left(100) + "...");
    
    historyList->addItem(displayText);
    saveHistory();
}

void MainWindow::loadHistory()
{
    QFile file(QCoreApplication::applicationDirPath() + "/history.json");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray array = doc.array();
        
        for (const QJsonValue& value : array) {
            QJsonObject obj = value.toObject();
            ProcessingHistoryItem item;
            item.inputText = obj["inputText"].toString();
            item.result = obj["result"].toString();
            item.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
            
            processingHistory.append(item);
            
            QString displayText = QString("%1\n%2")
                .arg(item.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
                .arg(item.inputText.left(100) + "...");
            
            historyList->addItem(displayText);
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
    QFile file(QCoreApplication::applicationDirPath() + "/history.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

bool MainWindow::initializeLLM()
{
    QString modelPath = QCoreApplication::applicationDirPath() + "/models/mistral-7b-v0.1.Q4_K_M.gguf";
    QFile modelFile(modelPath);
    if (!modelFile.exists()) {
        QMessageBox::warning(this, "Warning",
            "Model file not found: " + modelPath + "\n\n"
            "Please download the model file from:\n"
            "https://huggingface.co/TheBloke/Mistral-7B-v0.1-GGUF\n\n"
            "and place it in the 'models' directory.");
        return false;
    }
    
    return llmProcessor->initialize(modelPath);
}
