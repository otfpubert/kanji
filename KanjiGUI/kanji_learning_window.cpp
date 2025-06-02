#include "kanji_learning_window.h"
#include <japanese_text_utils.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

KanjiLearningWindow::KanjiLearningWindow(KanjiDatabase *db, Mode mode, QWidget *parent)
    : QMainWindow(parent), database(db), currentMode(mode), currentKanjiIndex(0), 
      currentQuizIndex(0), currentQuizType(QuizType::Meaning),
      questionAnsweredCorrectly(false), retryCount(0), isConverting(false)
{
    try {
        setupUI();
        
        if (currentMode == Mode::Learning) {
            loadKanjiForLearning();
            if (!studyKanji.isEmpty()) {
                displayCurrentKanji();
                switchToStudyMode();
            } else {
                QMessageBox::information(this, "No New Kanji", "No new kanji available for learning.");
            }
        } else if (currentMode == Mode::Review) {
            loadKanjiForReview();
            if (!studyKanji.isEmpty()) {
                // For review mode, go directly to quiz - no need to study first
                displayCurrentKanji();
                
                // Initialize quiz state
                currentQuizIndex = 0;
                quizResults.clear();
                processedKanjiIds.clear(); // Clear tracking for review mode
                for (int i = 0; i < studyKanji.size() * 2; ++i) {
                    quizResults.append(false);
                }
                
                switchToQuizMode();
                startNextQuizQuestion();
            } else {
                QMessageBox::information(this, "No Reviews", "No kanji are due for review at this time.");
            }
        }
        
        connect(answerLineEdit, &QLineEdit::textChanged, this, &KanjiLearningWindow::onAnswerTextChanged);
        
        // Set focus to answer input
        answerLineEdit->setFocus();
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", 
            QString("Failed to initialize Kanji Learning Window: %1").arg(e.what()));
        qDebug() << "Exception in KanjiLearningWindow constructor:" << e.what();
        close();
        
    } catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error occurred while initializing Kanji Learning Window");
        qDebug() << "Unknown exception in KanjiLearningWindow constructor";
        close();
    }
}

KanjiLearningWindow::~KanjiLearningWindow()
{
}

void KanjiLearningWindow::setupUI()
{
    // Set window properties
    setWindowTitle("Learn New Kanji");
    setFixedSize(1400, 1000);  // Even bigger for more kanji space
    
    // Create central widget
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Main layout
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    createStudyInterface();
    createQuizInterface();
    
    // Much darker, more visible styling
    setStyleSheet(R"(
        QWidget {
            background-color: #f8f9fa;
            font-family: Arial;
        }
        QPushButton {
            background-color: #007bff;
            color: white;
            border: none;
            padding: 12px 24px;
            font-size: 16px;
            font-weight: bold;
            border-radius: 6px;
            min-height: 40px;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:disabled {
            background-color: #6c757d;
            color: #ffffff;
        }
        QLineEdit {
            background-color: white;
            border: 3px solid #007bff;
            padding: 12px;
            font-size: 18px;
            font-weight: bold;
            border-radius: 6px;
            color: #000000;
        }
        QLabel {
            color: #000000;
            font-weight: bold;
        }
    )");
}

void KanjiLearningWindow::createStudyInterface()
{
    studyWidget = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(studyWidget);
    layout->setContentsMargins(60, 60, 60, 60);
    layout->setSpacing(40);
    
    // Title - different for learning vs review
    QString titleText = (currentMode == Mode::Learning) ? "Study Mode" : "Review Mode";
    titleLabel = new QLabel(titleText);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #000000; background: white; padding: 25px; border-radius: 8px; border: 2px solid #007bff;");
    layout->addWidget(titleLabel);
    
    // Progress
    progressLabel = new QLabel("Kanji 1 of 5");
    progressLabel->setAlignment(Qt::AlignCenter);
    progressLabel->setStyleSheet("font-size: 18px; color: #000000; font-weight: bold; padding: 15px;");
    layout->addWidget(progressLabel);
    
    // Main content area with much more space
    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background: white; border-radius: 12px; padding: 80px; border: 2px solid #dee2e6;");
    
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(50);
    
    // Much larger kanji with even more space
    kanjiCharLabel = new QLabel("一");
    kanjiCharLabel->setAlignment(Qt::AlignCenter);
    kanjiCharLabel->setStyleSheet("font-size: 240px; font-weight: bold; color: #000000; min-height: 350px; padding: 20px 50px 50px 50px;");
    contentLayout->addWidget(kanjiCharLabel);
    
    // Meaning with darker text
    meaningLabel = new QLabel("Meaning: one");
    meaningLabel->setAlignment(Qt::AlignCenter);
    meaningLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #000000; background: #f8d7da; padding: 15px; border-radius: 6px; border: 2px solid #e74c3c;");
    contentLayout->addWidget(meaningLabel);
    
    // Reading with darker text
    readingLabel = new QLabel("Reading: いち");
    readingLabel->setAlignment(Qt::AlignCenter);
    readingLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #000000; background: #d1ecf1; padding: 15px; border-radius: 6px; border: 2px solid #3498db;");
    contentLayout->addWidget(readingLabel);
    
    layout->addWidget(contentWidget);
    
    // Buttons with better spacing
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);
    
    backToMainButton = new QPushButton("← Back");
    backToMainButton->setStyleSheet("background-color: #6c757d; font-size: 16px;");
    
    previousButton = new QPushButton("Previous");
    previousButton->setStyleSheet("background-color: #ffc107; color: #000000; font-size: 16px;");
    
    nextButton = new QPushButton("Next");
    nextButton->setStyleSheet("background-color: #28a745; font-size: 16px;");
    
    // Different button text for learning vs review
    QString startButtonText = (currentMode == Mode::Learning) ? "Start Quiz" : "Start Review";
    startQuizButton = new QPushButton(startButtonText);
    startQuizButton->setStyleSheet("background-color: #dc3545; font-size: 18px; min-width: 150px;");
    
    connect(backToMainButton, &QPushButton::clicked, this, &KanjiLearningWindow::onBackToMain);
    connect(previousButton, &QPushButton::clicked, this, &KanjiLearningWindow::onPreviousKanji);
    connect(nextButton, &QPushButton::clicked, this, &KanjiLearningWindow::onNextKanji);
    connect(startQuizButton, &QPushButton::clicked, this, &KanjiLearningWindow::onStartQuiz);
    
    buttonLayout->addWidget(backToMainButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(previousButton);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(startQuizButton);
    
    layout->addLayout(buttonLayout);
    
    mainLayout->addWidget(studyWidget);
}

void KanjiLearningWindow::createQuizInterface()
{
    quizWidget = new QWidget();
    quizWidget->setVisible(false);
    
    QVBoxLayout *layout = new QVBoxLayout(quizWidget);
    layout->setContentsMargins(60, 60, 60, 60);
    layout->setSpacing(30);
    
    // Title and progress with darker text - different for learning vs review
    QString quizTitleText = (currentMode == Mode::Learning) ? "Quiz Mode" : "Review Mode";
    quizTitleLabel = new QLabel(quizTitleText);
    quizTitleLabel->setAlignment(Qt::AlignCenter);
    quizTitleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #000000; background: white; padding: 25px; border-radius: 8px; border: 2px solid #007bff;");
    layout->addWidget(quizTitleLabel);
    
    quizProgressLabel = new QLabel("Question 1 of 10");
    quizProgressLabel->setAlignment(Qt::AlignCenter);
    quizProgressLabel->setStyleSheet("font-size: 18px; color: #000000; font-weight: bold; padding: 15px;");
    layout->addWidget(quizProgressLabel);
    
    quizProgressBar = new QProgressBar();
    quizProgressBar->setRange(0, 10);
    quizProgressBar->setValue(1);
    quizProgressBar->setFixedHeight(25);
    quizProgressBar->setStyleSheet("QProgressBar { border: 2px solid #007bff; border-radius: 6px; background: white; font-size: 14px; font-weight: bold; color: #000000; } QProgressBar::chunk { background-color: #28a745; border-radius: 4px; }");
    layout->addWidget(quizProgressBar);
    
    // Main content area with horizontal layout
    QWidget *questionWidget = new QWidget();
    questionWidget->setStyleSheet("background: white; border-radius: 12px; padding: 40px; border: 2px solid #dee2e6;");
    
    QHBoxLayout *questionLayout = new QHBoxLayout(questionWidget);
    questionLayout->setSpacing(60);
    
    // Left side - Kanji display (much bigger now!)
    QWidget *kanjiWidget = new QWidget();
    kanjiWidget->setStyleSheet("background: #f8f9fa; border-radius: 8px; padding: 20px 40px 40px 40px; min-width: 500px;");
    QVBoxLayout *kanjiLayout = new QVBoxLayout(kanjiWidget);
    kanjiLayout->setSpacing(20);
    
    quizKanjiLabel = new QLabel("一");
    quizKanjiLabel->setAlignment(Qt::AlignCenter);
    quizKanjiLabel->setStyleSheet("font-size: 300px; font-weight: bold; color: #000000; min-height: 400px;");
    kanjiLayout->addWidget(quizKanjiLabel);
    
    questionLayout->addWidget(kanjiWidget);
    
    // Right side - Question and input
    QWidget *inputWidget = new QWidget();
    inputWidget->setStyleSheet("background: #f8f9fa; border-radius: 8px; padding: 40px; min-width: 400px;");
    QVBoxLayout *inputLayout = new QVBoxLayout(inputWidget);
    inputLayout->setSpacing(40);
    
    quizQuestionLabel = new QLabel("What is the meaning of this kanji?");
    quizQuestionLabel->setAlignment(Qt::AlignCenter);
    quizQuestionLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #000000; padding: 20px; background: white; border-radius: 8px;");
    quizQuestionLabel->setWordWrap(true);
    inputLayout->addWidget(quizQuestionLabel);
    
    answerLineEdit = new QLineEdit();
    answerLineEdit->setPlaceholderText("Type your answer here...");
    answerLineEdit->setAlignment(Qt::AlignCenter);
    answerLineEdit->setStyleSheet("min-height: 80px; font-size: 24px; font-weight: bold; background: white; border: 3px solid #007bff; border-radius: 8px; padding: 15px;");
    inputLayout->addWidget(answerLineEdit);
    
    // Add some instruction text
    QLabel *instructionLabel = new QLabel("Press Enter to submit your answer");
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("font-size: 16px; color: #6c757d; font-style: italic; padding: 10px;");
    inputLayout->addWidget(instructionLabel);
    
    inputLayout->addStretch(); // Push everything to top
    
    questionLayout->addWidget(inputWidget);
    
    layout->addWidget(questionWidget);
    
    // Remove the old feedback label and buttons - we'll use overlay instead
    feedbackLabel = new QLabel("");
    feedbackLabel->setVisible(false);  // Hide it, we'll use overlay
    
    retryButton = new QPushButton("Try Again");
    retryButton->setStyleSheet("background-color: #dc3545; min-height: 50px; font-size: 18px;");
    retryButton->setVisible(false);
    layout->addWidget(retryButton);
    
    connect(answerLineEdit, &QLineEdit::returnPressed, this, &KanjiLearningWindow::onAnswerSubmitted);
    connect(answerLineEdit, &QLineEdit::textChanged, this, &KanjiLearningWindow::onAnswerTextChanged);
    connect(retryButton, &QPushButton::clicked, this, &KanjiLearningWindow::onRetryQuestion);
    
    mainLayout->addWidget(quizWidget);
}

void KanjiLearningWindow::onAnswerTextChanged(const QString &text)
{
    if (isConverting || text.isEmpty()) {
        return;
    }
    
    // Only convert for reading questions
    if (currentQuizType != QuizType::Reading) {
        return;
    }
    
    // Check if text contains uppercase letters
    bool hasUppercase = false;
    for (const QChar &ch : text) {
        if (ch.isLetter() && ch.isUpper()) {
            hasUppercase = true;
            break;
        }
    }
    
    if (hasUppercase) {
        QString converted = ::convertRomajiToHiragana(text);
        if (converted != text) {
            int cursorPos = answerLineEdit->cursorPosition();
            isConverting = true;
            answerLineEdit->setText(converted);
            answerLineEdit->setCursorPosition(qMin(cursorPos, converted.length()));
            isConverting = false;
        }
    }
}

void KanjiLearningWindow::loadKanjiForLearning()
{
    studyKanji = database->getNewKanji(5);
    currentKanjiIndex = 0;
}

void KanjiLearningWindow::displayCurrentKanji()
{
    if (studyKanji.isEmpty() || currentKanjiIndex >= studyKanji.size()) {
        return;
    }
    
    const KanjiCard &kanji = studyKanji[currentKanjiIndex];
    
    kanjiCharLabel->setText(kanji.kanji);
    meaningLabel->setText(QString("Meaning: %1").arg(kanji.meaning));
    
    QString reading = kanji.on_reading.isEmpty() ? kanji.kun_reading : kanji.on_reading;
    if (reading.isEmpty()) reading = "N/A";
    readingLabel->setText(QString("Reading: %1").arg(reading));
    
    progressLabel->setText(QString("Kanji %1 of %2").arg(currentKanjiIndex + 1).arg(studyKanji.size()));
    
    previousButton->setEnabled(currentKanjiIndex > 0);
    nextButton->setEnabled(currentKanjiIndex < studyKanji.size() - 1);
}

void KanjiLearningWindow::onPreviousKanji()
{
    if (currentKanjiIndex > 0) {
        currentKanjiIndex--;
        displayCurrentKanji();
    }
}

void KanjiLearningWindow::onNextKanji()
{
    if (currentKanjiIndex < studyKanji.size() - 1) {
        currentKanjiIndex++;
        displayCurrentKanji();
    }
}

void KanjiLearningWindow::onStartQuiz()
{
    currentQuizIndex = 0;
    quizResults.clear();
    processedKanjiIds.clear(); // Clear tracking for review mode
    for (int i = 0; i < studyKanji.size() * 2; ++i) {
        quizResults.append(false);
    }
    switchToQuizMode();
    startNextQuizQuestion();
}

void KanjiLearningWindow::switchToStudyMode()
{
    studyWidget->setVisible(true);
    quizWidget->setVisible(false);
}

void KanjiLearningWindow::switchToQuizMode()
{
    studyWidget->setVisible(false);
    quizWidget->setVisible(true);
}

void KanjiLearningWindow::startNextQuizQuestion()
{
    if (currentQuizIndex >= studyKanji.size() * 2) {
        completeQuiz();
        return;
    }
    
    int kanjiIndex = currentQuizIndex / 2;
    currentQuizType = (currentQuizIndex % 2 == 0) ? QuizType::Meaning : QuizType::Reading;
    
    const KanjiCard &kanji = studyKanji[kanjiIndex];
    
    quizKanjiLabel->setText(kanji.kanji);
    quizProgressLabel->setText(QString("Question %1 of %2").arg(currentQuizIndex + 1).arg(studyKanji.size() * 2));
    quizProgressBar->setValue(currentQuizIndex + 1);
    
    feedbackLabel->clear();
    answerLineEdit->clear();
    answerLineEdit->setEnabled(true);
    retryButton->setVisible(false);
    
    if (currentQuizType == QuizType::Meaning) {
        quizQuestionLabel->setText("What is the meaning of this kanji?");
        correctAnswer = kanji.meaning;
        answerLineEdit->setPlaceholderText("Type the meaning in English...");
    } else {
        quizQuestionLabel->setText("What is the reading of this kanji?");
        correctAnswer = kanji.on_reading.isEmpty() ? kanji.kun_reading : kanji.on_reading;
        answerLineEdit->setPlaceholderText("Type the reading in hiragana...");
    }
    
    answerLineEdit->setFocus();
}

void KanjiLearningWindow::onAnswerSubmitted()
{
    QString userAnswer = answerLineEdit->text().trimmed();
    if (userAnswer.isEmpty()) {
        showFeedbackOverlay("Warning: Please enter an answer", "#ffc107");
        return;
    }
    checkQuizAnswer();
}

void KanjiLearningWindow::checkQuizAnswer()
{
    QString userAnswer = answerLineEdit->text().trimmed().toLower();
    bool isCorrect = false;
    
    if (currentQuizType == QuizType::Meaning) {
        // Handle multiple meanings separated by "/"
        QStringList possibleAnswers = correctAnswer.toLower().split("/");
        for (const QString &answer : possibleAnswers) {
            if (userAnswer == answer.trimmed()) {
                isCorrect = true;
                break;
            }
        }
    } else {
        const KanjiCard &kanji = studyKanji[currentQuizIndex / 2];
        isCorrect = (userAnswer == kanji.on_reading || userAnswer == kanji.kun_reading);
    }
    
    if (isCorrect) {
        // Ensure we don't go out of bounds
        if (currentQuizIndex < quizResults.size()) {
            quizResults[currentQuizIndex] = true;
        }
        
        showFeedbackOverlay("Correct!", "#28a745");
        
        // For review mode, check if both meaning and reading are now correct for this kanji
        if (currentMode == Mode::Review) {
            int kanjiIndex = currentQuizIndex / 2;
            int meaningIndex = kanjiIndex * 2;     // Meaning question index
            int readingIndex = kanjiIndex * 2 + 1; // Reading question index
            
            // Check if both questions for this kanji are answered correctly
            if (meaningIndex < quizResults.size() && readingIndex < quizResults.size() &&
                quizResults[meaningIndex] && quizResults[readingIndex]) {
                
                // Check if we haven't already processed this kanji
                const KanjiCard &kanji = studyKanji[kanjiIndex];
                
                // Only process if we haven't already leveled up this kanji
                if (!processedKanjiIds.contains(kanji.id)) {
                    // Level up the kanji and update next review time
                    qDebug() << "Both meaning and reading correct for kanji:" << kanji.kanji << "- Leveling up!";
                    database->updateKanjiProgress(kanji.id, true, 1);
                    
                    // Mark this kanji as processed
                    processedKanjiIds.insert(kanji.id);
                }
            }
        }
        
        // Hide retry button and disable input temporarily, then automatically proceed
        retryButton->setVisible(false);
        answerLineEdit->setEnabled(false);
        
        QTimer::singleShot(800, [this]() {
            currentQuizIndex++;
            startNextQuizQuestion();
        });
    } else {
        showFeedbackOverlay(QString("Incorrect\nCorrect answer: %1").arg(correctAnswer), "#dc3545");
        
        // For review mode, wrong answer lowers SRS level immediately
        if (currentMode == Mode::Review) {
            int kanjiIndex = currentQuizIndex / 2;
            const KanjiCard &kanji = studyKanji[kanjiIndex];
            qDebug() << "Wrong answer for kanji:" << kanji.kanji << "- Lowering level!";
            database->updateKanjiProgress(kanji.id, false, 1);
        }
        
        retryButton->setVisible(true);
        answerLineEdit->setEnabled(false);
    }
}

void KanjiLearningWindow::showFeedbackOverlay(const QString &message, const QString &color)
{
    // Create overlay widget
    QWidget *overlay = new QWidget(this);
    overlay->setGeometry(0, 0, width(), height());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.5);");
    
    // Create message box in center
    QLabel *messageLabel = new QLabel(message, overlay);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet(QString(
        "background-color: white; "
        "color: %1; "
        "font-size: 32px; "
        "font-weight: bold; "
        "padding: 40px; "
        "border-radius: 15px; "
        "border: 4px solid %1; "
        "min-width: 400px; "
        "min-height: 200px;"
    ).arg(color));
    
    // Center the message
    int msgWidth = 500;
    int msgHeight = 250;
    messageLabel->setGeometry(
        (width() - msgWidth) / 2,
        (height() - msgHeight) / 2,
        msgWidth,
        msgHeight
    );
    
    overlay->show();
    overlay->raise();
    
    // Auto-hide after 1 second (reduced from 2 seconds)
    QTimer::singleShot(1000, [overlay]() {
        overlay->deleteLater();
    });
}

void KanjiLearningWindow::onRetryQuestion()
{
    // Hide the retry button
    retryButton->setVisible(false);
    
    // Clear and re-enable the answer input
    answerLineEdit->clear();
    answerLineEdit->setEnabled(true);
    answerLineEdit->setFocus();
}

void KanjiLearningWindow::completeQuiz()
{
    bool allCorrect = true;
    for (bool result : quizResults) {
        if (!result) {
            allCorrect = false;
            break;
        }
    }
    
    if (allCorrect) {
        if (currentMode == Mode::Learning) {
            markKanjiAsLearned();
            showFeedbackOverlay(QString("Congratulations!\nYou learned %1 kanji!").arg(studyKanji.size()), "#28a745");
        } else {
            // For review mode, progress is already updated per-answer, just show completion message
            showFeedbackOverlay(QString("Review Complete!\nYou reviewed %1 kanji!").arg(studyKanji.size()), "#28a745");
        }
        
        QTimer::singleShot(3000, [this]() {
            close();
        });
    } else {
        QString message = (currentMode == Mode::Learning) ? 
                         "Some answers were incorrect.\nReview the kanji and try again!" :
                         "Some answers were incorrect.\nReview these kanji again!";
        showFeedbackOverlay(message, "#ffc107");
        
        QTimer::singleShot(3000, [this]() {
            switchToStudyMode();
        });
    }
}

void KanjiLearningWindow::markKanjiAsLearned()
{
    for (const KanjiCard &kanji : studyKanji) {
        database->updateKanjiProgress(kanji.id, true, 1);
    }
}

void KanjiLearningWindow::onBackToMain()
{
    close();
}

void KanjiLearningWindow::loadKanjiForReview()
{
    studyKanji = database->getReviewKanji();
    currentKanjiIndex = 0;
}

void KanjiLearningWindow::updateKanjiReviewProgress()
{
    for (const KanjiCard &kanji : studyKanji) {
        // For reviews, we call updateKanjiProgress which handles SRS level increment
        database->updateKanjiProgress(kanji.id, true, 1);
    }
}

void KanjiLearningWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        // Just close without updating progress - progress is already updated per-answer during quiz
        close();
        return;
    }
    QMainWindow::keyPressEvent(event);
} 