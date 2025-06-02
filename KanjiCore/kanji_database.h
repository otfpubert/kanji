#ifndef KANJI_DATABASE_H
#define KANJI_DATABASE_H

// DLL Export/Import macros
#ifdef _WIN32
    #ifdef KANJICORE_EXPORTS
        #define KANJICORE_API __declspec(dllexport)
    #else
        #define KANJICORE_API __declspec(dllimport)
    #endif
#else
    #define KANJICORE_API
#endif

#include <QSqlDatabase>
#include <QStandardPaths>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QString>
#include <QList>
#include <QVariant>
#include <QMap>
#include <QDebug>
#include <stdexcept>
#include <exception>

struct KANJICORE_API KanjiCard {
    int id;
    QString kanji;
    QString meaning;
    QString on_reading;    // On'yomi in hiragana
    QString kun_reading;   // Kun'yomi in hiragana
    QString example_word;  // Example word using this kanji
    QString example_reading; // Reading of example word
    QString example_meaning; // Meaning of example word
    int difficulty_level;   // 1-5, where 1 is easiest
    bool is_learned;       // Has user studied this kanji?
    QDateTime last_reviewed;
    QDateTime next_review;
    int srs_level;         // SRS level (1-8)
    int review_count;
};

class KANJICORE_API KanjiDatabase
{
public:
    KanjiDatabase();
    ~KanjiDatabase();

    bool initialize();
    bool createTables();
    bool populateN5Kanji();
    
    // Card operations
    QList<KanjiCard> getNewKanji(int limit = 10);
    QList<KanjiCard> getReviewKanji();
    QList<KanjiCard> getAllKanji();
    KanjiCard getKanjiById(int id);
    bool updateKanjiProgress(int id, bool correct, int difficulty);
    
    // Statistics
    int getTotalKanjiCount();
    int getLearnedKanjiCount();
    int getReviewDueCount();
    int getNewKanjiCount();
    QMap<int, int> getKanjiCountByLevel(); // Get count of kanji at each SRS level
    
    // Testing utilities
    bool setImmediateReviewTime(int id, int secondsFromNow);
    bool resetAllKanjiToUnlearned(); // Reset all kanji to unlearned state
    void debugShowAllLearnedKanji(); // Debug: show all learned kanji and their times
    
    QString getLastError() const { return lastError; }

private:
    QSqlDatabase db;
    QString lastError;
    
    bool executeQuery(const QString &query, const QVariantList &values = QVariantList());
    QString getDatabasePath();
};

#endif // KANJI_DATABASE_H 