#ifndef RESULTS_PAGE_H
#define RESULTS_PAGE_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

class ResultsPage : public QWidget {
    Q_OBJECT

public:
    explicit ResultsPage(QWidget* parent = nullptr);
    ~ResultsPage() override = default;

    void setResults(const QString& text);
    QString getResults() const;
    void clear();

signals:
    void backToHome();

private:
    QTextEdit* m_resultsText;
    QPushButton* m_backButton;
    QVBoxLayout* m_layout;

    void setupUI();
    void connectSignals();
};

#endif // RESULTS_PAGE_H 