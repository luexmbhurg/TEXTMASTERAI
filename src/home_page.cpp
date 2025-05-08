#include "home_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

HomePage::~HomePage()
{
}

void HomePage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Create input frame
    QFrame *inputFrame = new QFrame(this);
    inputFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputFrame);
    
    // Create text input area
    textInput = new QTextEdit(this);
    textInput->setPlaceholderText("Enter or paste your text here...");
    
    // Create word count label
    wordCountLabel = new QLabel("Words: 0", this);
    wordCountLabel->setAlignment(Qt::AlignRight);
    
    // Create buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    importButton = new QPushButton("Import File", this);
    analyzeButton = new QPushButton("Analyze Text", this);
    flashcardsButton = new QPushButton("Flashcards", this);
    quizButton = new QPushButton("Quiz", this);
    enumerationsButton = new QPushButton("Enumerations", this);
    
    buttonLayout->addWidget(importButton);
    buttonLayout->addWidget(analyzeButton);
    buttonLayout->addWidget(flashcardsButton);
    buttonLayout->addWidget(quizButton);
    buttonLayout->addWidget(enumerationsButton);
    
    // Create mode selection
    modeComboBox = new QComboBox(this);
    modeComboBox->addItem("Study Guide");
    modeComboBox->addItem("Flashcards");
    modeComboBox->addItem("Quiz");
    modeComboBox->addItem("Enumerations");
    
    inputLayout->addWidget(textInput);
    inputLayout->addWidget(wordCountLabel);
    inputLayout->addWidget(modeComboBox);
    inputLayout->addLayout(buttonLayout);
    
    layout->addWidget(inputFrame);
    
    // Connect signals
    connect(textInput, &QTextEdit::textChanged, this, &HomePage::onTextChanged);
    connect(importButton, &QPushButton::clicked, this, &HomePage::onImportClicked);
    connect(analyzeButton, &QPushButton::clicked, this, &HomePage::onAnalyzeClicked);
    connect(flashcardsButton, &QPushButton::clicked, this, &HomePage::onFlashcardsClicked);
    connect(quizButton, &QPushButton::clicked, this, &HomePage::onQuizClicked);
    connect(enumerationsButton, &QPushButton::clicked, this, &HomePage::onEnumerationsClicked);
}

QString HomePage::getInputText() const
{
    return textInput->toPlainText();
}

void HomePage::setInputText(const QString& text)
{
    textInput->setPlainText(text);
}

void HomePage::clearInput()
{
    textInput->clear();
}

void HomePage::onTextChanged()
{
    QString text = textInput->toPlainText();
    int wordCount = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    wordCountLabel->setText(QString("Words: %1").arg(wordCount));
    emit wordCountChanged(wordCount);
}

void HomePage::onImportClicked()
{
    emit importFileClicked();
}

void HomePage::onAnalyzeClicked()
{
    emit analyzeTextClicked();
}

void HomePage::onFlashcardsClicked()
{
    emit flashcardsClicked();
}

void HomePage::onQuizClicked()
{
    emit quizClicked();
}

void HomePage::onEnumerationsClicked()
{
    emit enumerationsClicked();
} 