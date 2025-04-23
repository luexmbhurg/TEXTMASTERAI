#include "mainwindow.h"
#include "text_processor.h"
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    setupMenuBar();
    setupStyles();
    
    setWindowTitle("TextMaster - AI Notes Creator");
    setAcceptDrops(true);
    resize(900, 700);
    
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Ready");
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    // Set window properties
    setWindowTitle("TextMaster");
    setMinimumSize(1024, 768);
    
    // Create main container
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    
    // Setup components
    setupHeader();
    createHomePage();
    createLoadingPage();
    createResultsPage();
}

void MainWindow::setupHeader() {
    headerWidget = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    
    // Menu button
    menuButton = new QPushButton(this);
    menuButton->setIcon(QIcon(":/icons/menu.png"));
    menuButton->setFixedSize(32, 32);
    
    // Title
    titleLabel = new QLabel("TEXTMASTER", this);
    QLabel *subtitleLabel = new QLabel("YOUR POWERFUL STUDY TOOL", this);
    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    titleLayout->addWidget(subtitleLabel, 0, Qt::AlignCenter);
    
    // Profile button
    profileButton = new QPushButton(this);
    profileButton->setIcon(QIcon(":/icons/profile.png"));
    profileButton->setFixedSize(32, 32);
    
    headerLayout->addWidget(menuButton);
    headerLayout->addStretch();
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(profileButton);
}

void MainWindow::createHomePage() {
    homePage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(homePage);
    
    // Decoder label with gradient background
    decoderLabel = new QLabel("LESSON DECODER", this);
    decoderLabel->setAlignment(Qt::AlignCenter);
    
    // Input frame
    inputFrame = new QFrame(this);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputFrame);
    
    // Text input and word count
    QVBoxLayout *textLayout = new QVBoxLayout();
    textInput = new QTextEdit(this);
    textInput->setPlaceholderText("Enter or paste your text here...");
    
    // Initialize word count label
    wordCountLabel = new QLabel("Words: 0/1000", this);
    wordCountLabel->setAlignment(Qt::AlignRight);
    wordCountLabel->setStyleSheet("color: #333333;");
    
    textLayout->addWidget(textInput);
    textLayout->addWidget(wordCountLabel);
    
    // Import button
    importButton = new QPushButton("Import File", this);
    
    // Let's Go button
    letsGoButton = new QPushButton("LET'S GO!", this);
    
    inputLayout->addWidget(importButton);
    inputLayout->addLayout(textLayout);
    inputLayout->addWidget(letsGoButton, 0, Qt::AlignCenter);
    
    layout->addWidget(decoderLabel);
    layout->addWidget(inputFrame);
    
    stackedWidget->addWidget(homePage);
    
    // Connect signals
    connect(importButton, &QPushButton::clicked, this, &MainWindow::onImportFileClicked);
    connect(letsGoButton, &QPushButton::clicked, this, &MainWindow::onLetsGoClicked);
    connect(textInput, &QTextEdit::textChanged, this, &MainWindow::updateWordCount);
}

void MainWindow::createLoadingPage() {
    loadingPage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(loadingPage);
    
    loadingIcon = new QLabel(this);
    loadingIcon->setPixmap(QPixmap(":/icons/loading.png"));
    loadingIcon->setAlignment(Qt::AlignCenter);
    
    loadingLabel = new QLabel("Processing...", this);
    loadingLabel->setAlignment(Qt::AlignCenter);
    
    progressLabel = new QLabel("Initializing...", this);
    progressLabel->setAlignment(Qt::AlignCenter);
    
    progressBar = new QProgressBar(this);
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFormat("%p%");
    
    cancelButton = new QPushButton("Cancel", this);
    cancelButton->setStyleSheet("background-color: #FF5252; color: white;");
    
    layout->addStretch();
    layout->addWidget(loadingIcon);
    layout->addWidget(loadingLabel);
    layout->addWidget(progressLabel);
    layout->addWidget(progressBar);
    layout->addWidget(cancelButton, 0, Qt::AlignCenter);
    layout->addStretch();
    
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    
    stackedWidget->addWidget(loadingPage);
}

void MainWindow::createResultsPage() {
    resultsPage = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(resultsPage);
    
    // Sidebar
    sidebarFrame = new QFrame(this);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebarFrame);
    
    homeButton = new QPushButton("HOME", this);
    generateQuizButton = new QPushButton("GENERATE\nQUIZ", this);
    generateFlashcardsButton = new QPushButton("GENERATE\nFLASHCARD", this);
    historyButton = new QPushButton("HISTORY", this);
    
    sidebarLayout->addWidget(homeButton);
    sidebarLayout->addWidget(generateQuizButton);
    sidebarLayout->addWidget(generateFlashcardsButton);
    sidebarLayout->addWidget(historyButton);
    sidebarLayout->addStretch();
    
    // Content area
    contentFrame = new QFrame(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentFrame);
    
    resultsLabel = new QLabel("RESULTS:", this);
    resultsText = new QTextEdit(this);
    resultsText->setReadOnly(true);
    
    // Mode selection
    QHBoxLayout *modeLayout = new QHBoxLayout();
    QLabel *modeLabel = new QLabel("Mode:", this);
    modeComboBox = new QComboBox(this);
    modeComboBox->addItem("Summary");
    modeComboBox->addItem("Key Points");
    modeComboBox->addItem("Detailed");
    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(modeComboBox);
    modeLayout->addStretch();
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    downloadButton = new QPushButton(this);
    downloadButton->setIcon(QIcon(":/icons/download.png"));
    copyButton = new QPushButton(this);
    copyButton->setIcon(QIcon(":/icons/copy.png"));
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(downloadButton);
    buttonLayout->addWidget(copyButton);
    
    contentLayout->addLayout(modeLayout);
    contentLayout->addWidget(resultsLabel);
    contentLayout->addWidget(resultsText);
    contentLayout->addLayout(buttonLayout);
    
    layout->addWidget(sidebarFrame);
    layout->addWidget(contentFrame);
    
    stackedWidget->addWidget(resultsPage);
    
    // Connect signals
    connect(homeButton, &QPushButton::clicked, this, &MainWindow::showHomePage);
    connect(generateQuizButton, &QPushButton::clicked, this, &MainWindow::onGenerateQuizClicked);
    connect(generateFlashcardsButton, &QPushButton::clicked, this, &MainWindow::onGenerateFlashcardsClicked);
    connect(historyButton, &QPushButton::clicked, this, &MainWindow::showHistoryPage);
    connect(downloadButton, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(copyButton, &QPushButton::clicked, this, &MainWindow::onCopyClicked);
}

void MainWindow::createHistoryPage() {
    historyPage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(historyPage);
    
    QLabel *titleLabel = new QLabel("Processing History", this);
    titleLabel->setObjectName("historyTitleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    historyList = new QListWidget(this);
    historyList->setAlternatingRowColors(true);
    
    QPushButton *backButton = new QPushButton("Back", this);
    
    layout->addWidget(titleLabel);
    layout->addWidget(historyList);
    layout->addWidget(backButton);
    
    stackedWidget->addWidget(historyPage);
    
    connect(historyList, &QListWidget::itemClicked, this, &MainWindow::onHistoryItemClicked);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::showResultsPage);
    
    // Load history from file
    loadHistory();
}

void MainWindow::updateWordCount() {
    QString text = textInput->toPlainText();
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    int count = words.count();
    
    wordCountLabel->setText(QString("Words: %1/1000").arg(count));
    
    if (count > 1000) {
        wordCountLabel->setStyleSheet("color: #FF5252;"); // Red color for overflow
    } else {
        wordCountLabel->setStyleSheet("color: #333333;"); // Normal color
    }
}

void MainWindow::onCancelClicked() {
    isCancelled = true;
    showHomePage();
    statusBar->showMessage("Processing cancelled");
}

void MainWindow::onHistoryItemClicked(QListWidgetItem* item) {
    int index = historyList->row(item);
    if (index >= 0 && index < processingHistory.size()) {
        const ProcessingHistoryItem& historyItem = processingHistory[index];
        textInput->setPlainText(historyItem.inputText);
        
        resultsText->setPlainText(historyItem.result);
        showResultsPage();
    }
}

void MainWindow::addToHistory(const QString& inputText, const QString& result) {
    ProcessingHistoryItem item;
    item.inputText = inputText;
    item.result = result;
    item.timestamp = QDateTime::currentDateTime();
    
    processingHistory.prepend(item); // Add to front of list
    
    // Limit history to last 50 items
    while (processingHistory.size() > 50) {
        processingHistory.removeLast();
    }
    
    // Update history list widget
    historyList->clear();
    for (const ProcessingHistoryItem& histItem : processingHistory) {
        QString displayText = QString("%1 - %2")
            .arg(histItem.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(histItem.result.left(100)); // Limit to 100 characters
        historyList->addItem(displayText);
    }
    
    // Save history to file
    saveHistory();
}

void MainWindow::loadHistory() {
    QFile file("processing_history.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream in(&file);
    processingHistory.clear();
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split("|");
        if (parts.size() >= 3) {
            ProcessingHistoryItem item;
            item.timestamp = QDateTime::fromString(parts[0], Qt::ISODate);
            item.inputText = parts[1];
            item.result = parts[2];
            processingHistory.append(item);
        }
    }
    
    file.close();
    
    // Update history list widget
    historyList->clear();
    for (const ProcessingHistoryItem& histItem : processingHistory) {
        QString displayText = QString("%1 - %2")
            .arg(histItem.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(histItem.result.left(100)); // Limit to 100 characters
        historyList->addItem(displayText);
    }
}

void MainWindow::saveHistory() {
    QFile file("processing_history.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream out(&file);
    for (const ProcessingHistoryItem& item : processingHistory) {
        out << item.timestamp.toString(Qt::ISODate) << "|"
            << item.inputText << "|"
            << item.result << "\n";
    }
    
    file.close();
}

void MainWindow::onLetsGoClicked() {
    if (!validateInputText()) {
        return;
    }
    
    showLoadingPage();
    QString inputText = textInput->toPlainText();
    
    // Process the text using Python NLP
    QString result = processPythonNLP(inputText);
    
    if (result.startsWith("Error:")) {
        // Use a scrollable, resizable dialog for long error messages
        QDialog *errorDialog = new QDialog(this);
        errorDialog->setWindowTitle("Error");
        errorDialog->resize(600, 400);
        QVBoxLayout *layout = new QVBoxLayout(errorDialog);
        QTextEdit *errorText = new QTextEdit(errorDialog);
        errorText->setReadOnly(true);
        errorText->setPlainText(result);
        layout->addWidget(errorText);
        QPushButton *closeButton = new QPushButton("Close", errorDialog);
        connect(closeButton, &QPushButton::clicked, errorDialog, &QDialog::accept);
        layout->addWidget(closeButton, 0, Qt::AlignRight);
        errorDialog->setLayout(layout);
        errorDialog->exec();
        return;
    }
    
    // Display the results
    resultsText->setPlainText(result);
    
    // Add to history
    addToHistory(inputText, result);
    
    // Show results page
    stackedWidget->setCurrentWidget(resultsPage);
}

void MainWindow::onGenerateQuizClicked() {
    // TODO: Implement quiz generation
    QMessageBox::information(this, "Coming Soon", "Quiz generation feature is coming soon!");
}

void MainWindow::onGenerateFlashcardsClicked() {
    // TODO: Implement flashcard generation
    QMessageBox::information(this, "Coming Soon", "Flashcard generation feature is coming soon!");
}

void MainWindow::onDownloadClicked() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Results", "", "Text Files (*.txt);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
            stream << resultsText->toPlainText();
        file.close();
            statusBar->showMessage("Results saved successfully");
    } else {
            QMessageBox::warning(this, "Error", "Could not save file: " + file.errorString());
        }
    }
}

void MainWindow::onCopyClicked() {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(resultsText->toPlainText());
    statusBar->showMessage("Results copied to clipboard");
}

void MainWindow::onAboutAction() {
    QMessageBox::about(this, "About TextMaster",
        "TextMaster\n\n"
        "A comprehensive Windows application for converting text (max 1000 words) into concise notes.\n\n"
        "Version 1.0.0\n"
        "Copyright © 2024");
}

bool MainWindow::validateInputText() {
    QString text = textInput->toPlainText().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
            tr("Please enter some text to process."));
        return false;
    }
    return true;
}

void MainWindow::setupMenuBar() {
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *importAction = fileMenu->addAction("Import Text File");
    QAction *saveAction = fileMenu->addAction("Save Notes");
    QAction *exitAction = fileMenu->addAction("Exit");
    
    QMenu *helpMenu = menuBar->addMenu("Help");
    QAction *aboutAction = helpMenu->addAction("About");
    
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportFileClicked);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onDownloadClicked);
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutAction);
}

// Style setup
void MainWindow::setupStyles() {
    // Apply stylesheets
    setStyleSheet(getMainStyleSheet());
    headerWidget->setStyleSheet(getHeaderStyleSheet());
    homePage->setStyleSheet(getHomePageStyleSheet());
    loadingPage->setStyleSheet(getLoadingPageStyleSheet());
    resultsPage->setStyleSheet(getResultsPageStyleSheet());
}

QString MainWindow::getMainStyleSheet() {
    return R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QPushButton {
            background-color: #2196F3;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
        QPushButton:disabled {
            background-color: #BDBDBD;
        }
        QTextEdit {
            border: 1px solid #E0E0E0;
            border-radius: 4px;
            padding: 8px;
            background-color: white;
            color: #212121;
        }
        QLabel {
            color: #212121;
            font-size: 14px;
        }
    )";
}

QString MainWindow::getHeaderStyleSheet() {
    return R"(
        QWidget {
            background-color: #2196F3;
        }
        QPushButton {
            background-color: transparent;
            border: none;
            padding: 8px;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.1);
        }
        QLabel {
            color: white;
            font-size: 18px;
            font-weight: bold;
        }
    )";
}

QString MainWindow::getHomePageStyleSheet() {
    return R"(
        QFrame {
            background-color: white;
            border-radius: 8px;
            padding: 16px;
        }
        QLabel#decoderLabel {
            font-size: 24px;
            font-weight: bold;
            color: #212121;
        }
    )";
}

QString MainWindow::getLoadingPageStyleSheet() {
    return R"(
        QLabel {
            font-size: 16px;
            color: #424242;
        }
        QProgressBar {
            border: 1px solid #E0E0E0;
            border-radius: 4px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #2196F3;
        }
    )";
}

QString MainWindow::getResultsPageStyleSheet() {
    return R"(
        QFrame#sidebarFrame {
            background-color: #FAFAFA;
            border-right: 1px solid #E0E0E0;
        }
        QFrame#contentFrame {
            background-color: white;
            padding: 16px;
        }
        QPushButton#homeButton, 
        QPushButton#generateQuizButton,
        QPushButton#generateFlashcardsButton,
        QPushButton#historyButton {
            text-align: left;
            padding: 12px 16px;
            border-radius: 0;
            background-color: transparent;
            color: #424242;
        }
        QPushButton#homeButton:hover,
        QPushButton#generateQuizButton:hover,
        QPushButton#generateFlashcardsButton:hover,
        QPushButton#historyButton:hover {
            background-color: #E0E0E0;
        }
    )";
}

QString MainWindow::getHistoryPageStyleSheet() {
    return R"(
        QListWidget {
            background-color: white;
            border: 1px solid #E0E0E0;
            border-radius: 4px;
        }
        QListWidget::item {
            padding: 12px;
            border-bottom: 1px solid #E0E0E0;
        }
        QListWidget::item:hover {
            background-color: #F5F5F5;
        }
        QListWidget::item:selected {
            background-color: #E3F2FD;
            color: #212121;
        }
    )";
}

// Navigation functions
void MainWindow::showHomePage() {
    stackedWidget->setCurrentWidget(homePage);
}

void MainWindow::showLoadingPage() {
    stackedWidget->setCurrentWidget(loadingPage);
    isCancelled = false;
    progressBar->setValue(0);
    loadingLabel->setText("Processing your text...");
}

void MainWindow::showResultsPage() {
    stackedWidget->setCurrentWidget(resultsPage);
}

void MainWindow::showHistoryPage() {
    stackedWidget->setCurrentWidget(historyPage);
}

// Button actions
void MainWindow::onImportFileClicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Text File"), "",
        tr("Text Files (*.txt);;All Files (*)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            textInput->setText(in.readAll());
            file.close();
        } else {
            QMessageBox::warning(this, tr("Error"),
                tr("Could not open file: %1").arg(fileName));
        }
    }
}
