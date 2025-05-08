#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QDateTime>
#include "home_page.h"
#include "flashcards_page.h"
#include "quiz_page.h"
#include "enumerations_page.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class LLMProcessor;

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
    void onLLMStatusUpdate(const QString& status);
    void onLLMError(const QString& error);

private:
    void setupUI();
    void setupMenuBar();
    void createHistoryPage();
    bool validateInputText();
    void addToHistory(const QString& inputText, const QString& result);
    void loadHistory();
    void saveHistory();
    bool initializeLLM();
    void setupStyles();
    QString getMainStyleSheet();
    QString getHeaderStyleSheet();
    QString getHomePageStyleSheet();
    QString getResultsPageStyleSheet();
    QString getHistoryPageStyleSheet();

    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    HomePage *homePage;
    FlashcardsPage *flashcardsPage;
    QuizPage *quizPage;
    EnumerationsPage *enumerationsPage;
    QWidget *historyPage;
    QWidget *resultsPage;
    QTextEdit *resultsText;
    QListWidget *historyList;
    QStatusBar *statusBar;
    LLMProcessor *llmProcessor;

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
