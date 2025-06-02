#include "kanji_learning_window.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QKeyEvent>

KanjiLearningWindow::KanjiLearningWindow(KanjiDatabase *db, Mode mode, QWidget *parent)
    : QMainWindow(parent), database(db), currentMode(mode), currentKanjiIndex(0), 
      currentQuizIndex(0), currentQuizType(QuizType::Meaning),
      questionAnsweredCorrectly(false), retryCount(0), isConverting(false)
{
    try {
        setupUI();
        initializeRomajiMap();
        
        if (currentMode == Mode::Learning) {
            loadKanjiForLearning();
            setWindowTitle("Learn New Kanji");
        } else {
            loadKanjiForReview();
            setWindowTitle("Review Kanji");
        }
        
        if (studyKanji.isEmpty()) {
            QString message = (currentMode == Mode::Learning) ? 
                             "No new kanji available for learning." :
                             "No kanji due for review.";
            QMessageBox::information(this, "No Kanji Available", message);
            close();
            return;
        }
        
        displayCurrentKanji();
        
        // For review mode, skip study interface and go straight to quiz
        if (currentMode == Mode::Review) {
            switchToQuizMode();
            onStartQuiz(); // Start the quiz immediately for review mode
        } else {
            switchToStudyMode();
        }
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Initialization Error", 
                             QString("Failed to initialize learning window: %1").arg(e.what()));
        qDebug() << "Exception in KanjiLearningWindow constructor:" << e.what();
        close();
    }
    catch (...) {
        QMessageBox::critical(this, "Initialization Error", 
                             "Unknown error occurred during window initialization");
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
    
    connect(answerLineEdit, &QLineEdit::returnPressed, this, &KanjiLearningWindow::onSubmitAnswer);
    connect(answerLineEdit, &QLineEdit::textChanged, this, &KanjiLearningWindow::onAnswerTextChanged);
    connect(retryButton, &QPushButton::clicked, this, &KanjiLearningWindow::onRetryQuestion);
    
    mainLayout->addWidget(quizWidget);
}

void KanjiLearningWindow::initializeRomajiMap()
{
    // Basic mappings - require double letters for single vowels
    romajiToHiragana["AA"] = "あ";
    romajiToHiragana["II"] = "い";
    romajiToHiragana["UU"] = "う";
    romajiToHiragana["EE"] = "え";
    romajiToHiragana["OO"] = "お";
    romajiToHiragana["NN"] = "ん";
    
    // Small characters
    romajiToHiragana["XYA"] = "ゃ";  // Small ya
    romajiToHiragana["XYU"] = "ゅ";  // Small yu
    romajiToHiragana["XYO"] = "ょ";  // Small yo
    romajiToHiragana["XTSU"] = "っ"; // Small tsu
    romajiToHiragana["XA"] = "ぁ";   // Small a
    romajiToHiragana["XI"] = "ぃ";   // Small i
    romajiToHiragana["XU"] = "ぅ";   // Small u
    romajiToHiragana["XE"] = "ぇ";   // Small e
    romajiToHiragana["XO"] = "ぉ";   // Small o
    
    // K sounds
    romajiToHiragana["KA"] = "か";
    romajiToHiragana["KI"] = "き";
    romajiToHiragana["KU"] = "く";
    romajiToHiragana["KE"] = "け";
    romajiToHiragana["KO"] = "こ";
    
    // G sounds (dakuten)
    romajiToHiragana["GA"] = "が";
    romajiToHiragana["GI"] = "ぎ";
    romajiToHiragana["GU"] = "ぐ";
    romajiToHiragana["GE"] = "げ";
    romajiToHiragana["GO"] = "ご";
    
    // S sounds
    romajiToHiragana["SA"] = "さ";
    romajiToHiragana["SHI"] = "し";
    romajiToHiragana["SU"] = "す";
    romajiToHiragana["SE"] = "せ";
    romajiToHiragana["SO"] = "そ";
    
    // Z sounds (dakuten)
    romajiToHiragana["ZA"] = "ざ";
    romajiToHiragana["JI"] = "じ";   // JI = じ
    romajiToHiragana["ZI"] = "じ";   // Alternative
    romajiToHiragana["ZU"] = "ず";
    romajiToHiragana["ZE"] = "ぜ";
    romajiToHiragana["ZO"] = "ぞ";
    
    // T sounds
    romajiToHiragana["TA"] = "た";
    romajiToHiragana["CHI"] = "ち";
    romajiToHiragana["TSU"] = "つ";
    romajiToHiragana["TE"] = "て";
    romajiToHiragana["TO"] = "と";
    
    // D sounds (dakuten)
    romajiToHiragana["DA"] = "だ";
    romajiToHiragana["DI"] = "ぢ";
    romajiToHiragana["DU"] = "づ";
    romajiToHiragana["DE"] = "で";
    romajiToHiragana["DO"] = "ど";
    
    // N sounds
    romajiToHiragana["NA"] = "な";
    romajiToHiragana["NI"] = "に";
    romajiToHiragana["NU"] = "ぬ";
    romajiToHiragana["NE"] = "ね";
    romajiToHiragana["NO"] = "の";
    
    // H sounds
    romajiToHiragana["HA"] = "は";
    romajiToHiragana["HI"] = "ひ";
    romajiToHiragana["FU"] = "ふ";
    romajiToHiragana["HE"] = "へ";
    romajiToHiragana["HO"] = "ほ";
    
    // B sounds (dakuten)
    romajiToHiragana["BA"] = "ば";
    romajiToHiragana["BI"] = "び";
    romajiToHiragana["BU"] = "ぶ";
    romajiToHiragana["BE"] = "べ";
    romajiToHiragana["BO"] = "ぼ";
    
    // P sounds (handakuten)
    romajiToHiragana["PA"] = "ぱ";
    romajiToHiragana["PI"] = "ぴ";
    romajiToHiragana["PU"] = "ぷ";
    romajiToHiragana["PE"] = "ぺ";
    romajiToHiragana["PO"] = "ぽ";
    
    // M sounds
    romajiToHiragana["MA"] = "ま";
    romajiToHiragana["MI"] = "み";
    romajiToHiragana["MU"] = "む";
    romajiToHiragana["ME"] = "め";
    romajiToHiragana["MO"] = "も";
    
    // Y sounds
    romajiToHiragana["YA"] = "や";
    romajiToHiragana["YU"] = "ゆ";
    romajiToHiragana["YO"] = "よ";
    
    // R sounds
    romajiToHiragana["RA"] = "ら";
    romajiToHiragana["RI"] = "り";
    romajiToHiragana["RU"] = "る";
    romajiToHiragana["RE"] = "れ";
    romajiToHiragana["RO"] = "ろ";
    
    // W sounds
    romajiToHiragana["WA"] = "わ";
    romajiToHiragana["WI"] = "ゐ";   // Archaic
    romajiToHiragana["WE"] = "ゑ";   // Archaic
    romajiToHiragana["WO"] = "を";
    
    // Combination sounds with Y (3 characters)
    romajiToHiragana["KYA"] = "きゃ";
    romajiToHiragana["KYU"] = "きゅ";
    romajiToHiragana["KYO"] = "きょ";
    
    romajiToHiragana["GYA"] = "ぎゃ";
    romajiToHiragana["GYU"] = "ぎゅ";
    romajiToHiragana["GYO"] = "ぎょ";
    
    romajiToHiragana["SHA"] = "しゃ";
    romajiToHiragana["SHU"] = "しゅ";
    romajiToHiragana["SHO"] = "しょ";
    
    romajiToHiragana["JA"] = "じゃ";   // JA = じゃ
    romajiToHiragana["JU"] = "じゅ";   // JU = じゅ  
    romajiToHiragana["JO"] = "じょ";   // JO = じょ
    romajiToHiragana["ZYA"] = "じゃ";  // Alternative
    romajiToHiragana["ZYU"] = "じゅ";  // Alternative
    romajiToHiragana["ZYO"] = "じょ";  // Alternative
    
    romajiToHiragana["CHA"] = "ちゃ";
    romajiToHiragana["CHU"] = "ちゅ";
    romajiToHiragana["CHO"] = "ちょ";
    
    romajiToHiragana["NYA"] = "にゃ";
    romajiToHiragana["NYU"] = "にゅ";
    romajiToHiragana["NYO"] = "にょ";
    
    romajiToHiragana["HYA"] = "ひゃ";
    romajiToHiragana["HYU"] = "ひゅ";
    romajiToHiragana["HYO"] = "ひょ";
    
    romajiToHiragana["BYA"] = "びゃ";
    romajiToHiragana["BYU"] = "びゅ";
    romajiToHiragana["BYO"] = "びょ";
    
    romajiToHiragana["PYA"] = "ぴゃ";
    romajiToHiragana["PYU"] = "ぴゅ";
    romajiToHiragana["PYO"] = "ぴょ";
    
    romajiToHiragana["MYA"] = "みゃ";
    romajiToHiragana["MYU"] = "みゅ";
    romajiToHiragana["MYO"] = "みょ";
    
    romajiToHiragana["RYA"] = "りゃ";
    romajiToHiragana["RYU"] = "りゅ";
    romajiToHiragana["RYO"] = "りょ";
}

QString KanjiLearningWindow::convertRomajiToHiragana(const QString &romaji)
{
    QString result = "";
    QString upperRomaji = romaji.toUpper();
    
    int i = 0;
    while (i < upperRomaji.length()) {
        bool found = false;
        
        // Try 4-character combinations first (for XTSU)
        if (i + 4 <= upperRomaji.length()) {
            QString four_char = upperRomaji.mid(i, 4);
            if (romajiToHiragana.contains(four_char)) {
                result += romajiToHiragana[four_char];
                i += 4;
                found = true;
            }
        }
        
        // Try 3-character combinations
        if (!found && i + 3 <= upperRomaji.length()) {
            QString three_char = upperRomaji.mid(i, 3);
            if (romajiToHiragana.contains(three_char)) {
                result += romajiToHiragana[three_char];
                i += 3;
                found = true;
            }
        }
        
        // Try 2-character combinations
        if (!found && i + 2 <= upperRomaji.length()) {
            QString two_char = upperRomaji.mid(i, 2);
            if (romajiToHiragana.contains(two_char)) {
                result += romajiToHiragana[two_char];
                i += 2;
                found = true;
            }
        }
        
        // If no match found, keep the original character
        if (!found) {
            result += romaji.mid(i, 1);
            i += 1;
        }
    }
    
    return result;
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
        QString converted = convertRomajiToHiragana(text);
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

void KanjiLearningWindow::onSubmitAnswer()
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
        quizResults[currentQuizIndex] = true;
        showFeedbackOverlay("Correct!", "#28a745");
        
        // For review mode, check if both meaning and reading are now correct for this kanji
        if (currentMode == Mode::Review) {
            int kanjiIndex = currentQuizIndex / 2;
            int meaningIndex = kanjiIndex * 2;     // Meaning question index
            int readingIndex = kanjiIndex * 2 + 1; // Reading question index
            
            // Check if both questions for this kanji are answered correctly
            if (meaningIndex < quizResults.size() && readingIndex < quizResults.size() &&
                quizResults[meaningIndex] && quizResults[readingIndex]) {
                // Both meaning and reading correct - advance SRS level
                const KanjiCard &kanji = studyKanji[kanjiIndex];
                database->updateKanjiProgress(kanji.id, true, 1);
            }
        }
        
        // Disable input temporarily and automatically proceed
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