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
    emit analyzeTextClicked();
} 