#ifndef FLASHCARDSPAGE_H
#define FLASHCARDSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class FlashcardsPage : public QWidget {
    Q_OBJECT

public:
    explicit FlashcardsPage(QWidget *parent = nullptr);
    ~FlashcardsPage();

    void setTerm(const QString& term);
    void setDefinition(const QString& definition);
    void showDefinition(bool show);

private:
    QLabel *termLabel;
    QLabel *definitionLabel;
    QPushButton *revealButton;
    QPushButton *prevButton;
    QPushButton *nextButton;
};

#endif // FLASHCARDSPAGE_H 