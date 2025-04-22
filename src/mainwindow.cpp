#include "mainwindow.h"
#include <QImageReader>
#include <QStandardPaths>
#include <QDateTime>
#include <QAudioBuffer>
#include <algorithm>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QClipboard>
#include <QFile>
#include <QDir>

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
    // Create central widget and layout
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Input Section
    inputGroup = new QGroupBox("Input Text", this);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    
    inputTextEdit = new QTextEdit(this);
    inputTextEdit->setPlaceholderText("Enter or paste your text here (max 1000 words)...");
    inputTextEdit->setMinimumHeight(200);
    
    QHBoxLayout *inputButtonsLayout = new QHBoxLayout();
    importButton = new QPushButton("Import Text File", this);
    wordCountLabel = new QLabel("Word Count: 0", this);
    inputButtonsLayout->addWidget(importButton);
    inputButtonsLayout->addStretch();
    inputButtonsLayout->addWidget(wordCountLabel);
    
    inputLayout->addWidget(inputTextEdit);
    inputLayout->addLayout(inputButtonsLayout);

    // Options Section
    optionsGroup = new QGroupBox("Note Generation Options", this);
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);
    
    QHBoxLayout *modeLayout = new QHBoxLayout();
    briefRadio = new QRadioButton("Brief Summary", this);
    detailedRadio = new QRadioButton("Detailed Summary", this);
    bulletPointsRadio = new QRadioButton("Bullet Points", this);
    briefRadio->setChecked(true);
    modeLayout->addWidget(briefRadio);
    modeLayout->addWidget(detailedRadio);
    modeLayout->addWidget(bulletPointsRadio);
    
    QHBoxLayout *lengthLayout = new QHBoxLayout();
    summaryLengthLabel = new QLabel("Summary Length:", this);
    summaryLengthSpinBox = new QSpinBox(this);
    summaryLengthSpinBox->setRange(50, 500);
    summaryLengthSpinBox->setValue(150);
    summaryLengthSpinBox->setSingleStep(50);
    lengthLayout->addWidget(summaryLengthLabel);
    lengthLayout->addWidget(summaryLengthSpinBox);
    lengthLayout->addStretch();
    
    QHBoxLayout *readabilityLayout = new QHBoxLayout();
    readabilityLabel = new QLabel("Readability Level:", this);
    readabilitySlider = new QSlider(Qt::Horizontal, this);
    readabilitySlider->setRange(1, 5);
    readabilitySlider->setValue(3);
    readabilityLayout->addWidget(readabilityLabel);
    readabilityLayout->addWidget(readabilitySlider);
    readabilityLayout->addStretch();
    
    optionsLayout->addLayout(modeLayout);
    optionsLayout->addLayout(lengthLayout);
    optionsLayout->addLayout(readabilityLayout);

    // Output Section
    outputGroup = new QGroupBox("Generated Notes", this);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);
    
    outputTextEdit = new QTextEdit(this);
    outputTextEdit->setReadOnly(true);
    outputTextEdit->setMinimumHeight(200);
    
    QHBoxLayout *outputButtonsLayout = new QHBoxLayout();
    generateNotesButton = new QPushButton("Generate Notes", this);
    copyButton = new QPushButton("Copy to Clipboard", this);
    saveButton = new QPushButton("Save Notes", this);
    clearButton = new QPushButton("Clear", this);
    outputButtonsLayout->addWidget(generateNotesButton);
    outputButtonsLayout->addWidget(copyButton);
    outputButtonsLayout->addWidget(saveButton);
    outputButtonsLayout->addWidget(clearButton);
    
    outputLayout->addWidget(outputTextEdit);
    outputLayout->addLayout(outputButtonsLayout);

    // Add all sections to main layout
    mainLayout->addWidget(inputGroup);
    mainLayout->addWidget(optionsGroup);
    mainLayout->addWidget(outputGroup);

    // Status bar
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Ready");
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
    
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportButtonClicked);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveButtonClicked);
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);
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
    QString fileName = QFileDialog::getOpenFileName(this,
        "Import Text File", "",
        "Text Files (*.txt);;All Files (*)");
    
    if (fileName.isEmpty())
        return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error",
            "Could not open file: " + file.errorString());
        return;
    }
    
    QTextStream in(&file);
    inputTextEdit->setPlainText(in.readAll());
    file.close();
}

void MainWindow::onGenerateNotesButtonClicked() {
    if (!validateInputText())
        return;
    
    statusBar->showMessage("Generating notes...");
    
    QString inputText = inputTextEdit->toPlainText();
    QString mode = summarizationModeString();
    
    // Call the C++ processing function
    std::string result = processText(inputText.toStdString(), mode.toStdString());
    
    // Parse the JSON response
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(result).toUtf8(), &jsonError);
    
    if (jsonError.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, "Error", 
            "Failed to parse response: " + jsonError.errorString());
        statusBar->showMessage("Error processing response");
        return;
    }
    
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        
        // Check for success flag
        if (!obj["success"].toBool()) {
            QString errorMessage = obj["error"].toString();
            if (errorMessage.isEmpty()) {
                errorMessage = "Unknown error occurred";
            }
            QMessageBox::warning(this, "Error", errorMessage);
            statusBar->showMessage("Error generating notes");
            return;
        }
        
        // Format the output
        QString formattedOutput;
        
        // Add summary if present
        if (obj.contains("summary")) {
            formattedOutput += "Summary:\n" + obj["summary"].toString() + "\n\n";
        }
        
        // Add key concepts if present
        if (obj.contains("key_concepts") && obj["key_concepts"].isArray()) {
            formattedOutput += "Key Concepts:\n";
            QJsonArray concepts = obj["key_concepts"].toArray();
            for (const QJsonValue& concept : concepts) {
                QJsonObject conceptObj = concept.toObject();
                formattedOutput += "• " + conceptObj["concept"].toString() + ":\n";
                formattedOutput += "  " + conceptObj["definition"].toString() + "\n\n";
            }
        }
        
        // Add formulas if present
        if (obj.contains("formulas") && obj["formulas"].isArray()) {
            formattedOutput += "Formulas:\n";
            QJsonArray formulas = obj["formulas"].toArray();
            for (const QJsonValue& formula : formulas) {
                QJsonObject formulaObj = formula.toObject();
                formattedOutput += "• " + formulaObj["formula"].toString() + "\n";
            }
            formattedOutput += "\n";
        }
        
        // Add topics if present
        if (obj.contains("topics") && obj["topics"].isArray()) {
            formattedOutput += "Topics:\n";
            QJsonArray topics = obj["topics"].toArray();
            for (const QJsonValue& topic : topics) {
                QJsonObject topicObj = topic.toObject();
                formattedOutput += "• " + topicObj["topic"].toString() + 
                                 " (Frequency: " + QString::number(topicObj["frequency"].toInt()) + ")\n";
            }
        }
        
        outputTextEdit->setPlainText(formattedOutput);
        statusBar->showMessage("Notes generated successfully");
    } else {
        QMessageBox::warning(this, "Error", "Invalid response format");
        statusBar->showMessage("Error generating notes");
    }
}

bool MainWindow::validateInputText() {
    QString text = inputTextEdit->toPlainText();
    int wordCount = text.split(QRegExp("\\s+"), QString::SkipEmptyParts).count();
    
    if (wordCount > 1000) {
        QMessageBox::warning(this, "Input Error", 
            "Input text exceeds 1000 words limit. Please reduce the text length.");
        return false;
    }
    
    if (wordCount == 0) {
        QMessageBox::warning(this, "Input Error", 
            "Please enter some text to process.");
        return false;
    }
    
    return true;
}

QString MainWindow::summarizationModeString() {
    if (briefRadio->isChecked()) return "brief";
    if (detailedRadio->isChecked()) return "detailed";
    if (bulletPointsRadio->isChecked()) return "bullet_points";
    return "brief";
}

void MainWindow::onCopyButtonClicked() {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(outputTextEdit->toPlainText());
    statusBar->showMessage("Notes copied to clipboard");
}

void MainWindow::onSaveButtonClicked() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Notes", "",
        "Text Files (*.txt);;All Files (*)");
    
    if (fileName.isEmpty())
        return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error",
            "Could not save file: " + file.errorString());
        return;
    }
    
    QTextStream out(&file);
    out << outputTextEdit->toPlainText();
    file.close();
    
    statusBar->showMessage("Notes saved successfully");
}

void MainWindow::onClearButtonClicked() {
    inputTextEdit->clear();
    outputTextEdit->clear();
    recordingLevelBar->setValue(0);
    recordingStatusLabel->setText("Ready to record");
    
    statusBar->showMessage("Cleared");
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
        "TextMaster\n\n"
        "A comprehensive Windows application for converting text (max 1000 words) into concise notes.\n\n"
        "Version 1.0.0\n"
        "Copyright © 2024");
}

void MainWindow::updateWordCount() {
    QString text = inputTextEdit->toPlainText();
    int wordCount = text.split(QRegExp("\\s+"), QString::SkipEmptyParts).count();
    wordCountLabel->setText(QString("Word Count: %1").arg(wordCount));
    
    if (wordCount > 1000) {
        wordCountLabel->setStyleSheet("color: red;");
    } else {
        wordCountLabel->setStyleSheet("");
    }
}
