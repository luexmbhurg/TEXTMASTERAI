#ifndef HOME_PAGE_H
#define HOME_PAGE_H

#include <QWidget>

namespace Ui {
class HomePage;
}

class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

    QString getInputText() const;
    void setInputText(const QString& text);
    void clearInput();

signals:
    void importFileClicked();
    void analyzeTextClicked();
    void wordCountChanged(int count);

private slots:
    void onTextChanged();
    void onImportClicked();
    void onAnalyzeClicked();

private:
    Ui::HomePage *ui;
};

#endif // HOME_PAGE_H 