#include "home_page.h"
#include "ui_homepage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QRegularExpression>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
{
    ui->setupUi(this);
    
    // Connect signals
    connect(ui->textInput, &QTextEdit::textChanged, this, &HomePage::onTextChanged);
    connect(ui->importButton, &QPushButton::clicked, this, &HomePage::onImportClicked);
    connect(ui->letsGoButton, &QPushButton::clicked, this, &HomePage::onAnalyzeClicked);
    connect(ui->studyGuideButton, &QPushButton::clicked, this, &HomePage::onStudyGuideClicked);
    connect(ui->quizButton, &QPushButton::clicked, this, &HomePage::onQuizClicked);
    connect(ui->flashcardsButton, &QPushButton::clicked, this, &HomePage::onFlashcardsClicked);
    connect(ui->enumerationsButton, &QPushButton::clicked, this, &HomePage::onEnumerationsClicked);

    // Set default analysis type
    currentAnalysisType = "study_guide";
    ui->studyGuideButton->setChecked(true);
}

HomePage::~HomePage()
{
    delete ui;
}

QString HomePage::getInputText() const
{
    return ui->textInput->toPlainText();
}

void HomePage::setInputText(const QString& text)
{
    ui->textInput->setPlainText(text);
}

void HomePage::clearInput()
{
    ui->textInput->clear();
}

void HomePage::onTextChanged()
{
    QString text = ui->textInput->toPlainText();
    int wordCount = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    ui->wordCountLabel->setText(QString("Words: %1/1000").arg(wordCount));
    emit wordCountChanged(wordCount);
}

void HomePage::onImportClicked()
{
    emit importFileClicked();
}

void HomePage::onAnalyzeClicked()
{
    if (currentAnalysisType.isEmpty()) {
        // If no analysis type is selected, default to study guide
        currentAnalysisType = "study_guide";
        ui->studyGuideButton->setChecked(true);
    }
    emit analyzeTextClicked();
}

void HomePage::onStudyGuideClicked()
{
    currentAnalysisType = "study_guide";
    ui->studyGuideButton->setChecked(true);
    ui->quizButton->setChecked(false);
    ui->flashcardsButton->setChecked(false);
    ui->enumerationsButton->setChecked(false);
    emit studyGuideClicked();
}

void HomePage::onQuizClicked()
{
    currentAnalysisType = "quiz";
    ui->studyGuideButton->setChecked(false);
    ui->quizButton->setChecked(true);
    ui->flashcardsButton->setChecked(false);
    ui->enumerationsButton->setChecked(false);
    emit quizClicked();
}

void HomePage::onFlashcardsClicked()
{
    currentAnalysisType = "flashcards";
    ui->studyGuideButton->setChecked(false);
    ui->quizButton->setChecked(false);
    ui->flashcardsButton->setChecked(true);
    ui->enumerationsButton->setChecked(false);
    emit flashcardsClicked();
}

void HomePage::onEnumerationsClicked()
{
    currentAnalysisType = "enumerations";
    ui->studyGuideButton->setChecked(false);
    ui->quizButton->setChecked(false);
    ui->flashcardsButton->setChecked(false);
    ui->enumerationsButton->setChecked(true);
    emit enumerationsClicked();
} 