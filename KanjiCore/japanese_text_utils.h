#ifndef JAPANESE_TEXT_UTILS_H
#define JAPANESE_TEXT_UTILS_H

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

#include <QString>
#include <QMap>

class KANJICORE_API JapaneseTextUtils
{
public:
    JapaneseTextUtils();
    
    // Romaji to Hiragana conversion
    QString convertRomajiToHiragana(const QString &romaji);
    
    // Additional utility functions can be added here later
    bool isHiragana(const QString &text);
    bool isKatakana(const QString &text);
    bool isKanji(const QString &text);
    
private:
    void initializeRomajiMap();
    QMap<QString, QString> romajiToHiragana;
};

// Convenience function for global access
KANJICORE_API QString convertRomajiToHiragana(const QString &romaji);

#endif // JAPANESE_TEXT_UTILS_H 