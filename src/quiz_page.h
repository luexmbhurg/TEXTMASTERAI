#ifndef QUIZ_PAGE_H
#define QUIZ_PAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVector>
#include <QPair>

class QuizPage : public QWidget {
    Q_OBJECT

public:
    explicit QuizPage(QWidget *parent = nullptr);
    ~QuizPage();

    void setQuestions(const QVector<QPair<QString, QString>>& questions);
    void clearQuestions();

signals:
    void backToHome();
    void answerSubmitted(const QString& answer);

private slots:
    void onSubmitClicked();

private:
    void setupUI();
    void updateQuestionDisplay();
    void updateScore();

    QLabel *questionLabel;
    QLineEdit *answerInput;
    QPushButton *submitButton;
    QLabel *scoreLabel;
    QLabel *feedbackLabel;
    QPushButton *backButton;

    QVector<QPair<QString, QString>> quizQuestions;
    int currentIndex;
    int score;
    int totalQuestions;
};

#endif // QUIZ_PAGE_H 