#include "japanese_text_utils.h"
#include <QChar>

// Global instance for convenience function
static JapaneseTextUtils* g_textUtils = nullptr;

JapaneseTextUtils::JapaneseTextUtils()
{
    initializeRomajiMap();
}

void JapaneseTextUtils::initializeRomajiMap()
{
    // Basic vowels (long vowels) - ONLY double letters for single vowels
    romajiToHiragana["AA"] = "あ";
    romajiToHiragana["II"] = "い";
    romajiToHiragana["UU"] = "う";
    romajiToHiragana["EE"] = "え";
    romajiToHiragana["OO"] = "お";
    
    // Special small characters
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
    
    // G sounds
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
    
    // Z sounds
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
    
    // D sounds
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
    romajiToHiragana["NN"] = "ん";
    
    // H sounds
    romajiToHiragana["HA"] = "は";
    romajiToHiragana["HI"] = "ひ";
    romajiToHiragana["FU"] = "ふ";
    romajiToHiragana["HU"] = "ふ";   // Alternative
    romajiToHiragana["HE"] = "へ";
    romajiToHiragana["HO"] = "ほ";
    
    // B sounds
    romajiToHiragana["BA"] = "ば";
    romajiToHiragana["BI"] = "び";
    romajiToHiragana["BU"] = "ぶ";
    romajiToHiragana["BE"] = "べ";
    romajiToHiragana["BO"] = "ぼ";
    
    // P sounds
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

QString JapaneseTextUtils::convertRomajiToHiragana(const QString &romaji)
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
        
        // Try single character
        if (!found && i + 1 <= upperRomaji.length()) {
            QString single_char = upperRomaji.mid(i, 1);
            if (romajiToHiragana.contains(single_char)) {
                result += romajiToHiragana[single_char];
                i += 1;
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

bool JapaneseTextUtils::isHiragana(const QString &text)
{
    for (const QChar &ch : text) {
        ushort unicode = ch.unicode();
        // Hiragana block: U+3040-U+309F
        if (unicode < 0x3040 || unicode > 0x309F) {
            return false;
        }
    }
    return !text.isEmpty();
}

bool JapaneseTextUtils::isKatakana(const QString &text)
{
    for (const QChar &ch : text) {
        ushort unicode = ch.unicode();
        // Katakana block: U+30A0-U+30FF
        if (unicode < 0x30A0 || unicode > 0x30FF) {
            return false;
        }
    }
    return !text.isEmpty();
}

bool JapaneseTextUtils::isKanji(const QString &text)
{
    for (const QChar &ch : text) {
        ushort unicode = ch.unicode();
        // Common kanji blocks: U+4E00-U+9FAF (CJK Unified Ideographs)
        if (unicode < 0x4E00 || unicode > 0x9FAF) {
            return false;
        }
    }
    return !text.isEmpty();
}

// Global convenience function
QString convertRomajiToHiragana(const QString &romaji)
{
    if (!g_textUtils) {
        g_textUtils = new JapaneseTextUtils();
    }
    return g_textUtils->convertRomajiToHiragana(romaji);
} 