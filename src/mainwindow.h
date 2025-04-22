
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QProgressBar>
#include <QTimer>
#include <QTabWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QMediaRecorder>
#include <QUrl>
#include <QString>
#include <QBuffer>
#include <QByteArray>
#include <QMimeData>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDropEvent>

// Forward declare external functions
std::string processText(const std::string& inputText, const std::string& mode);

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onImportButtonClicked();
    void onGenerateNotesButtonClicked();
    void onCopyButtonClicked();
    void onSaveButtonClicked();
    void onClearButtonClicked();
    void onRecordButtonToggled(bool checked);
    void onInputTypeChanged();
    void onAboutAction();
    void updateRecordingLevel(const QAudioBuffer& buffer);
    void updateRecordingStatus();
    void updateWordCount();

private:
    void setupUI();
    void setupMenuBar();
    void setupLayout();
    void setupActions();
    void setupRecording();
    void processAudioInput(const QString& audioFilePath);
    void processImageInput(const QString& imageFilePath);
    void startRecording();
    void stopRecording();
    QString summarizationModeString();

    // UI Components
    QTabWidget *tabWidget;
    
    // Input Tab
    QWidget *inputTab;
    QGroupBox *inputTypeGroup;
    QRadioButton *textInputRadio;
    QRadioButton *audioInputRadio;
    QRadioButton *imageInputRadio;
    QTextEdit *inputTextEdit;
    QPushButton *importButton;
    QPushButton *recordButton;
    QProgressBar *recordingLevelBar;
    QLabel *recordingStatusLabel;
    QLabel *wordCountLabel;
    
    // Options Tab
    QWidget *optionsTab;
    QGroupBox *summaryOptionsGroup;
    QRadioButton *briefRadio;
    QRadioButton *detailedRadio;
    QRadioButton *bulletPointsRadio;
    QSpinBox *summaryLengthSpinBox;
    QLabel *summaryLengthLabel;
    QSlider *readabilitySlider;
    QLabel *readabilityLabel;
    
    // Output
    QTextEdit *outputTextEdit;
    QPushButton *generateNotesButton;
    QPushButton *copyButton;
    QPushButton *saveButton;
    QPushButton *clearButton;
    
    // Audio Recording
    QAudioRecorder *audioRecorder;
    QAudioProbe *audioProbe;
    QString audioFilePath;
    bool isRecording;
    QTimer *recordingTimer;
    int recordingDuration;
    
    // Status bar
    QStatusBar *statusBar;
};

#endif // MAINWINDOW_H
