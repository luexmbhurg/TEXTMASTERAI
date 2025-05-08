#ifndef ENUMERATIONSPAGE_H
#define ENUMERATIONSPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>

class EnumerationsPage : public QWidget {
    Q_OBJECT

public:
    explicit EnumerationsPage(QWidget *parent = nullptr);
    ~EnumerationsPage();

    void setItems(const QStringList& items);
    void clearItems();

private:
    QListWidget *enumerationsList;
};

#endif // ENUMERATIONSPAGE_H 