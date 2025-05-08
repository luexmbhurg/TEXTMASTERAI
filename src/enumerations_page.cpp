#include "enumerations_page.h"
#include <QVBoxLayout>

EnumerationsPage::EnumerationsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

EnumerationsPage::~EnumerationsPage()
{
}

void EnumerationsPage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Create enumerations list
    enumerationsList = new QListWidget(this);
    enumerationsList->setWordWrap(true);
    enumerationsList->setSpacing(2);
    
    // Add back button
    backButton = new QPushButton("Back to Home", this);
    
    layout->addWidget(enumerationsList);
    layout->addWidget(backButton);
    
    // Connect signals
    connect(backButton, &QPushButton::clicked, this, &EnumerationsPage::backToHome);
}

void EnumerationsPage::setEnumerations(const QStringList& items)
{
    enumerationsList->clear();
    for (const QString& item : items) {
        QListWidgetItem *listItem = new QListWidgetItem(item);
        listItem->setFlags(listItem->flags() | Qt::ItemIsSelectable);
        enumerationsList->addItem(listItem);
    }
}

void EnumerationsPage::clearEnumerations()
{
    enumerationsList->clear();
} 