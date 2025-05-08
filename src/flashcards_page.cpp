#include "flashcards_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

FlashcardsPage::FlashcardsPage(QWidget *parent)
    : QWidget(parent)
    , currentIndex(0)
{
    setupUI();
}

FlashcardsPage::~FlashcardsPage()
{
}

void FlashcardsPage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Create flashcard display area
    QFrame *cardFrame = new QFrame(this);
    cardFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    QVBoxLayout *cardLayout = new QVBoxLayout(cardFrame);
    
    termLabel = new QLabel(this);
    termLabel->setAlignment(Qt::AlignCenter);
    termLabel->setWordWrap(true);
    termLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    
    definitionLabel = new QLabel(this);
    definitionLabel->setAlignment(Qt::AlignCenter);
    definitionLabel->setWordWrap(true);
    definitionLabel->setVisible(false);
    
    cardLayout->addWidget(termLabel);
    cardLayout->addWidget(definitionLabel);
    
    // Create navigation buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    prevButton = new QPushButton("Previous", this);
    revealButton = new QPushButton("Reveal", this);
    nextButton = new QPushButton("Next", this);
    
    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(revealButton);
    buttonLayout->addWidget(nextButton);
    
    // Add back button
    backButton = new QPushButton("Back to Home", this);
    
    layout->addWidget(cardFrame);
    layout->addLayout(buttonLayout);
    layout->addWidget(backButton);
    
    // Connect signals
    connect(prevButton, &QPushButton::clicked, this, &FlashcardsPage::onPrevClicked);
    connect(revealButton, &QPushButton::clicked, this, &FlashcardsPage::onRevealClicked);
    connect(nextButton, &QPushButton::clicked, this, &FlashcardsPage::onNextClicked);
    connect(backButton, &QPushButton::clicked, this, &FlashcardsPage::backToHome);
}

void FlashcardsPage::setFlashcards(const QVector<QPair<QString, QString>>& cards)
{
    flashcards = cards;
    currentIndex = 0;
    updateCardDisplay();
}

void FlashcardsPage::clearFlashcards()
{
    flashcards.clear();
    currentIndex = 0;
    termLabel->setText("");
    definitionLabel->setText("");
    definitionLabel->setVisible(false);
    prevButton->setEnabled(false);
    nextButton->setEnabled(false);
}

void FlashcardsPage::updateCardDisplay()
{
    if (flashcards.isEmpty()) {
        termLabel->setText("No flashcards available");
        definitionLabel->setText("");
        definitionLabel->setVisible(false);
        prevButton->setEnabled(false);
        nextButton->setEnabled(false);
        return;
    }
    
    termLabel->setText(flashcards[currentIndex].first);
    definitionLabel->setText(flashcards[currentIndex].second);
    definitionLabel->setVisible(false);
    
    prevButton->setEnabled(currentIndex > 0);
    nextButton->setEnabled(currentIndex < flashcards.size() - 1);
}

void FlashcardsPage::onRevealClicked()
{
    if (definitionLabel) {
        definitionLabel->setVisible(!definitionLabel->isVisible());
        revealButton->setText(definitionLabel->isVisible() ? "Hide Definition" : "Show Definition");
    }
}

void FlashcardsPage::onPrevClicked()
{
    if (!flashcards.isEmpty()) {
        currentIndex = (currentIndex - 1 + flashcards.size()) % flashcards.size();
        updateCardDisplay();
    }
}

void FlashcardsPage::onNextClicked()
{
    if (!flashcards.isEmpty()) {
        currentIndex = (currentIndex + 1) % flashcards.size();
        updateCardDisplay();
    }
}

void FlashcardsPage::revealDefinition()
{
    if (definitionLabel) {
        definitionLabel->setVisible(!definitionLabel->isVisible());
        revealButton->setText(definitionLabel->isVisible() ? "Hide Definition" : "Show Definition");
    }
}

void FlashcardsPage::prevCard()
{
    if (!flashcards.isEmpty()) {
        currentIndex = (currentIndex - 1 + flashcards.size()) % flashcards.size();
        updateCardDisplay();
    }
}

void FlashcardsPage::nextCard()
{
    if (!flashcards.isEmpty()) {
        currentIndex = (currentIndex + 1) % flashcards.size();
        updateCardDisplay();
    }
} 