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
#include <QJsonObject>
#include "llm_processor.h"
#include <QFutureWatcher>
#include <QLineEdit>
#include <QComboBox>
#include "home_page.h"
#include "flashcards_page.h"
#include "quiz_page.h"
#include "enumerations_page.h"

namespace Ui {
class MainWindow;
}

// Structure to store processing history
struct ProcessingHistoryItem {
    QString inputText;
    QString result;
    QDateTime timestamp;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Navigation
    void showHomePage();
    void showResultsPage();
    void showHistoryPage();
    
    // Button actions
    void onImportFileClicked();
    void onDownloadClicked();
    void onCopyClicked();
    void onAboutAction();
    void onHistoryItemClicked(QListWidgetItem* item);
    
    // Text editing
    void updateWordCount();
    
    // AI Processing
    void onAnalyzeTextClicked();
    void onFlashcardsClicked();
    void onQuizClicked();
    void onEnumerationsClicked();
    
    // Flashcards
    void onRevealDefinitionClicked();
    void onPrevFlashcardClicked();
    void onNextFlashcardClicked();
    
    // Quiz
    void onQuizSubmitClicked();
    
    // LLM processing
    void onStudyGuideGenerated(const QString& result);
    void onFlashcardsGenerated(const QString& result);
    void onQuizGenerated(const QString& result);
    void onEnumerationsGenerated(const QString& result);
    void onLLMStatusUpdate(const QString& status);
    void onLLMError(const QString& error);

    // New slots
    void onGenerateQuizClicked();
    void onGenerateFlashcardsClicked();

private:
    void setupUI();
    void setupHeader();
    void createHomePage();
    void createResultsPage();
    void createHistoryPage();
    void setupStyles();
    void setupMenuBar();
    bool validateInputText();
    void addToHistory(const QString& inputText, const QString& result);
    void loadHistory();
    void saveHistory();
    
    // Initialize LLM
    bool initializeLLM();
    
    // Process AI responses
    void processFlashcardsResponse(const QString& response);
    void processQuizResponse(const QString& response);
    void processEnumerationsResponse(const QString& response);
    
    // LLM processor
    LLMProcessor* llmProcessor;

    // Main container
    QStackedWidget *stackedWidget;

    // Header widgets
    QWidget *headerWidget;
    QPushButton *menuButton;
    QLabel *titleLabel;
    QPushButton *profileButton;

    // Pages
    HomePage *homePage;
    FlashcardsPage *flashcardsPage;
    QuizPage *quizPage;
    EnumerationsPage *enumerationsPage;
    QWidget *historyPage;

    // History page widgets
    QListWidget *historyList;
    QVector<ProcessingHistoryItem> processingHistory;

    // Status bar
    QStatusBar *statusBar;

    // Debug output text area
    QTextEdit *debugOutput;

    // Results page widgets
    QWidget *resultsPage;
    QFrame *sidebarFrame;
    QFrame *contentFrame;
    QPushButton *homeButton;
    QPushButton *generateQuizButton;
    QPushButton *generateFlashcardsButton;
    QPushButton *historyButton;
    QPushButton *downloadButton;
    QPushButton *copyButton;
    QLabel *modeDisplayLabel;
    QLabel *resultsLabel;
    QTextEdit *resultsText;

    // State variables
    int currentFlashcardIndex;
    int currentQuizQuestionIndex;
    int quizScore;
    int totalQuestions;

    // Stylesheet strings
    QString getMainStyleSheet();
    QString getHeaderStyleSheet();
    QString getHomePageStyleSheet();
    QString getResultsPageStyleSheet();
    QString getHistoryPageStyleSheet();

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
