#ifndef FLASHCARDS_PAGE_H
#define FLASHCARDS_PAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include <QPair>

class FlashcardsPage : public QWidget {
    Q_OBJECT

public:
    explicit FlashcardsPage(QWidget *parent = nullptr);
    ~FlashcardsPage();

    void setFlashcards(const QVector<QPair<QString, QString>>& cards);
    void clearFlashcards();
    void revealDefinition();
    void prevCard();
    void nextCard();

signals:
    void backToHome();

private slots:
    void onRevealClicked();
    void onPrevClicked();
    void onNextClicked();

private:
    void setupUI();
    void updateCardDisplay();

    QLabel *termLabel;
    QLabel *definitionLabel;
    QPushButton *revealButton;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QPushButton *backButton;

    QVector<QPair<QString, QString>> flashcards;
    int currentIndex;
};

#endif // FLASHCARDS_PAGE_H 