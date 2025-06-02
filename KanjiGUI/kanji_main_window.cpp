#include "kanji_main_window.h"
#include "kanji_learning_window.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QProcess>
#include <QTimer>
#include <QDebug>

KanjiMainWindow::KanjiMainWindow(QWidget *parent)
    : QMainWindow(parent), database(new KanjiDatabase()), learningWindow(nullptr)
{
    // Initialize database
    if (!database->initialize()) {
        QMessageBox::critical(this, "Database Error", 
                            "Failed to initialize database: " + database->getLastError());
        return;
    }
    
    setupUI();
    refreshStatistics();
    
    // Set up timer to refresh statistics every 5 seconds
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &KanjiMainWindow::refreshStatistics);
    refreshTimer->start(5000); // 5 seconds
}

KanjiMainWindow::~KanjiMainWindow()
{
    delete database;
    if (learningWindow) {
        learningWindow->close();
        delete learningWindow;
    }
}

void KanjiMainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    createMenuBar();
    createMainContent();
    
    setWindowTitle("漢字学習 - Kanji Learning System");
    setMinimumSize(800, 600);
    resize(1000, 700);
    
    // Apply modern styling
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f8f9fa;
        }
        QPushButton {
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 15px 25px;
            font-size: 14px;
            font-weight: bold;
            min-height: 20px;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:pressed {
            background-color: #004085;
        }
        QFrame {
            background-color: white;
            border: 1px solid #dee2e6;
            border-radius: 10px;
            padding: 20px;
        }
        QLabel {
            color: #333333;
        }
        QProgressBar {
            border: 2px solid #dee2e6;
            border-radius: 5px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #28a745;
            border-radius: 3px;
        }
    )");
}

void KanjiMainWindow::createMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu("&File");
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Study menu
    QMenu *studyMenu = menuBar->addMenu("&Study");
    
    QAction *learnAction = studyMenu->addAction("&Learn New Kanji");
    connect(learnAction, &QAction::triggered, this, &KanjiMainWindow::onLearnNewKanji);
    
    QAction *reviewAction = studyMenu->addAction("&Review Kanji");
    connect(reviewAction, &QAction::triggered, this, &KanjiMainWindow::onReviewKanji);
    
    // View menu
    QMenu *viewMenu = menuBar->addMenu("&View");
    
    QAction *statsAction = viewMenu->addAction("&Statistics");
    connect(statsAction, &QAction::triggered, this, &KanjiMainWindow::onViewStatistics);
    
    QAction *refreshAction = viewMenu->addAction("&Refresh");
    connect(refreshAction, &QAction::triggered, this, &KanjiMainWindow::refreshStatistics);
    
    // Test menu for debugging
    QMenu *testMenu = menuBar->addMenu("&Test");
    
    QAction *addReviewAction = testMenu->addAction("Add Test Reviews");
    connect(addReviewAction, &QAction::triggered, [this]() {
        // Reset first to clean state
        database->resetAllKanjiToUnlearned();
        
        // Add kanji to review queue with immediate review times
        QList<KanjiCard> allKanji = database->getAllKanji();
        int count = 0;
        
        for (const KanjiCard &kanji : allKanji) {
            if (count >= 3) break;
            // Mark as learned and set review time to NOW (0 seconds)
            database->updateKanjiProgress(kanji.id, true, 1);
            database->setImmediateReviewTime(kanji.id, 0); // Due NOW
            count++;
        }
        
        // Debug what we have
        database->debugShowAllLearnedKanji();
        int reviewCount = database->getReviewDueCount();
        
        refreshStatistics();
        QMessageBox::information(this, "Test", QString("Added %1 kanji to review queue!\nReview count: %2").arg(count).arg(reviewCount));
    });
    
    QAction *resetAction = testMenu->addAction("Reset All Kanji");
    connect(resetAction, &QAction::triggered, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Reset Database", 
            "This will reset ALL kanji back to unlearned state.\nAre you sure?",
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::Yes) {
            database->resetAllKanjiToUnlearned();
            refreshStatistics();
            QMessageBox::information(this, "Reset Complete", "All kanji have been reset to unlearned state.");
        }
    });
    
    QAction *debugAction = testMenu->addAction("Debug: Show All Learned Kanji");
    connect(debugAction, &QAction::triggered, [this]() {
        database->debugShowAllLearnedKanji();
        QMessageBox::information(this, "Debug", "Check console output for learned kanji details.");
    });
    
    QAction *testFlowAction = testMenu->addAction("Test: Complete SRS Flow");
    connect(testFlowAction, &QAction::triggered, [this]() {
        // Reset database first
        database->resetAllKanjiToUnlearned();
        
        // Learn a few kanji with immediate review times
        QList<KanjiCard> allKanji = database->getAllKanji();
        int count = 0;
        QDateTime now = QDateTime::currentDateTime();
        
        for (const KanjiCard &kanji : allKanji) {
            if (count >= 3) break;
            if (!kanji.is_learned) {
                qDebug() << "Learning kanji:" << kanji.kanji;
                // Mark as learned (this should set SRS level 1 and 10 second review time)
                database->updateKanjiProgress(kanji.id, true, 1);
                count++;
            }
        }
        
        // Show what we have now
        database->debugShowAllLearnedKanji();
        
        // Check review count
        int reviewCount = database->getReviewDueCount();
        
        refreshStatistics();
        QMessageBox::information(this, "Test Complete", 
            QString("Learned %1 kanji.\nReview count: %2\nCheck console for details.").arg(count).arg(reviewCount));
    });
}

void KanjiMainWindow::createMainContent()
{
    topLayout = new QHBoxLayout();
    topLayout->setSpacing(30);
    
    // Left side - Main actions
    leftLayout = new QVBoxLayout();
    
    // Welcome section
    welcomeLabel = new QLabel("Welcome to Kanji Learning System", this);
    welcomeLabel->setFont(QFont("Arial", 24, QFont::Bold));
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    
    infoLabel = new QLabel("Choose an option below to start your Japanese learning journey!", this);
    infoLabel->setFont(QFont("Arial", 12));
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("color: #6c757d; margin-bottom: 30px;");
    
    leftLayout->addWidget(welcomeLabel);
    leftLayout->addWidget(infoLabel);
    
    // Main action buttons
    learnNewButton = new QPushButton("Learn New Kanji", this);
    learnNewButton->setFont(QFont("Arial", 16, QFont::Bold));
    learnNewButton->setMinimumHeight(60);
    learnNewButton->setStyleSheet("QPushButton { background-color: #28a745; } QPushButton:hover { background-color: #1e7e34; }");
    connect(learnNewButton, &QPushButton::clicked, this, &KanjiMainWindow::onLearnNewKanji);
    
    reviewButton = new QPushButton("Review Kanji", this);
    reviewButton->setFont(QFont("Arial", 16, QFont::Bold));
    reviewButton->setMinimumHeight(60);
    reviewButton->setStyleSheet("QPushButton { background-color: #ffc107; color: #212529; } QPushButton:hover { background-color: #e0a800; }");
    connect(reviewButton, &QPushButton::clicked, this, &KanjiMainWindow::onReviewKanji);
    
    statisticsButton = new QPushButton("View Statistics", this);
    statisticsButton->setFont(QFont("Arial", 14));
    statisticsButton->setMinimumHeight(50);
    statisticsButton->setStyleSheet("QPushButton { background-color: #6f42c1; } QPushButton:hover { background-color: #5a32a3; }");
    connect(statisticsButton, &QPushButton::clicked, this, &KanjiMainWindow::onViewStatistics);
    
    leftLayout->addWidget(learnNewButton);
    leftLayout->addSpacing(15);
    leftLayout->addWidget(reviewButton);
    leftLayout->addSpacing(25);
    leftLayout->addWidget(statisticsButton);
    
    leftLayout->addStretch();
    
    // Right side - Statistics panel
    createStatisticsPanel();
    
    topLayout->addLayout(leftLayout, 2);
    topLayout->addWidget(statsFrame, 1);
    
    mainLayout->addLayout(topLayout);
    
    // Status bar
    statusBar()->showMessage("Ready to learn Japanese kanji!");
}

void KanjiMainWindow::createStatisticsPanel()
{
    statsFrame = new QFrame(this);
    QVBoxLayout *statsLayout = new QVBoxLayout(statsFrame);
    
    QLabel *statsTitle = new QLabel("Your Progress", statsFrame);
    statsTitle->setFont(QFont("Arial", 16, QFont::Bold));
    statsTitle->setAlignment(Qt::AlignCenter);
    statsTitle->setStyleSheet("color: #495057; margin-bottom: 20px;");
    
    totalKanjiLabel = new QLabel("Total Kanji: Loading...", statsFrame);
    totalKanjiLabel->setFont(QFont("Arial", 12));
    
    learnedKanjiLabel = new QLabel("Learned: Loading...", statsFrame);
    learnedKanjiLabel->setFont(QFont("Arial", 12));
    learnedKanjiLabel->setStyleSheet("color: #28a745;");
    
    newKanjiLabel = new QLabel("New: Loading...", statsFrame);
    newKanjiLabel->setFont(QFont("Arial", 12));
    newKanjiLabel->setStyleSheet("color: #007bff;");
    
    reviewDueLabel = new QLabel("Due for Review: Loading...", statsFrame);
    reviewDueLabel->setFont(QFont("Arial", 12));
    reviewDueLabel->setStyleSheet("color: #ffc107;");
    
    progressLabel = new QLabel("Progress: 0%", statsFrame);
    progressLabel->setFont(QFont("Arial", 12, QFont::Bold));
    progressLabel->setAlignment(Qt::AlignCenter);
    
    progressBar = new QProgressBar(statsFrame);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    
    statsLayout->addWidget(statsTitle);
    statsLayout->addSpacing(10);
    statsLayout->addWidget(totalKanjiLabel);
    statsLayout->addWidget(learnedKanjiLabel);
    statsLayout->addWidget(newKanjiLabel);
    statsLayout->addWidget(reviewDueLabel);
    statsLayout->addSpacing(20);
    statsLayout->addWidget(progressLabel);
    statsLayout->addWidget(progressBar);
    statsLayout->addStretch();
}

void KanjiMainWindow::refreshStatistics()
{
    // Safety check to prevent crashes
    if (!database) {
        qDebug() << "Database not available for statistics refresh";
        return;
    }
    
    int total = database->getTotalKanjiCount();
    int learned = database->getLearnedKanjiCount();
    int newKanji = database->getNewKanjiCount();
    int reviewDue = database->getReviewDueCount();
    
    totalKanjiLabel->setText(QString("Total Kanji: %1").arg(total));
    learnedKanjiLabel->setText(QString("Learned: %1").arg(learned));
    newKanjiLabel->setText(QString("New: %1").arg(newKanji));
    reviewDueLabel->setText(QString("Due for Review: %1").arg(reviewDue));
    
    int progressPercent = total > 0 ? (learned * 100) / total : 0;
    progressLabel->setText(QString("Progress: %1%").arg(progressPercent));
    progressBar->setValue(progressPercent);
    
    // Update button text with counts
    learnNewButton->setText(QString("Learn New Kanji (%1)").arg(newKanji));
    reviewButton->setText(QString("Review Kanji (%1)").arg(reviewDue));
    
    // Enable/disable buttons based on availability
    learnNewButton->setEnabled(newKanji > 0);
    reviewButton->setEnabled(reviewDue > 0);
    
    statusBar()->showMessage(QString("Statistics updated - %1 kanji learned, %2 due for review")
                           .arg(learned).arg(reviewDue));
}

void KanjiMainWindow::onLearnNewKanji()
{
    int newCount = database->getNewKanjiCount();
    if (newCount == 0) {
        QMessageBox::information(this, "No New Kanji", 
                                "Congratulations! You have studied all available kanji.");
        return;
    }
    
    // Close existing learning window if open
    if (learningWindow) {
        learningWindow->close();
        delete learningWindow;
        learningWindow = nullptr;
    }
    
    // Create and show new learning window
    learningWindow = new KanjiLearningWindow(database, KanjiLearningWindow::Mode::Learning, this);
    connect(learningWindow, &QMainWindow::destroyed, this, &KanjiMainWindow::onLearningWindowClosed);
    learningWindow->show();
    learningWindow->raise();
    learningWindow->activateWindow();
}

void KanjiMainWindow::onLearningWindowClosed()
{
    learningWindow = nullptr;
    // Refresh statistics when learning window is closed - but do it safely with a delay
    QTimer::singleShot(100, this, &KanjiMainWindow::refreshStatistics);
}

void KanjiMainWindow::onReviewKanji()
{
    int reviewCount = database->getReviewDueCount();
    int learnedCount = database->getLearnedKanjiCount();
    
    qDebug() << "Review button clicked:";
    qDebug() << "- Learned kanji count:" << learnedCount;
    qDebug() << "- Review due count:" << reviewCount;
    
    // Get detailed info about learned kanji
    QList<KanjiCard> allKanji = database->getAllKanji();
    int learnedWithReviewTime = 0;
    QDateTime now = QDateTime::currentDateTime();
    
    for (const KanjiCard &kanji : allKanji) {
        if (kanji.is_learned) {
            learnedWithReviewTime++;
            qDebug() << "Learned kanji:" << kanji.kanji 
                     << "Level:" << kanji.srs_level 
                     << "Next review:" << kanji.next_review.toString()
                     << "Due?" << (kanji.next_review <= now);
        }
    }
    
    qDebug() << "- Learned kanji with review times:" << learnedWithReviewTime;
    
    if (reviewCount == 0) {
        QString message;
        if (learnedCount == 0) {
            message = "No kanji have been learned yet!\nLearn some kanji first, then come back for reviews.";
        } else {
            message = QString("You have %1 learned kanji, but none are due for review yet.\n"
                             "Wait a bit longer or learn more kanji!").arg(learnedCount);
        }
        
        QMessageBox::information(this, "No Reviews Due", message);
        return;
    }
    
    // Close existing learning window if open
    if (learningWindow) {
        learningWindow->close();
        delete learningWindow;
        learningWindow = nullptr;
    }
    
    // Create and show review window
    learningWindow = new KanjiLearningWindow(database, KanjiLearningWindow::Mode::Review, this);
    connect(learningWindow, &QMainWindow::destroyed, this, &KanjiMainWindow::onLearningWindowClosed);
    learningWindow->show();
    learningWindow->raise();
    learningWindow->activateWindow();
}

void KanjiMainWindow::onViewStatistics()
{
    refreshStatistics();
    
    // Get SRS level breakdown
    QMap<int, int> levelCounts = database->getKanjiCountByLevel();
    
    QString levelBreakdown = "SRS Level Breakdown:\n\n";
    levelBreakdown += QString("Level 0 (Unlearned): %1\n").arg(levelCounts[0]);
    levelBreakdown += QString("Level 1 (10 sec): %1\n").arg(levelCounts[1]);
    levelBreakdown += QString("Level 2 (30 sec): %1\n").arg(levelCounts[2]);
    levelBreakdown += QString("Level 3 (1 min): %1\n").arg(levelCounts[3]);
    levelBreakdown += QString("Level 4 (2 min): %1\n").arg(levelCounts[4]);
    levelBreakdown += QString("Level 5 (5 min): %1\n").arg(levelCounts[5]);
    levelBreakdown += QString("Level 6 (10 min): %1\n").arg(levelCounts[6]);
    levelBreakdown += QString("Level 7 (30 min): %1\n").arg(levelCounts[7]);
    levelBreakdown += QString("Level 8 (1 hour): %1\n\n").arg(levelCounts[8]);
    
    QMessageBox::information(this, "Statistics", 
                            QString("Kanji Learning Statistics\n\n"
                                   "Total Kanji: %1\n"
                                   "Learned: %2 (%3%)\n"
                                   "New: %4\n"
                                   "Due for Review: %5\n\n"
                                   "%6"
                                   "Keep up the great work!")
                           .arg(database->getTotalKanjiCount())
                           .arg(database->getLearnedKanjiCount())
                           .arg(database->getTotalKanjiCount() > 0 ? 
                                (database->getLearnedKanjiCount() * 100) / database->getTotalKanjiCount() : 0)
                           .arg(database->getNewKanjiCount())
                           .arg(database->getReviewDueCount())
                           .arg(levelBreakdown));
}

void KanjiMainWindow::updateStatistics()
{
    refreshStatistics();
} 