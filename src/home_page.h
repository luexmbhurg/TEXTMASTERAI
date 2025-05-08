#ifndef HOME_PAGE_H
#define HOME_PAGE_H

#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

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
    void flashcardsClicked();
    void quizClicked();
    void enumerationsClicked();
    void wordCountChanged(int count);

private slots:
    void onTextChanged();
    void onImportClicked();
    void onAnalyzeClicked();
    void onFlashcardsClicked();
    void onQuizClicked();
    void onEnumerationsClicked();

private:
    void setupUI();

    QTextEdit *textInput;
    QLabel *wordCountLabel;
    QPushButton *importButton;
    QPushButton *analyzeButton;
    QPushButton *flashcardsButton;
    QPushButton *quizButton;
    QPushButton *enumerationsButton;
    QComboBox *modeComboBox;
};

#endif // HOME_PAGE_H 