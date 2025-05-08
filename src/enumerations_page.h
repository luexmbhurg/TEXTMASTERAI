#ifndef ENUMERATIONS_PAGE_H
#define ENUMERATIONS_PAGE_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>

class EnumerationsPage : public QWidget {
    Q_OBJECT

public:
    explicit EnumerationsPage(QWidget *parent = nullptr);
    ~EnumerationsPage();

    void setEnumerations(const QStringList& items);
    void clearEnumerations();

signals:
    void backToHome();

private:
    void setupUI();

    QListWidget *enumerationsList;
    QPushButton *backButton;
};

#endif // ENUMERATIONS_PAGE_H 