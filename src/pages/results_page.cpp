#include "results_page.h"
#include <QClipboard>
#include <QApplication>

ResultsPage::ResultsPage(QWidget* parent)
    : QWidget(parent)
    , m_resultsText(new QTextEdit(this))
    , m_backButton(new QPushButton("Back to Home", this))
    , m_layout(new QVBoxLayout(this))
{
    setupUI();
    connectSignals();
}

void ResultsPage::setupUI() {
    // Configure results text display
    m_resultsText->setReadOnly(true);
    m_resultsText->setFont(QFont("Consolas", 10));
    m_resultsText->setLineWrapMode(QTextEdit::WidgetWidth);
    
    // Configure back button
    m_backButton->setFixedHeight(40);
    m_backButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
    );

    // Setup layout
    m_layout->addWidget(m_resultsText, 1);
    m_layout->addWidget(m_backButton);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(10);
    
    setLayout(m_layout);
}

void ResultsPage::connectSignals() {
    connect(m_backButton, &QPushButton::clicked, this, &ResultsPage::backToHome);
}

void ResultsPage::setResults(const QString& text) {
    m_resultsText->setText(text);
}

QString ResultsPage::getResults() const {
    return m_resultsText->toPlainText();
}

void ResultsPage::clear() {
    m_resultsText->clear();
} 