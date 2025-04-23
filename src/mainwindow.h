#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMenuBar>
#include <QStatusBar>
#include <QListWidget>
#include <QDateTime>
#include <QProgressBar>
#include <QComboBox>
#include "text_processor.h"

// Structure to store processing history
struct ProcessingHistoryItem {
    QString inputText;
    QString result;
    QDateTime timestamp;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Navigation
    void showHomePage();
    void showLoadingPage();
    void showResultsPage();
    void showHistoryPage();
    
    // Button actions
    void onImportFileClicked();
    void onLetsGoClicked();
    void onGenerateQuizClicked();
    void onGenerateFlashcardsClicked();
    void onDownloadClicked();
    void onCopyClicked();
    void onAboutAction();
    void onCancelClicked();
    void onHistoryItemClicked(QListWidgetItem* item);
    
    // Text editing
    void updateWordCount();

private:
    void setupUI();
    void setupHeader();
    void createHomePage();
    void createLoadingPage();
    void createResultsPage();
    void createHistoryPage();
    void setupStyles();
    void setupMenuBar();
    bool validateInputText();
    void addToHistory(const QString& inputText, const QString& result);
    void loadHistory();
    void saveHistory();

    // Main container
    QStackedWidget *stackedWidget;

    // Header widgets
    QWidget *headerWidget;
    QPushButton *menuButton;
    QLabel *titleLabel;
    QPushButton *profileButton;

    // Home page widgets
    QWidget *homePage;
    QLabel *decoderLabel;
    QFrame *inputFrame;
    QTextEdit *textInput;
    QLabel *wordCountLabel;
    QPushButton *importButton;
    QPushButton *letsGoButton;

    // Loading page widgets
    QWidget *loadingPage;
    QLabel *loadingIcon;
    QLabel *loadingLabel;
    QLabel *progressLabel;
    QProgressBar *progressBar;
    QPushButton *cancelButton;
    bool isCancelled;

    // Results page widgets
    QWidget *resultsPage;
    QFrame *sidebarFrame;
    QPushButton *homeButton;
    QPushButton *generateQuizButton;
    QPushButton *generateFlashcardsButton;
    QPushButton *historyButton;
    QFrame *contentFrame;
    QLabel *resultsLabel;
    QTextEdit *resultsText;
    QPushButton *downloadButton;
    QPushButton *copyButton;
    QComboBox *modeComboBox;

    // History page widgets
    QWidget *historyPage;
    QListWidget *historyList;
    QVector<ProcessingHistoryItem> processingHistory;

    // Status bar
    QStatusBar *statusBar;

    // Stylesheet strings
    QString getMainStyleSheet();
    QString getHeaderStyleSheet();
    QString getHomePageStyleSheet();
    QString getLoadingPageStyleSheet();
    QString getResultsPageStyleSheet();
    QString getHistoryPageStyleSheet();
};

#endif // MAINWINDOW_H
