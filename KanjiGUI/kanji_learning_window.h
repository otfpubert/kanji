#ifndef KANJI_LEARNING_WINDOW_H
#define KANJI_LEARNING_WINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QMap>
#include <QList>
#include <QSet>
#include <stdexcept>
#include <exception>
#include <kanji_database.h>

class KanjiLearningWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class Mode {
        Learning,
        Review
    };
    
    explicit KanjiLearningWindow(KanjiDatabase *db, Mode mode = Mode::Learning, QWidget *parent = nullptr);
    ~KanjiLearningWindow();

private slots:
    void onBackToMain();
    void onPreviousKanji();
    void onNextKanji();
    void onStartQuiz();
    void onAnswerSubmitted();
    void onAnswerTextChanged(const QString &text);
    void onRetryQuestion();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUI();
    void createStudyInterface();
    void createQuizInterface();
    void loadKanjiForLearning();
    void loadKanjiForReview();
    void displayCurrentKanji();
    void switchToStudyMode();
    void switchToQuizMode();
    void startNextQuizQuestion();
    void checkQuizAnswer();
    void completeQuiz();
    void markKanjiAsLearned();
    void updateKanjiReviewProgress();
    void showFeedbackOverlay(const QString &message, const QString &color);

    // Database and mode
    KanjiDatabase *database;
    Mode currentMode;
    QList<KanjiCard> studyKanji;
    int currentKanjiIndex;

    // Main layout
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QWidget *studyWidget;
    QWidget *quizWidget;

    // Study interface
    QLabel *titleLabel;
    QLabel *progressLabel;
    QLabel *kanjiCharLabel;
    QLabel *meaningLabel;
    QLabel *readingLabel;
    QPushButton *backToMainButton;
    QPushButton *previousButton;
    QPushButton *nextButton;
    QPushButton *startQuizButton;

    // Quiz interface
    QLabel *quizTitleLabel;
    QLabel *quizProgressLabel;
    QProgressBar *quizProgressBar;
    QLabel *quizKanjiLabel;
    QLabel *quizQuestionLabel;
    QLineEdit *answerLineEdit;
    QPushButton *retryButton;
    QLabel *feedbackLabel;

    // Quiz state
    int currentQuizIndex;
    QString correctAnswer;
    QList<bool> quizResults;
    QSet<int> processedKanjiIds; // Track kanji that have been leveled up in review mode
    bool questionAnsweredCorrectly;
    int retryCount;

    // Conversion state
    bool isConverting = false;

    enum class QuizType {
        Meaning,
        Reading
    };
    QuizType currentQuizType;
};

#endif // KANJI_LEARNING_WINDOW_H 