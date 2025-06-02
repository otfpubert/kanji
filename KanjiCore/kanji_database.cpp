#include "kanji_database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

KanjiDatabase::KanjiDatabase()
{
}

KanjiDatabase::~KanjiDatabase()
{
    if (db.isOpen()) {
        db.close();
    }
}

QString KanjiDatabase::getDatabasePath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    return dataPath + "/kanji_learning.db";
}

bool KanjiDatabase::initialize()
{
    try {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(getDatabasePath());
        
        if (!db.open()) {
            lastError = "Cannot open database: " + db.lastError().text();
            return false;
        }
        
        if (!createTables()) {
            return false;
        }
        
        // Check if we need to populate the database
        QSqlQuery query(db);
        query.prepare("SELECT COUNT(*) FROM kanji");
        if (query.exec() && query.next()) {
            int count = query.value(0).toInt();
            if (count == 0) {
                return populateN5Kanji();
            }
        }
        
        return true;
    }
    catch (const std::exception& e) {
        lastError = QString("Database initialization failed: %1").arg(e.what());
        qDebug() << "Exception in database initialization:" << e.what();
        return false;
    }
    catch (...) {
        lastError = "Unknown error occurred during database initialization";
        qDebug() << "Unknown exception in database initialization";
        return false;
    }
}

bool KanjiDatabase::createTables()
{
    QString createKanjiTable = R"(
        CREATE TABLE IF NOT EXISTS kanji (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            kanji TEXT NOT NULL UNIQUE,
            meaning TEXT NOT NULL,
            on_reading TEXT,
            kun_reading TEXT,
            example_word TEXT,
            example_reading TEXT,
            example_meaning TEXT,
            difficulty_level INTEGER DEFAULT 1,
            is_learned BOOLEAN DEFAULT FALSE,
            last_reviewed DATETIME,
            next_review DATETIME,
            srs_level INTEGER DEFAULT 1,
            review_count INTEGER DEFAULT 0
        )
    )";
    
    if (!executeQuery(createKanjiTable)) {
        return false;
    }
    
    // Add srs_level column if it doesn't exist (for existing databases)
    QString addSrsLevelColumn = "ALTER TABLE kanji ADD COLUMN srs_level INTEGER DEFAULT 1";
    executeQuery(addSrsLevelColumn); // Don't check result - column might already exist
    
    return true;
}

bool KanjiDatabase::populateN5Kanji()
{
    // N5 level kanji with meanings and readings
    QList<QVariantList> kanjiData = {
        // Basic numbers and time
        {"一", "one", "いち", "ひと", "一人", "ひとり", "one person", 1},
        {"二", "two", "に", "ふた", "二人", "ふたり", "two people", 1},
        {"三", "three", "さん", "みっ", "三時", "さんじ", "three o'clock", 1},
        {"四", "four", "し", "よん", "四月", "しがつ", "April", 1},
        {"五", "five", "ご", "いつ", "五時", "ごじ", "five o'clock", 1},
        {"六", "six", "ろく", "むっ", "六月", "ろくがつ", "June", 1},
        {"七", "seven", "しち", "なな", "七時", "しちじ", "seven o'clock", 1},
        {"八", "eight", "はち", "やっ", "八月", "はちがつ", "August", 1},
        {"九", "nine", "きゅう", "ここの", "九時", "くじ", "nine o'clock", 1},
        {"十", "ten", "じゅう", "とお", "十時", "じゅうじ", "ten o'clock", 1},
        
        // Days and time
        {"日", "day/sun", "にち", "ひ", "今日", "きょう", "today", 1},
        {"月", "month/moon", "げつ", "つき", "月曜日", "げつようび", "Monday", 1},
        {"火", "fire/Tuesday", "か", "ひ", "火曜日", "かようび", "Tuesday", 2},
        {"水", "water/Wednesday", "すい", "みず", "水曜日", "すいようび", "Wednesday", 1},
        {"木", "tree/Thursday", "もく", "き", "木曜日", "もくようび", "Thursday", 1},
        {"金", "gold/Friday/money", "きん", "かね", "金曜日", "きんようび", "Friday", 2},
        {"土", "earth/Saturday", "ど", "つち", "土曜日", "どようび", "Saturday", 1},
        {"年", "year", "ねん", "とし", "今年", "ことし", "this year", 1},
        {"時", "time/hour", "じ", "とき", "時間", "じかん", "time", 2},
        
        // People and family
        {"人", "person", "じん", "ひと", "日本人", "にほんじん", "Japanese person", 1},
        {"私", "I/me", "", "わたし", "私達", "わたしたち", "we", 1},
        {"父", "father", "ふ", "ちち", "お父さん", "おとうさん", "father", 2},
        {"母", "mother", "ぼ", "はは", "お母さん", "おかあさん", "mother", 2},
        {"子", "child", "し", "こ", "子供", "こども", "child", 2},
        {"男", "man/male", "だん", "おとこ", "男性", "だんせい", "male", 2},
        {"女", "woman/female", "じょ", "おんな", "女性", "じょせい", "female", 2},
        
        // Basic verbs and adjectives
        {"大", "big", "だい", "おお", "大きい", "おおきい", "big", 1},
        {"小", "small", "しょう", "ちい", "小さい", "ちいさい", "small", 1},
        {"中", "middle/inside", "ちゅう", "なか", "中学校", "ちゅうがっこう", "middle school", 2},
        {"上", "up/above", "じょう", "うえ", "上手", "じょうず", "skillful", 2},
        {"下", "down/below", "か", "した", "下手", "へた", "unskillful", 2},
        {"前", "front/before", "ぜん", "まえ", "午前", "ごぜん", "morning", 2},
        {"後", "back/after", "ご", "うしろ", "午後", "ごご", "afternoon", 2},
        {"右", "right", "う", "みぎ", "右手", "みぎて", "right hand", 2},
        {"左", "left", "さ", "ひだり", "左手", "ひだりて", "left hand", 2},
        
        // Places and directions
        {"国", "country", "こく", "くに", "外国", "がいこく", "foreign country", 2},
        {"家", "house/home", "か", "いえ", "家族", "かぞく", "family", 1},
        {"学", "study/learn", "がく", "まな", "学校", "がっこう", "school", 1},
        {"校", "school", "こう", "", "学校", "がっこう", "school", 1},
        {"先", "previous/ahead", "せん", "さき", "先生", "せんせい", "teacher", 2},
        {"生", "life/birth", "せい", "い", "学生", "がくせい", "student", 1},
        {"東", "east", "とう", "ひがし", "東京", "とうきょう", "Tokyo", 3},
        {"西", "west", "せい", "にし", "関西", "かんさい", "Kansai region", 3},
        {"南", "south", "なん", "みなみ", "南口", "みなみぐち", "south exit", 3},
        {"北", "north", "ほく", "きた", "北海道", "ほっかいどう", "Hokkaido", 3},
        
        // Actions and states
        {"行", "go", "こう", "い", "行く", "いく", "to go", 2},
        {"来", "come", "らい", "く", "来る", "くる", "to come", 2},
        {"見", "see/look", "けん", "み", "見る", "みる", "to see", 1},
        {"聞", "hear/listen", "ぶん", "き", "聞く", "きく", "to hear", 2},
        {"話", "talk/story", "わ", "はなし", "話す", "はなす", "to speak", 2},
        {"読", "read", "どく", "よ", "読む", "よむ", "to read", 2},
        {"書", "write", "しょ", "か", "書く", "かく", "to write", 2},
        {"食", "eat/food", "しょく", "た", "食べる", "たべる", "to eat", 1},
        {"飲", "drink", "いん", "の", "飲む", "のむ", "to drink", 2},
        
        // Transportation and travel
        {"車", "car", "しゃ", "くるま", "電車", "でんしゃ", "train", 1},
        {"電", "electricity", "でん", "", "電話", "でんわ", "telephone", 2},
        {"気", "spirit/feeling", "き", "", "元気", "げんき", "healthy", 2},
        {"元", "origin/source", "げん", "もと", "元気", "げんき", "healthy", 2},
        
        // Money and shopping
        {"円", "yen/circle", "えん", "", "百円", "ひゃくえん", "100 yen", 1},
        {"百", "hundred", "ひゃく", "", "百円", "ひゃくえん", "100 yen", 2},
        {"千", "thousand", "せん", "", "千円", "せんえん", "1000 yen", 2},
        {"万", "ten thousand", "まん", "", "一万円", "いちまんえん", "10,000 yen", 3},
        
        // Colors and descriptions
        {"白", "white", "はく", "しろ", "白い", "しろい", "white", 2},
        {"黒", "black", "こく", "くろ", "黒い", "くろい", "black", 2},
        {"赤", "red", "せき", "あか", "赤い", "あかい", "red", 2},
        {"青", "blue", "せい", "あお", "青い", "あおい", "blue", 2},
        
        // Weather and nature
        {"天", "heaven/sky", "てん", "", "天気", "てんき", "weather", 3},
        {"雨", "rain", "う", "あめ", "雨天", "うてん", "rainy weather", 2},
        {"風", "wind", "ふう", "かぜ", "台風", "たいふう", "typhoon", 3},
        
        // Body and health
        {"手", "hand", "しゅ", "て", "手紙", "てがみ", "letter", 1},
        {"足", "foot/leg", "そく", "あし", "足音", "あしおと", "footstep", 2},
        {"目", "eye", "もく", "め", "目玉", "めだま", "eyeball", 2},
        {"口", "mouth", "こう", "くち", "入口", "いりぐち", "entrance", 1},
        {"耳", "ear", "じ", "みみ", "耳鼻科", "じびか", "ENT clinic", 3},
        
        // Additional useful kanji
        {"出", "exit/come out", "しゅつ", "で", "出る", "でる", "to go out", 2},
        {"入", "enter", "にゅう", "はい", "入る", "はいる", "to enter", 1},
        {"立", "stand", "りつ", "た", "立つ", "たつ", "to stand", 2},
        {"休", "rest", "きゅう", "やす", "休む", "やすむ", "to rest", 2},
        {"何", "what", "なに", "なん", "何時", "なんじ", "what time", 1},
        {"名", "name", "めい", "な", "名前", "なまえ", "name", 1},
        {"今", "now", "こん", "いま", "今日", "きょう", "today", 1},
        {"新", "new", "しん", "あたら", "新しい", "あたらしい", "new", 2},
        {"古", "old", "こ", "ふる", "古い", "ふるい", "old", 2}
    };
    
    QString insertQuery = R"(
        INSERT INTO kanji (kanji, meaning, on_reading, kun_reading, example_word, 
                          example_reading, example_meaning, difficulty_level)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    QSqlQuery query(db);
    query.prepare(insertQuery);
    
    for (const auto& data : kanjiData) {
        for (int i = 0; i < data.size(); ++i) {
            query.addBindValue(data[i]);
        }
        
        if (!query.exec()) {
            lastError = "Failed to insert kanji: " + query.lastError().text();
            return false;
        }
    }
    
    return true;
}

bool KanjiDatabase::executeQuery(const QString &queryString, const QVariantList &values)
{
    QSqlQuery query(db);
    query.prepare(queryString);
    
    for (const auto& value : values) {
        query.addBindValue(value);
    }
    
    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    
    return true;
}

QList<KanjiCard> KanjiDatabase::getNewKanji(int limit)
{
    QList<KanjiCard> cards;
    QSqlQuery query(db);
    query.prepare("SELECT * FROM kanji WHERE is_learned = FALSE LIMIT ?");
    query.addBindValue(limit);
    
    if (query.exec()) {
        while (query.next()) {
            KanjiCard card;
            card.id = query.value("id").toInt();
            card.kanji = query.value("kanji").toString();
            card.meaning = query.value("meaning").toString();
            card.on_reading = query.value("on_reading").toString();
            card.kun_reading = query.value("kun_reading").toString();
            card.example_word = query.value("example_word").toString();
            card.example_reading = query.value("example_reading").toString();
            card.example_meaning = query.value("example_meaning").toString();
            card.difficulty_level = query.value("difficulty_level").toInt();
            card.is_learned = query.value("is_learned").toBool();
            card.srs_level = query.value("srs_level").toInt();
            card.review_count = query.value("review_count").toInt();
            cards.append(card);
        }
    }
    
    return cards;
}

QList<KanjiCard> KanjiDatabase::getReviewKanji()
{
    QList<KanjiCard> cards;
    QSqlQuery query(db);
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("SELECT * FROM kanji WHERE is_learned = TRUE AND next_review <= ? ORDER BY next_review");
    query.addBindValue(now);
    
    qDebug() << "getReviewKanji: Current time is" << now.toString();
    qDebug() << "getReviewKanji: Looking for learned kanji with next_review <=" << now.toString();
    
    if (query.exec()) {
        while (query.next()) {
            KanjiCard card;
            card.id = query.value("id").toInt();
            card.kanji = query.value("kanji").toString();
            card.meaning = query.value("meaning").toString();
            card.on_reading = query.value("on_reading").toString();
            card.kun_reading = query.value("kun_reading").toString();
            card.example_word = query.value("example_word").toString();
            card.example_reading = query.value("example_reading").toString();
            card.example_meaning = query.value("example_meaning").toString();
            card.difficulty_level = query.value("difficulty_level").toInt();
            card.is_learned = query.value("is_learned").toBool();
            card.last_reviewed = query.value("last_reviewed").toDateTime();
            card.next_review = query.value("next_review").toDateTime();
            card.srs_level = query.value("srs_level").toInt();
            card.review_count = query.value("review_count").toInt();
            cards.append(card);
            
            qDebug() << "getReviewKanji: Found kanji" << card.kanji 
                     << "level" << card.srs_level 
                     << "next_review" << card.next_review.toString()
                     << "is_learned" << card.is_learned;
        }
    }
    
    qDebug() << "getReviewKanji: Returning" << cards.size() << "kanji for review";
    return cards;
}

int KanjiDatabase::getTotalKanjiCount()
{
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM kanji");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int KanjiDatabase::getLearnedKanjiCount()
{
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM kanji WHERE is_learned = TRUE");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int KanjiDatabase::getReviewDueCount()
{
    QSqlQuery query(db);
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("SELECT COUNT(*) FROM kanji WHERE is_learned = TRUE AND next_review <= ?");
    query.addBindValue(now);
    
    qDebug() << "getReviewDueCount: Current time is" << now.toString();
    qDebug() << "getReviewDueCount: Looking for kanji with next_review <=" << now.toString();
    
    if (query.exec() && query.next()) {
        int count = query.value(0).toInt();
        qDebug() << "getReviewDueCount: Found" << count << "kanji due for review";
        return count;
    }
    return 0;
}

int KanjiDatabase::getNewKanjiCount()
{
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM kanji WHERE is_learned = FALSE");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QMap<int, int> KanjiDatabase::getKanjiCountByLevel()
{
    QMap<int, int> levelCounts;
    
    // Initialize all levels to 0
    for (int i = 0; i <= 8; ++i) {
        levelCounts[i] = 0;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT srs_level, COUNT(*) FROM kanji WHERE is_learned = TRUE GROUP BY srs_level");
    
    if (query.exec()) {
        while (query.next()) {
            int level = query.value(0).toInt();
            int count = query.value(1).toInt();
            levelCounts[level] = count;
        }
    }
    
    // Also get unlearned kanji count (level 0)
    query.prepare("SELECT COUNT(*) FROM kanji WHERE is_learned = FALSE");
    if (query.exec() && query.next()) {
        levelCounts[0] = query.value(0).toInt();
    }
    
    return levelCounts;
}

bool KanjiDatabase::updateKanjiProgress(int id, bool correct, int difficulty)
{
    try {
        QSqlQuery query(db);
        QDateTime now = QDateTime::currentDateTime();
        
        // DEBUG: Check if datetime is working correctly
        qDebug() << "updateKanjiProgress: Current time:" << now.toString();
        qDebug() << "updateKanjiProgress: Unix timestamp:" << now.toSecsSinceEpoch();
        
        // Get current kanji data
        KanjiCard currentKanji = getKanjiById(id);
        
        if (correct) {
            // For unlearned kanji (level 0), start at level 1
            // For already learned kanji, advance to next level (max level 8)
            int newLevel;
            if (currentKanji.srs_level == 0 || !currentKanji.is_learned) {
                newLevel = 1; // First time learning
            } else {
                newLevel = qMin(currentKanji.srs_level + 1, 8); // Advance level
            }
            
            // SRS intervals (in seconds for testing - very short for quick testing)
            // Level 1: 10 sec, Level 2: 30 sec, Level 3: 60 sec, Level 4: 120 sec
            // Level 5: 300 sec (5 min), Level 6: 600 sec (10 min), Level 7: 1800 sec (30 min), Level 8: 3600 sec (1 hour)
            QDateTime nextReview;
            switch (newLevel) {
                case 1: nextReview = now.addSecs(10); break;     // 10 seconds
                case 2: nextReview = now.addSecs(30); break;     // 30 seconds  
                case 3: nextReview = now.addSecs(60); break;     // 1 minute
                case 4: nextReview = now.addSecs(120); break;    // 2 minutes
                case 5: nextReview = now.addSecs(300); break;    // 5 minutes
                case 6: nextReview = now.addSecs(600); break;    // 10 minutes
                case 7: nextReview = now.addSecs(1800); break;   // 30 minutes
                case 8: nextReview = now.addSecs(3600); break;   // 1 hour
                default: nextReview = now.addSecs(10); break;
            }
            
            qDebug() << "Setting kanji" << currentKanji.kanji << "to level" << newLevel 
                     << "with next review at" << nextReview.toString();
            
            query.prepare(R"(
                UPDATE kanji SET 
                    is_learned = TRUE,
                    last_reviewed = ?,
                    next_review = ?,
                    srs_level = ?,
                    review_count = review_count + 1
                WHERE id = ?
            )");
            
            query.addBindValue(now);
            query.addBindValue(nextReview);
            query.addBindValue(newLevel);
            query.addBindValue(id);
            
        } else {
            // Lower level by 1 (minimum level 1) and set review time based on new level
            int newLevel = qMax(currentKanji.srs_level - 1, 1);
            
            QDateTime nextReview;
            switch (newLevel) {
                case 1: nextReview = now.addSecs(10); break;     // 10 seconds
                case 2: nextReview = now.addSecs(30); break;     // 30 seconds  
                case 3: nextReview = now.addSecs(60); break;     // 1 minute
                case 4: nextReview = now.addSecs(120); break;    // 2 minutes
                case 5: nextReview = now.addSecs(300); break;    // 5 minutes
                case 6: nextReview = now.addSecs(600); break;    // 10 minutes
                case 7: nextReview = now.addSecs(1800); break;   // 30 minutes
                case 8: nextReview = now.addSecs(3600); break;   // 1 hour
                default: nextReview = now.addSecs(10); break;
            }
            
            qDebug() << "Lowering kanji" << currentKanji.kanji << "to level" << newLevel 
                     << "with next review at" << nextReview.toString();
            
            query.prepare(R"(
                UPDATE kanji SET 
                    last_reviewed = ?,
                    next_review = ?,
                    srs_level = ?,
                    review_count = review_count + 1
                WHERE id = ?
            )");
            
            query.addBindValue(now);
            query.addBindValue(nextReview);
            query.addBindValue(newLevel);
            query.addBindValue(id);
        }
        
        if (!query.exec()) {
            throw std::runtime_error(("Failed to update kanji progress: " + query.lastError().text()).toStdString());
        }
        
        return true;
    }
    catch (const std::exception& e) {
        lastError = QString("Error updating kanji progress: %1").arg(e.what());
        qDebug() << "Exception in updateKanjiProgress:" << e.what();
        return false;
    }
    catch (...) {
        lastError = "Unknown error occurred while updating kanji progress";
        qDebug() << "Unknown exception in updateKanjiProgress";
        return false;
    }
}

QList<KanjiCard> KanjiDatabase::getAllKanji()
{
    QList<KanjiCard> cards;
    QSqlQuery query(db);
    query.prepare("SELECT * FROM kanji ORDER BY id");
    
    if (query.exec()) {
        while (query.next()) {
            KanjiCard card;
            card.id = query.value("id").toInt();
            card.kanji = query.value("kanji").toString();
            card.meaning = query.value("meaning").toString();
            card.on_reading = query.value("on_reading").toString();
            card.kun_reading = query.value("kun_reading").toString();
            card.example_word = query.value("example_word").toString();
            card.example_reading = query.value("example_reading").toString();
            card.example_meaning = query.value("example_meaning").toString();
            card.difficulty_level = query.value("difficulty_level").toInt();
            card.is_learned = query.value("is_learned").toBool();
            card.last_reviewed = query.value("last_reviewed").toDateTime();
            card.next_review = query.value("next_review").toDateTime();
            card.srs_level = query.value("srs_level").toInt();
            card.review_count = query.value("review_count").toInt();
            cards.append(card);
        }
    }
    
    return cards;
}

KanjiCard KanjiDatabase::getKanjiById(int id)
{
    KanjiCard card;
    QSqlQuery query(db);
    query.prepare("SELECT * FROM kanji WHERE id = ?");
    query.addBindValue(id);
    
    if (query.exec() && query.next()) {
        card.id = query.value("id").toInt();
        card.kanji = query.value("kanji").toString();
        card.meaning = query.value("meaning").toString();
        card.on_reading = query.value("on_reading").toString();
        card.kun_reading = query.value("kun_reading").toString();
        card.example_word = query.value("example_word").toString();
        card.example_reading = query.value("example_reading").toString();
        card.example_meaning = query.value("example_meaning").toString();
        card.difficulty_level = query.value("difficulty_level").toInt();
        card.is_learned = query.value("is_learned").toBool();
        card.last_reviewed = query.value("last_reviewed").toDateTime();
        card.next_review = query.value("next_review").toDateTime();
        card.srs_level = query.value("srs_level").toInt();
        card.review_count = query.value("review_count").toInt();
    }
    
    return card;
}

bool KanjiDatabase::setImmediateReviewTime(int id, int secondsFromNow)
{
    QSqlQuery query(db);
    QDateTime reviewTime = QDateTime::currentDateTime().addSecs(secondsFromNow);
    
    query.prepare("UPDATE kanji SET next_review = ? WHERE id = ?");
    query.addBindValue(reviewTime);
    query.addBindValue(id);
    
    if (!query.exec()) {
        lastError = "Failed to set immediate review time: " + query.lastError().text();
        return false;
    }
    
    return true;
}

bool KanjiDatabase::resetAllKanjiToUnlearned()
{
    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE kanji SET 
            is_learned = FALSE,
            last_reviewed = NULL,
            next_review = NULL,
            srs_level = 0,
            review_count = 0
    )");
    
    if (!query.exec()) {
        lastError = "Failed to reset kanji: " + query.lastError().text();
        qDebug() << "Database error resetting kanji:" << lastError;
        return false;
    }
    
    qDebug() << "Successfully reset all kanji to unlearned state";
    return true;
}

void KanjiDatabase::debugShowAllLearnedKanji()
{
    QSqlQuery query(db);
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("SELECT * FROM kanji WHERE is_learned = TRUE ORDER BY next_review");
    
    qDebug() << "=== DEBUG: All Learned Kanji ===";
    qDebug() << "Current time:" << now.toString();
    
    if (query.exec()) {
        while (query.next()) {
            QString kanji = query.value("kanji").toString();
            int srs_level = query.value("srs_level").toInt();
            QDateTime next_review = query.value("next_review").toDateTime();
            QDateTime last_reviewed = query.value("last_reviewed").toDateTime();
            bool is_due = next_review <= now;
            
            qDebug() << "Kanji:" << kanji 
                     << "| Level:" << srs_level
                     << "| Last reviewed:" << last_reviewed.toString()
                     << "| Next review:" << next_review.toString()
                     << "| Due now?" << is_due;
        }
    }
    qDebug() << "=== End Debug ===";
} 