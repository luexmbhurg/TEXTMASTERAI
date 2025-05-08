#include "quiz_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

QuizPage::QuizPage(QWidget *parent)
    : QWidget(parent)
    , currentIndex(0)
    , score(0)
    , totalQuestions(0)
{
    setupUI();
}

QuizPage::~QuizPage()
{
}

void QuizPage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Create quiz display area
    QFrame *quizFrame = new QFrame(this);
    quizFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    QVBoxLayout *quizLayout = new QVBoxLayout(quizFrame);
    
    questionLabel = new QLabel(this);
    questionLabel->setWordWrap(true);
    questionLabel->setStyleSheet("font-size: 16px;");
    
    answerInput = new QLineEdit(this);
    submitButton = new QPushButton("Submit", this);
    
    feedbackLabel = new QLabel(this);
    feedbackLabel->setVisible(false);
    
    scoreLabel = new QLabel("Score: 0/0", this);
    scoreLabel->setAlignment(Qt::AlignRight);
    
    quizLayout->addWidget(questionLabel);
    quizLayout->addWidget(answerInput);
    quizLayout->addWidget(submitButton);
    quizLayout->addWidget(feedbackLabel);
    quizLayout->addWidget(scoreLabel);
    
    // Add back button
    backButton = new QPushButton("Back to Home", this);
    
    layout->addWidget(quizFrame);
    layout->addWidget(backButton);
    
    // Connect signals
    connect(submitButton, &QPushButton::clicked, this, &QuizPage::onSubmitClicked);
    connect(backButton, &QPushButton::clicked, this, &QuizPage::backToHome);
}

void QuizPage::setQuestions(const QVector<QPair<QString, QString>>& questions)
{
    quizQuestions = questions;
    currentIndex = 0;
    score = 0;
    totalQuestions = questions.size();
    updateQuestionDisplay();
    updateScore();
}

void QuizPage::clearQuestions()
{
    quizQuestions.clear();
    currentIndex = 0;
    score = 0;
    totalQuestions = 0;
    questionLabel->setText("");
    answerInput->clear();
    feedbackLabel->setText("");
    feedbackLabel->setVisible(false);
    updateScore();
}

void QuizPage::updateQuestionDisplay()
{
    if (quizQuestions.isEmpty()) {
        questionLabel->setText("No questions available");
        answerInput->clear();
        feedbackLabel->setText("");
        feedbackLabel->setVisible(false);
        return;
    }
    
    questionLabel->setText(quizQuestions[currentIndex].first);
    answerInput->clear();
    feedbackLabel->setText("");
    feedbackLabel->setVisible(false);
}

void QuizPage::updateScore()
{
    scoreLabel->setText(QString("Score: %1/%2").arg(score).arg(totalQuestions));
}

void QuizPage::onSubmitClicked()
{
    if (quizQuestions.isEmpty()) {
        return;
    }
    
    QString answer = answerInput->text().trimmed();
    QString correctAnswer = quizQuestions[currentIndex].second;
    
    bool isCorrect = answer.compare(correctAnswer, Qt::CaseInsensitive) == 0;
    if (isCorrect) {
        score++;
        feedbackLabel->setText("Correct!");
        feedbackLabel->setStyleSheet("color: green;");
    } else {
        feedbackLabel->setText(QString("Incorrect. The correct answer is: %1").arg(correctAnswer));
        feedbackLabel->setStyleSheet("color: red;");
    }
    
    feedbackLabel->setVisible(true);
    updateScore();
    
    // Move to next question or end quiz
    if (currentIndex < quizQuestions.size() - 1) {
        currentIndex++;
        updateQuestionDisplay();
    } else {
        submitButton->setEnabled(false);
        answerInput->setEnabled(false);
    }
    
    emit answerSubmitted(answer);
}

QString QuizPage::getAnswer() const
{
    return answerInput->text();
} 