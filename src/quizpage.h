#ifndef QUIZPAGE_H
#define QUIZPAGE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class QuizPage : public QWidget {
    Q_OBJECT

public:
    explicit QuizPage(QWidget *parent = nullptr);
    ~QuizPage();

    void setQuestion(const QString& question);
    void setAnswer(const QString& answer);
    void setScore(int score, int total);
    void setFeedback(const QString& feedback);
    void showFeedback(bool show);
    QString getAnswer() const;

private:
    QLabel *questionLabel;
    QLineEdit *answerInput;
    QPushButton *submitButton;
    QLabel *scoreLabel;
    QLabel *feedbackLabel;
};

#endif // QUIZPAGE_H 