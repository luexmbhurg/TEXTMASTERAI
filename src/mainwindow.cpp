
#include "mainwindow.h"
#include <QImageReader>
#include <QStandardPaths>
#include <QDateTime>
#include <QAudioBuffer>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), isRecording(false), recordingDuration(0) {
    setupUI();
    setupMenuBar();
    setupLayout();
    setupActions();
    setupRecording();
    
    setWindowTitle("TextMaster - AI Notes Creator");
    setAcceptDrops(true);
    resize(900, 700);
    
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Ready");
}

MainWindow::~MainWindow() {
    if (audioRecorder) {
        delete audioRecorder;
    }
    if (audioProbe) {
        delete audioProbe;
    }
    if (recordingTimer) {
        delete recordingTimer;
    }
}

void MainWindow::setupUI() {
    // Create Tab Widget
    tabWidget = new QTabWidget(this);
    
    // Input Tab
    inputTab = new QWidget();
    inputTypeGroup = new QGroupBox("Input Type");
    textInputRadio = new QRadioButton("Text");
    audioInputRadio = new QRadioButton("Audio");
    imageInputRadio = new QRadioButton("Image");
    textInputRadio->setChecked(true);
    
    QVBoxLayout *inputTypeLayout = new QVBoxLayout();
    inputTypeLayout->addWidget(textInputRadio);
    inputTypeLayout->addWidget(audioInputRadio);
    inputTypeLayout->addWidget(imageInputRadio);
    inputTypeGroup->setLayout(inputTypeLayout);
    
    inputTextEdit = new QTextEdit();
    inputTextEdit->setPlaceholderText("Enter or paste your text here...");
    
    importButton = new QPushButton("Import File");
    recordButton = new QPushButton("Record Audio");
    recordButton->setCheckable(true);
    recordingLevelBar = new QProgressBar();
    recordingLevelBar->setRange(0, 100);
    recordingLevelBar->setValue(0);
    recordingLevelBar->setTextVisible(false);
    recordingLevelBar->setVisible(false);
    recordingStatusLabel = new QLabel("Ready to record");
    recordingStatusLabel->setVisible(false);
    wordCountLabel = new QLabel("Words: 0");
    
    // Options Tab
    optionsTab = new QWidget();
    summaryOptionsGroup = new QGroupBox("Summary Style");
    briefRadio = new QRadioButton("Brief");
    detailedRadio = new QRadioButton("Detailed");
    bulletPointsRadio = new QRadioButton("Bullet Points");
    briefRadio->setChecked(true);
    
    QVBoxLayout *summaryOptionsLayout = new QVBoxLayout();
    summaryOptionsLayout->addWidget(briefRadio);
    summaryOptionsLayout->addWidget(detailedRadio);
    summaryOptionsLayout->addWidget(bulletPointsRadio);
    summaryOptionsGroup->setLayout(summaryOptionsLayout);
    
    summaryLengthLabel = new QLabel("Summary Length:");
    summaryLengthSpinBox = new QSpinBox();
    summaryLengthSpinBox->setRange(1, 10);
    summaryLengthSpinBox->setValue(3);
    summaryLengthSpinBox->setSuffix(" paragraphs");
    
    readabilityLabel = new QLabel("Readability Level:");
    readabilitySlider = new QSlider(Qt::Horizontal);
    readabilitySlider->setRange(1, 5);
    readabilitySlider->setValue(3);
    readabilitySlider->setTickPosition(QSlider::TicksBelow);
    readabilitySlider->setTickInterval(1);
    
    // Output
    outputTextEdit = new QTextEdit();
    outputTextEdit->setPlaceholderText("Your notes will appear here...");
    outputTextEdit->setReadOnly(true);
    
    generateNotesButton = new QPushButton("Generate Notes");
    copyButton = new QPushButton("Copy to Clipboard");
    saveButton = new QPushButton("Save Notes");
    clearButton = new QPushButton("Clear All");
}

void MainWindow::setupMenuBar() {
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    QMenu *fileMenu = menuBar->addMenu("&File");
    QAction *importAction = fileMenu->addAction("&Import");
    importAction->setShortcut(QKeySequence::Open);
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportButtonClicked);
    
    QAction *saveAction = fileMenu->addAction("&Save");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveButtonClicked);
    
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    
    QMenu *editMenu = menuBar->addMenu("&Edit");
    QAction *copyAction = editMenu->addAction("&Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &MainWindow::onCopyButtonClicked);
    
    QAction *clearAction = editMenu->addAction("Clear &All");
    connect(clearAction, &QAction::triggered, this, &MainWindow::onClearButtonClicked);
    
    QMenu *helpMenu = menuBar->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutAction);
}

void MainWindow::setupLayout() {
    // Input Tab Layout
    QVBoxLayout *inputLayout = new QVBoxLayout(inputTab);
    inputLayout->addWidget(inputTypeGroup);
    
    QHBoxLayout *recordingControlsLayout = new QHBoxLayout();
    recordingControlsLayout->addWidget(importButton);
    recordingControlsLayout->addWidget(recordButton);
    
    QVBoxLayout *recordingStatusLayout = new QVBoxLayout();
    recordingStatusLayout->addWidget(recordingStatusLabel);
    recordingStatusLayout->addWidget(recordingLevelBar);
    
    inputLayout->addLayout(recordingControlsLayout);
    inputLayout->addLayout(recordingStatusLayout);
    inputLayout->addWidget(inputTextEdit);
    inputLayout->addWidget(wordCountLabel);
    
    // Options Tab Layout
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsTab);
    optionsLayout->addWidget(summaryOptionsGroup);
    
    QHBoxLayout *lengthLayout = new QHBoxLayout();
    lengthLayout->addWidget(summaryLengthLabel);
    lengthLayout->addWidget(summaryLengthSpinBox);
    
    QVBoxLayout *readabilityLayout = new QVBoxLayout();
    readabilityLayout->addWidget(readabilityLabel);
    readabilityLayout->addWidget(readabilitySlider);
    
    optionsLayout->addLayout(lengthLayout);
    optionsLayout->addLayout(readabilityLayout);
    optionsLayout->addStretch();
    
    // Add tabs
    tabWidget->addTab(inputTab, "Input");
    tabWidget->addTab(optionsTab, "Options");
    
    // Button Layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(generateNotesButton);
    buttonLayout->addWidget(copyButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(clearButton);
    
    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(tabWidget, 1);
    mainLayout->addWidget(outputTextEdit, 1);
    mainLayout->addLayout(buttonLayout);
    
    // Central Widget
    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::setupActions() {
    connect(importButton, &QPushButton::clicked, this, &MainWindow::onImportButtonClicked);
    connect(generateNotesButton, &QPushButton::clicked, this, &MainWindow::onGenerateNotesButtonClicked);
    connect(copyButton, &QPushButton::clicked, this, &MainWindow::onCopyButtonClicked);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveButtonClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearButtonClicked);
    connect(recordButton, &QPushButton::toggled, this, &MainWindow::onRecordButtonToggled);
    connect(textInputRadio, &QRadioButton::toggled, this, &MainWindow::onInputTypeChanged);
    connect(audioInputRadio, &QRadioButton::toggled, this, &MainWindow::onInputTypeChanged);
    connect(imageInputRadio, &QRadioButton::toggled, this, &MainWindow::onInputTypeChanged);
    connect(inputTextEdit, &QTextEdit::textChanged, this, &MainWindow::updateWordCount);
}

void MainWindow::setupRecording() {
    audioRecorder = new QAudioRecorder(this);
    audioProbe = new QAudioProbe(this);
    audioProbe->setSource(audioRecorder);
    
    connect(audioProbe, &QAudioProbe::audioBufferProbed, this, &MainWindow::updateRecordingLevel);
    
    recordingTimer = new QTimer(this);
    connect(recordingTimer, &QTimer::timeout, this, &MainWindow::updateRecordingStatus);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData* mimeData = event->mimeData();
    
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QString filePath = urlList.at(0).toLocalFile();
        
        QFileInfo fileInfo(filePath);
        QString suffix = fileInfo.suffix().toLower();
        
        if (audioInputRadio->isChecked() && 
            (suffix == "mp3" || suffix == "wav" || suffix == "ogg" || suffix == "m4a")) {
            processAudioInput(filePath);
        } else if (imageInputRadio->isChecked() && 
                 (suffix == "jpg" || suffix == "jpeg" || suffix == "png" || suffix == "gif" || suffix == "bmp")) {
            processImageInput(filePath);
        } else if (textInputRadio->isChecked() && 
                 (suffix == "txt" || suffix == "doc" || suffix == "docx" || suffix == "pdf")) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                inputTextEdit->setText(stream.readAll());
                file.close();
            }
        } else {
            QMessageBox::warning(this, "Invalid File", "Please drop a valid file type based on your selected input type.");
        }
    }
}

void MainWindow::onImportButtonClicked() {
    QString filter;
    
    if (textInputRadio->isChecked()) {
        filter = "Text Files (*.txt);;Word Documents (*.doc *.docx);;PDF Files (*.pdf);;All Files (*)";
    } else if (audioInputRadio->isChecked()) {
        filter = "Audio Files (*.mp3 *.wav *.ogg *.m4a);;All Files (*)";
    } else if (imageInputRadio->isChecked()) {
        filter = "Image Files (*.jpg *.jpeg *.png *.gif *.bmp);;All Files (*)";
    }
    
    QString filePath = QFileDialog::getOpenFileName(this, "Import File", QDir::homePath(), filter);
    
    if (filePath.isEmpty()) {
        return;
    }
    
    if (textInputRadio->isChecked()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            inputTextEdit->setText(stream.readAll());
            file.close();
        }
    } else if (audioInputRadio->isChecked()) {
        processAudioInput(filePath);
    } else if (imageInputRadio->isChecked()) {
        processImageInput(filePath);
    }
}

void MainWindow::onGenerateNotesButtonClicked() {
    statusBar->showMessage("Generating notes...");
    
    QString inputText;
    if (textInputRadio->isChecked()) {
        inputText = inputTextEdit->toPlainText();
        if (inputText.isEmpty()) {
            QMessageBox::warning(this, "Empty Input", "Please enter some text to generate notes.");
            statusBar->showMessage("Ready");
            return;
        }
    } else if (!inputTextEdit->toPlainText().isEmpty()) {
        // If audio or image was processed, the text should already be in the text edit
        inputText = inputTextEdit->toPlainText();
    } else {
        QMessageBox::warning(this, "No Input", "Please import or record audio, or import an image first.");
        statusBar->showMessage("Ready");
        return;
    }
    
    // Get the processing mode
    std::string mode = summarizationModeString().toStdString();
    
    // Process the text with the C++ engine
    std::string result = processText(inputText.toStdString(), mode);
    
    // Display the result
    outputTextEdit->setText(QString::fromStdString(result));
    
    statusBar->showMessage("Notes generated successfully");
}

QString MainWindow::summarizationModeString() {
    QString mode;
    
    if (briefRadio->isChecked()) {
        mode = "brief";
    } else if (detailedRadio->isChecked()) {
        mode = "detailed";
    } else if (bulletPointsRadio->isChecked()) {
        mode = "bullets";
    }
    
    // Add length parameter
    mode += "_" + QString::number(summaryLengthSpinBox->value());
    
    // Add readability parameter
    mode += "_" + QString::number(readabilitySlider->value());
    
    return mode;
}

void MainWindow::onCopyButtonClicked() {
    if (outputTextEdit->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "No Content", "There are no notes to copy.");
        return;
    }
    
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(outputTextEdit->toPlainText());
    
    statusBar->showMessage("Notes copied to clipboard", 3000);
}

void MainWindow::onSaveButtonClicked() {
    if (outputTextEdit->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "No Content", "There are no notes to save.");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, "Save Notes", 
                                                   QDir::homePath() + "/notes.txt", 
                                                   "Text Files (*.txt);;All Files (*)");
    
    if (filePath.isEmpty()) {
        return;
    }
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << outputTextEdit->toPlainText();
        file.close();
        
        statusBar->showMessage("Notes saved successfully", 3000);
    } else {
        QMessageBox::critical(this, "Error", "Could not save the file.");
    }
}

void MainWindow::onClearButtonClicked() {
    inputTextEdit->clear();
    outputTextEdit->clear();
    recordingLevelBar->setValue(0);
    recordingStatusLabel->setText("Ready to record");
    
    statusBar->showMessage("All content cleared", 3000);
}

void MainWindow::onRecordButtonToggled(bool checked) {
    if (checked) {
        startRecording();
    } else {
        stopRecording();
    }
}

void MainWindow::startRecording() {
    // Create a temporary file for recording
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    audioFilePath = tempDir + "/textmaster_recording_" + 
                  QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".wav";
    
    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/pcm");
    audioSettings.setQuality(QMultimedia::HighQuality);
    audioSettings.setSampleRate(44100);
    audioSettings.setChannelCount(1);
    
    audioRecorder->setEncodingSettings(audioSettings);
    audioRecorder->setOutputLocation(QUrl::fromLocalFile(audioFilePath));
    
    audioRecorder->record();
    
    recordButton->setText("Stop Recording");
    recordingLevelBar->setVisible(true);
    recordingStatusLabel->setVisible(true);
    recordingStatusLabel->setText("Recording...");
    recordingDuration = 0;
    recordingTimer->start(1000); // Update every second
    
    statusBar->showMessage("Recording audio...");
    isRecording = true;
}

void MainWindow::stopRecording() {
    if (!isRecording) {
        return;
    }
    
    audioRecorder->stop();
    recordButton->setText("Record Audio");
    recordingTimer->stop();
    
    statusBar->showMessage("Processing audio recording...");
    
    // Process the recorded audio
    processAudioInput(audioFilePath);
    
    recordingLevelBar->setValue(0);
    recordingStatusLabel->setText("Recording finished");
    isRecording = false;
}

void MainWindow::processAudioInput(const QString& audioFilePath) {
    statusBar->showMessage("Processing audio file...");
    
    // In a real application, you would call speech-to-text processing here
    // For now, we'll just show a placeholder message
    inputTextEdit->setText("This is placeholder text from audio processing.\n\n"
                          "In a real application, this would be the transcribed text from your audio file: " + audioFilePath);
    
    statusBar->showMessage("Audio processed. Ready to generate notes.");
}

void MainWindow::processImageInput(const QString& imageFilePath) {
    statusBar->showMessage("Processing image file...");
    
    // In a real application, you would call OCR processing here
    // For now, we'll just show a placeholder message
    inputTextEdit->setText("This is placeholder text from image processing.\n\n"
                          "In a real application, this would be the extracted text from your image file: " + imageFilePath);
    
    statusBar->showMessage("Image processed. Ready to generate notes.");
}

void MainWindow::updateRecordingLevel(const QAudioBuffer& buffer) {
    if (!isRecording) {
        return;
    }
    
    // Calculate audio level from buffer
    const qint16 *data = buffer.constData<qint16>();
    int len = buffer.sampleCount();
    int maxValue = 0;
    
    for (int i = 0; i < len; ++i) {
        maxValue = std::max(maxValue, abs(data[i]));
    }
    
    // Convert to percentage (assuming 16-bit audio with max value of 32768)
    int level = (maxValue * 100) / 32768;
    recordingLevelBar->setValue(level);
}

void MainWindow::updateRecordingStatus() {
    recordingDuration++;
    int minutes = recordingDuration / 60;
    int seconds = recordingDuration % 60;
    
    QString timeStr = QString("%1:%2")
                      .arg(minutes, 2, 10, QChar('0'))
                      .arg(seconds, 2, 10, QChar('0'));
    
    recordingStatusLabel->setText("Recording... " + timeStr);
}

void MainWindow::onInputTypeChanged() {
    if (audioInputRadio->isChecked()) {
        recordButton->setVisible(true);
        importButton->setText("Import Audio");
        inputTextEdit->setReadOnly(true);
        inputTextEdit->setPlaceholderText("Audio transcript will appear here...");
    } else if (imageInputRadio->isChecked()) {
        recordButton->setVisible(false);
        importButton->setText("Import Image");
        inputTextEdit->setReadOnly(true);
        inputTextEdit->setPlaceholderText("Extracted text from image will appear here...");
    } else {
        recordButton->setVisible(false);
        importButton->setText("Import Text");
        inputTextEdit->setReadOnly(false);
        inputTextEdit->setPlaceholderText("Enter or paste your text here...");
    }
    
    recordingLevelBar->setVisible(false);
    recordingStatusLabel->setVisible(false);
}

void MainWindow::onAboutAction() {
    QMessageBox::about(this, "About TextMaster",
                      "<h2>TextMaster</h2>"
                      "<p>Version 1.0</p>"
                      "<p>An advanced note summarization tool that converts audio, text, "
                      "and images into concise, well-organized notes.</p>"
                      "<p>Powered by AI algorithms and natural language processing.</p>"
                      "<p>© 2025 TextMaster</p>");
}

void MainWindow::updateWordCount() {
    QString text = inputTextEdit->toPlainText();
    int wordCount = text.split(QRegExp("\\s+"), Qt::SkipEmptyParts).count();
    wordCountLabel->setText(QString("Words: %1").arg(wordCount));
}
