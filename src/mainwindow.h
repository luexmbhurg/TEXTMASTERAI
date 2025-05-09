#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QDateTime>
#include <QFutureWatcher>
#include "home_page.h"
#include "flashcards_page.h"
#include "quiz_page.h"
#include "enumerations_page.h"
#include "pages/results_page.h"
#include "llm_processor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showHomePage();
    void showResultsPage();
    void showHistoryPage();
    void onImportFileClicked();
    void onDownloadClicked();
    void onCopyClicked();
    void onAboutAction();
    void onHistoryItemClicked(QListWidgetItem* item);
    void onAnalyzeTextClicked();
    void onStudyGuideGenerated(const QString& result);
    void handleLLMResponse(const QString& response);
    void handleLLMError(const QString& error);
    void handleLLMStatus(const QString& status);

private:
    void setupUI();
    void setupConnections();
    void setupStyles();
    void loadPageStyles();
    bool validateInputText();
    void addToHistory(const QString& input, const QString& result);
    void setupMenuBar();
    void createHistoryPage();
    void loadHistory();
    void saveHistory();
    bool initializeLLM();
    QString getMainStyleSheet();
    QString getHeaderStyleSheet();
    QString getHomePageStyleSheet();
    QString getResultsPageStyleSheet();
    QString getHistoryPageStyleSheet();
    void connectSignals();

    // UI Components
    QStackedWidget* stackedWidget;
    HomePage* homePage;
    ResultsPage* resultsPage;
    QWidget* historyPage;
    FlashcardsPage* flashcardsPage;
    QuizPage* quizPage;
    EnumerationsPage* enumerationsPage;
    QStatusBar* statusBar;
    QTextEdit* resultsText;

    // LLM Processing
    LLMProcessor* m_llmProcessor;
    QFutureWatcher<QString>* m_studyGuideWatcher;
    bool isProcessing;
    QString currentInputText;

    // Style handling
    QString currentStyle;

    Ui::MainWindow *ui;
    QListWidget *historyList;

    int currentFlashcardIndex;
    int currentQuizQuestionIndex;
    int quizScore;
    int totalQuestions;

    struct ProcessingHistoryItem {
        QString inputText;
        QString result;
        QDateTime timestamp;
    };
    QList<ProcessingHistoryItem> processingHistory;
};

#endif // MAINWINDOW_H
