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
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QClipboard>

// Forward declare external functions
std::string processText(const std::string& inputText, const std::string& mode);

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onImportButtonClicked();
    void onGenerateNotesButtonClicked();
    void onCopyButtonClicked();
    void onSaveButtonClicked();
    void onClearButtonClicked();
    void onInputTypeChanged();
    void onAboutAction();
    void updateWordCount();

private:
    void setupUI();
    void setupMenuBar();
    void setupLayout();
    void setupActions();
    QString summarizationModeString();
    bool validateInputText();

    // UI Components
    QWidget *centralWidget;
    
    // Input Section
    QGroupBox *inputGroup;
    QTextEdit *inputTextEdit;
    QPushButton *importButton;
    QLabel *wordCountLabel;
    
    // Options Section
    QGroupBox *optionsGroup;
    QRadioButton *briefRadio;
    QRadioButton *detailedRadio;
    QRadioButton *bulletPointsRadio;
    QSpinBox *summaryLengthSpinBox;
    QLabel *summaryLengthLabel;
    QSlider *readabilitySlider;
    QLabel *readabilityLabel;
    
    // Output Section
    QGroupBox *outputGroup;
    QTextEdit *outputTextEdit;
    QPushButton *generateNotesButton;
    QPushButton *copyButton;
    QPushButton *saveButton;
    QPushButton *clearButton;
    
    // Status bar
    QStatusBar *statusBar;
};

#endif // MAINWINDOW_H
