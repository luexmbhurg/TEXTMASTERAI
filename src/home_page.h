#ifndef HOME_PAGE_H
#define HOME_PAGE_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

namespace Ui {
class HomePage;
}

class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

    QString getInputText() const;
    void setInputText(const QString& text);
    void clearInput();

signals:
    void importFileClicked();
    void analyzeTextClicked();
    void wordCountChanged(int count);
    void studyGuideClicked();
    void quizClicked();
    void flashcardsClicked();
    void enumerationsClicked();

private slots:
    void onTextChanged();
    void onImportClicked();
    void onAnalyzeClicked();
    void onStudyGuideClicked();
    void onQuizClicked();
    void onFlashcardsClicked();
    void onEnumerationsClicked();

private:
    Ui::HomePage *ui;
    QString currentAnalysisType;
};

#endif // HOME_PAGE_H 