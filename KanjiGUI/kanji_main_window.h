#ifndef KANJI_MAIN_WINDOW_H
#define KANJI_MAIN_WINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QFrame>
#include <QFont>
#include <QTimer>
#include "kanji_database.h"

// Forward declaration
class KanjiLearningWindow;

class KanjiMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    KanjiMainWindow(QWidget *parent = nullptr);
    ~KanjiMainWindow();

private slots:
    void onLearnNewKanji();
    void onReviewKanji();
    void onViewStatistics();
    void updateStatistics();
    void onLearningWindowClosed();

private:
    void setupUI();
    void createMenuBar();
    void createMainContent();
    void createStatisticsPanel();
    void refreshStatistics();
    
    // UI Components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *topLayout;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    
    // Main action buttons
    QPushButton *learnNewButton;
    QPushButton *reviewButton;
    QPushButton *statisticsButton;
    
    // Statistics display
    QFrame *statsFrame;
    QLabel *totalKanjiLabel;
    QLabel *learnedKanjiLabel;
    QLabel *reviewDueLabel;
    QLabel *newKanjiLabel;
    QLabel *progressLabel;
    QProgressBar *progressBar;
    
    // Welcome/Info panel
    QLabel *welcomeLabel;
    QLabel *infoLabel;
    
    // Database and windows
    KanjiDatabase *database;
    QTimer *refreshTimer;
    KanjiLearningWindow *learningWindow;
};

#endif // KANJI_MAIN_WINDOW_H 