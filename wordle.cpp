#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <queue>
#include <memory>
#include <algorithm>
#include <random>
#include <cctype>
#include <typeinfo>
#include <stdexcept>

using namespace std;

// ============================================================================
// UNIT 1: STRUCT SPECIFICATION & MANIPULATORS
// ============================================================================

enum class LetterColor {
    EMPTY,
    GRAY,    // Letter not in word
    YELLOW,  // Letter in word, but wrong spot
    GREEN    // Letter correct & right spot
};

// Struct (default public members vs class default private)
struct LetterTile {
    char letter;
    LetterColor status;

    LetterTile(char c = ' ', LetterColor s = LetterColor::EMPTY)
        : letter(static_cast<char>(toupper(c))), status(s) {}

    // Operator Overloading for equality check (Unit 2)
    bool operator==(const LetterTile& other) const {
        return (letter == other.letter && status == other.status);
    }

    // Friend stream insertion operator with ANSI terminal colors (Unit 1 & 2)
    friend ostream& operator<<(ostream& os, const LetterTile& tile) {
        if (tile.status == LetterColor::GREEN) {
            os << "\033[1;37;42m " << tile.letter << " \033[0m "; // Green
        } else if (tile.status == LetterColor::YELLOW) {
            os << "\033[1;37;43m " << tile.letter << " \033[0m "; // Yellow
        } else if (tile.status == LetterColor::GRAY) {
            os << "\033[1;37;100m " << tile.letter << " \033[0m "; // Gray
        } else {
            os << "\033[1;30;47m _ \033[0m "; // Empty slot
        }
        return os;
    }
};

// Inline function (Unit 1 self-learning topic)
inline bool isAlphaString(const string& str) {
    for (size_t i = 0; i < str.length(); ++i) {
        if (!isalpha(static_cast<unsigned char>(str[i]))) return false;
    }
    return true;
}

// Function Template (Unit 3)
template <typename T>
T clampValue(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

// Class Template: Circular Guess History Ring Buffer (Unit 3)
template <typename T, size_t Capacity>
class CircularGuessRingBuffer {
private:
    T entries[Capacity];
    size_t head;
    size_t count;

public:
    CircularGuessRingBuffer() : head(0), count(0) {}

    void push(const T& entry) {
        entries[head] = entry;
        head = (head + 1) % Capacity;
        if (count < Capacity) count++;
    }

    size_t size() const { return count; }
};

// ============================================================================
// UNIT 2: USER-DEFINED EXCEPTION HANDLING
// ============================================================================

class WordleException : public exception {
protected:
    string message;
public:
    explicit WordleException(const string& msg) : message(msg) {}
    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

class InvalidLengthException : public WordleException {
public:
    explicit InvalidLengthException(size_t len)
        : WordleException("INVALID GUESS LENGTH: Expected 5 letters, but got " +
                          to_string(len) + " letters!") {}
};

class NonAlphaWordException : public WordleException {
public:
    explicit NonAlphaWordException(const string& input)
        : WordleException("INPUT REJECTED: '" + input + "' contains numbers or symbols. Letters only!") {}
};

class HardModeViolationException : public WordleException {
public:
    HardModeViolationException(char expectedChar, int position)
        : WordleException("HARD MODE RULE: Must reuse '" + string(1, expectedChar) +
                          "' at position " + to_string(position + 1) + " from your green clue!") {}
};

// ============================================================================
// UNIT 1 & 2: ABSTRACT BASE CLASS & INHERITANCE
// ============================================================================

class WordleBase;
void revealSecretCheat(const WordleBase& game);

class WordleBase {
protected:
    string targetWord;            // C++ string
    char secretCStr[8];           // C-style string handling (Unit 1 self-learning)
    int maxAttempts;
    int currentAttempt;
    bool isGameWon;
    bool isGameOver;

    // Dynamic memory allocation on heap (Unit 1 & 3)
    char* auditLogBuffer;
    size_t logCapacity;

    vector<string> guessHistory;                  // STL Vector (Unit 3)
    vector<vector<LetterTile>> gridTiles;         // 6x5 Wordle visual grid
    map<char, LetterColor> keyboardTracker;       // STL Map (Unit 3)
    CircularGuessRingBuffer<string, 6> ringBuffer;// Class Template (Unit 3)

    // Static Data Members (Unit 1)
    static int s_totalGamesStarted;
    static int s_totalGamesWon;

public:
    // Nested Class / Member Class (Unit 2)
    class StatisticsCard {
    private:
        int winStreak;
        int maxStreak;
    public:
        StatisticsCard() : winStreak(0), maxStreak(0) {}
        void registerWin() {
            winStreak++;
            if (winStreak > maxStreak) maxStreak = winStreak;
        }
        void registerLoss() { winStreak = 0; }
        void displayStats() const {
            cout << "  [STREAK METRICS] Current Streak: " << winStreak 
                 << " 🔥 | Max Streak: " << maxStreak << " 🏆\n";
        }
    };

    StatisticsCard stats;

    // Default Constructor (Unit 1)
    WordleBase();

    // Parameterized Constructor (Unit 1)
    WordleBase(const string& secret, int attempts);

    // Deep Copy Constructor (Unit 1)
    WordleBase(const WordleBase& other);

    // Destructor - RAII Memory Cleanup (Unit 1 & 3)
    virtual ~WordleBase();

    // Static Member Functions (Unit 1)
    static int getTotalGamesStarted() { return s_totalGamesStarted; }
    static int getTotalGamesWon() { return s_totalGamesWon; }

    // Member function defined outside class (Unit 1)
    void appendAuditLog(const char* eventText);

    // Function Overloading (Unit 1 & 2)
    void logEvent(const string& eventStr);
    void logEvent(const string& eventStr, int attemptNumber);

    // Operator Overloading: checks if guess matches target word (Unit 2)
    bool operator==(const string& guess) const {
        return (this->targetWord == guess); // 'this' pointer (Unit 1)
    }

    // Pure Virtual Functions (Run-time polymorphism / Abstract Interface)
    virtual bool submitGuess(const string& inputGuess) = 0;
    virtual void printGameHUD() const = 0;

    // Helper method to print keyboard rows cleanly
    void printKeyRow(const string& keys) const {
        for (size_t i = 0; i < keys.length(); ++i) {
            char k = keys[i];
            map<char, LetterColor>::const_iterator it = keyboardTracker.find(k);
            if (it != keyboardTracker.end()) {
                if (it->second == LetterColor::GREEN) {
                    cout << "\033[1;32m" << k << "\033[0m ";
                } else if (it->second == LetterColor::YELLOW) {
                    cout << "\033[1;33m" << k << "\033[0m ";
                } else {
                    cout << "\033[1;90m" << k << "\033[0m ";
                }
            } else {
                cout << k << " ";
            }
        }
    }

    void resetRound(const string& newSecret);
    bool isFinished() const { return isGameOver; }
    bool hasWon() const { return isGameWon; }
    string getTargetWord() const { return targetWord; }
    int getRemainingAttempts() const { return maxAttempts - currentAttempt; }

    // Friend function declaration (Unit 1)
    friend void revealSecretCheat(const WordleBase& game);
};

// Static Data Members Initialization (Unit 1)
int WordleBase::s_totalGamesStarted = 0;
int WordleBase::s_totalGamesWon = 0;

// Default Constructor Definition
WordleBase::WordleBase()
    : targetWord("APPLE"), maxAttempts(6), currentAttempt(0),
      isGameWon(false), isGameOver(false), logCapacity(128) {
    strncpy(secretCStr, "APPLE", sizeof(secretCStr) - 1);
    secretCStr[sizeof(secretCStr) - 1] = '\0';
    auditLogBuffer = new char[logCapacity]; // Dynamic Heap Allocation
    strcpy(auditLogBuffer, "Session Initialized.");
    s_totalGamesStarted++;
}

// Parameterized Constructor Definition
WordleBase::WordleBase(const string& secret, int attempts)
    : targetWord(secret), maxAttempts(attempts), currentAttempt(0),
      isGameWon(false), isGameOver(false), logCapacity(128) {
    strncpy(secretCStr, secret.c_str(), sizeof(secretCStr) - 1);
    secretCStr[sizeof(secretCStr) - 1] = '\0';
    auditLogBuffer = new char[logCapacity]; // Dynamic Heap Allocation
    strcpy(auditLogBuffer, "Session Started.");
    s_totalGamesStarted++;
}

// Deep Copy Constructor Definition (Unit 1)
WordleBase::WordleBase(const WordleBase& other)
    : targetWord(other.targetWord), maxAttempts(other.maxAttempts),
      currentAttempt(other.currentAttempt), isGameWon(other.isGameWon),
      isGameOver(other.isGameOver), logCapacity(other.logCapacity),
      guessHistory(other.guessHistory), gridTiles(other.gridTiles),
      keyboardTracker(other.keyboardTracker), stats(other.stats) {
    strncpy(secretCStr, other.secretCStr, sizeof(secretCStr));
    auditLogBuffer = new char[logCapacity]; // Distinct heap allocation
    strncpy(auditLogBuffer, other.auditLogBuffer, logCapacity - 1);
    auditLogBuffer[logCapacity - 1] = '\0';
    s_totalGamesStarted++;
}

// Destructor Definition - RAII (Unit 1 & 3)
WordleBase::~WordleBase() {
    delete[] auditLogBuffer; // Free heap memory
}

// Outside Class Member Function (Unit 1)
void WordleBase::appendAuditLog(const char* eventText) {
    size_t newLen = strlen(auditLogBuffer) + strlen(eventText) + 4;
    if (newLen >= logCapacity) {
        size_t newCap = newLen + 64;
        char* newBuf = new char[newCap];
        strcpy(newBuf, auditLogBuffer);
        delete[] auditLogBuffer;
        auditLogBuffer = newBuf;
        logCapacity = newCap;
    }
    strcat(auditLogBuffer, " -> ");
    strcat(auditLogBuffer, eventText);
}

void WordleBase::logEvent(const string& eventStr) {
    appendAuditLog(eventStr.c_str());
}

void WordleBase::logEvent(const string& eventStr, int attemptNumber) {
    string fullEvent = eventStr + " [Attempt " + to_string(attemptNumber) + "]";
    appendAuditLog(fullEvent.c_str());
}

void WordleBase::resetRound(const string& newSecret) {
    this->targetWord = newSecret;
    strncpy(secretCStr, newSecret.c_str(), sizeof(secretCStr) - 1);
    secretCStr[sizeof(secretCStr) - 1] = '\0';
    currentAttempt = 0;
    isGameWon = false;
    isGameOver = false;
    guessHistory.clear();
    gridTiles.clear();
    keyboardTracker.clear();
    s_totalGamesStarted++;
    logEvent("New Round Started");
}

// Friend Function Definition (Unit 1)
void revealSecretCheat(const WordleBase& game) {
    cout << "\n  [ORACLE CHEAT AUDIT (FRIEND FUNCTION)]:\n";
    cout << "   * Private C-string: " << game.secretCStr << "\n";
    cout << "   * Private string Target: " << game.targetWord << "\n";
    cout << "   * Heap Buffer: " << game.logCapacity << " bytes allocated\n";
    cout << "   * Total Games Tracked: " << WordleBase::s_totalGamesStarted << "\n";
}

// ============================================================================
// UNIT 2: DERIVED CLASSES (STANDARD & HARD MODE)
// ============================================================================

class StandardWordle : public WordleBase {
public:
    StandardWordle(const string& secret = "APPLE", int attempts = 6)
        : WordleBase(secret, attempts) {}

    bool submitGuess(const string& rawGuess) override {
        if (isGameOver) return false;

        if (rawGuess.length() != 5) {
            throw InvalidLengthException(rawGuess.length());
        }
        if (!isAlphaString(rawGuess)) {
            throw NonAlphaWordException(rawGuess);
        }

        string upperGuess = rawGuess;
        for (size_t i = 0; i < upperGuess.length(); ++i) {
            upperGuess[i] = static_cast<char>(toupper(upperGuess[i]));
        }

        vector<LetterTile> row(5);
        string targetCopy = targetWord;

        // First Pass: Exact matches (GREEN)
        for (size_t i = 0; i < 5; ++i) {
            if (upperGuess[i] == targetCopy[i]) {
                row[i] = LetterTile(upperGuess[i], LetterColor::GREEN);
                keyboardTracker[upperGuess[i]] = LetterColor::GREEN;
                targetCopy[i] = '*'; // Consumed
            }
        }

        // Second Pass: Misplaced (YELLOW) or Misses (GRAY)
        for (size_t i = 0; i < 5; ++i) {
            if (row[i].status == LetterColor::GREEN) continue;

            size_t foundIdx = targetCopy.find(upperGuess[i]);
            if (foundIdx != string::npos) {
                row[i] = LetterTile(upperGuess[i], LetterColor::YELLOW);
                targetCopy[foundIdx] = '*';
                if (keyboardTracker[upperGuess[i]] != LetterColor::GREEN) {
                    keyboardTracker[upperGuess[i]] = LetterColor::YELLOW;
                }
            } else {
                row[i] = LetterTile(upperGuess[i], LetterColor::GRAY);
                if (keyboardTracker.find(upperGuess[i]) == keyboardTracker.end()) {
                    keyboardTracker[upperGuess[i]] = LetterColor::GRAY;
                }
            }
        }

        gridTiles.push_back(row);
        guessHistory.push_back(upperGuess);
        ringBuffer.push(upperGuess);
        currentAttempt++;

        logEvent("Submitted guess: " + upperGuess, currentAttempt);

        if (*this == upperGuess) {
            isGameWon = true;
            isGameOver = true;
            s_totalGamesWon++;
            stats.registerWin();
            return true;
        }

        if (currentAttempt >= maxAttempts) {
            isGameOver = true;
            stats.registerLoss();
        }

        return false;
    }

    void printGameHUD() const override {
        cout << "\n========================================================\n";
        cout << "                 WORDLE TERMINAL ENGINE                 \n";
        cout << "========================================================\n";

        for (size_t r = 0; r < 6; ++r) {
            cout << "       ";
            if (r < gridTiles.size()) {
                for (size_t c = 0; c < 5; ++c) cout << gridTiles[r][c];
            } else {
                for (size_t c = 0; c < 5; ++c) cout << LetterTile(' ', LetterColor::EMPTY);
            }
            cout << "\n";
        }

        cout << "--------------------------------------------------------\n";
        cout << "  Attempts: " << currentAttempt << " / " << maxAttempts 
             << "  |  Remaining: " << (maxAttempts - currentAttempt) << "\n";
        stats.displayStats();

        cout << "\n  [KEYBOARD TRACKER]:\n  ";
        printKeyRow("QWERTYUIOP");
        cout << "\n   ";
        printKeyRow("ASDFGHJKL");
        cout << "\n     ";
        printKeyRow("ZXCVBNM");
        cout << "\n========================================================\n";
    }
};

class HardModeWordle : public StandardWordle {
private:
    char lockedGreenLetters[5];
public:
    HardModeWordle(const string& secret = "CRANE", int attempts = 6)
        : StandardWordle(secret, attempts) {
        for (int i = 0; i < 5; ++i) lockedGreenLetters[i] = ' ';
    }

    bool submitGuess(const string& rawGuess) override {
        string upperGuess = rawGuess;
        for (size_t i = 0; i < upperGuess.length(); ++i) {
            upperGuess[i] = static_cast<char>(toupper(upperGuess[i]));
        }

        if (upperGuess.length() == 5) {
            for (int i = 0; i < 5; ++i) {
                if (lockedGreenLetters[i] != ' ' && upperGuess[i] != lockedGreenLetters[i]) {
                    throw HardModeViolationException(lockedGreenLetters[i], i);
                }
            }
        }

        bool won = StandardWordle::submitGuess(rawGuess);

        if (!gridTiles.empty()) {
            const vector<LetterTile>& latestRow = gridTiles.back();
            for (size_t i = 0; i < 5; ++i) {
                if (latestRow[i].status == LetterColor::GREEN) {
                    lockedGreenLetters[i] = latestRow[i].letter;
                }
            }
        }
        return won;
    }

    void printGameHUD() const override {
        cout << "\n  >>> [🔥 HARD MODE ACTIVE: GREEN LETTERS MUST BE REUSED 🔥] <<<\n";
        StandardWordle::printGameHUD();
    }
};

// ============================================================================
// UNIT 3: FILE STREAMS (LEADERBOARD PERSISTENCE)
// ============================================================================

class LeaderboardManager {
private:
    string filePath;
public:
    LeaderboardManager(string path = "wordle_scores.txt") : filePath(path) {}

    // File writing: ofstream (Unit 3)
    void recordScore(const string& player, const string& word, bool won, int tries) {
        ofstream outFile(filePath, ios::app);
        if (outFile) {
            outFile << left << setw(14) << player
                    << setw(10) << (won ? "WON" : "LOST")
                    << "Word: " << setw(8) << word
                    << "Tries: " << tries << "/6\n";
            outFile.close();
        }
    }

    // File reading: ifstream (Unit 3)
    void showLeaderboard() const {
        ifstream inFile(filePath);
        cout << "\n=======================================================\n";
        cout << "          WORDLE LEADERBOARD (FROM FILE STREAM)        \n";
        cout << "=======================================================\n";
        if (!inFile) {
            cout << "  (No saved match history found yet)\n";
            return;
        }
        string line;
        while (getline(inFile, line)) {
            cout << "  " << line << "\n";
        }
        inFile.close();
        cout << "=======================================================\n";
    }
};

// ============================================================================
// BUILT-IN WORD POOL
// ============================================================================

string getRandomWordFromPool() {
    static const vector<string> words = {
        "APPLE", "CHAIR", "PIANO", "GHOST", "SMILE",
        "TIGER", "BREAD", "CRANE", "WATER", "PLANT",
        "LIGHT", "STORM", "TRAIN", "BEACH", "FLAME",
        "CLOUD", "HEART", "MUSIC", "MONEY", "DREAM",
        "SPACE", "NIGHT", "SWEET", "RIVER", "OCEAN"
    };
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<size_t> dist(0, words.size() - 1);
    return words[dist(gen)];
}

// ============================================================================
// GAME ROUND RUNNER
// ============================================================================

void playRound(WordleBase& game, LeaderboardManager& leaderboard, const string& player) {
    string secret = getRandomWordFromPool();
    game.resetRound(secret);

    // Demonstrate RTTI (Unit 2 Self-Learning Topic)
    cout << "\n  [RTTI Check] Engine Type: " << typeid(game).name() << "\n";
    HardModeWordle* hardCheck = dynamic_cast<HardModeWordle*>(&game);
    if (hardCheck) {
        cout << "  [RTTI Verified] HardModeWordle class detected!\n";
    }

    while (!game.isFinished()) {
        game.printGameHUD();
        cout << "  Enter 5-Letter Guess (or 'Q' to forfeit): ";
        
        string guess;
        if (!(cin >> guess)) {
            if (cin.eof()) return;
            cin.clear();
            continue;
        }

        if (guess == "Q" || guess == "q") {
            cout << "  Forfeited! The secret word was: " << game.getTargetWord() << "\n";
            leaderboard.recordScore(player, game.getTargetWord(), false, 6);
            return;
        }

        try {
            bool won = game.submitGuess(guess);
            if (won) {
                game.printGameHUD();
                int attemptsUsed = 6 - game.getRemainingAttempts();
                cout << "\n  🎉 CONGRATULATIONS " << player << "! Solved in " 
                     << attemptsUsed << " attempt(s)!\n";
                leaderboard.recordScore(player, game.getTargetWord(), true, attemptsUsed);
                return;
            }
        } catch (const WordleException& ex) {
            cout << "\n  ⚠️  " << ex.what() << "\n";
            cout << "  (Not counted as a guess. Try again!)\n";
        }
    }

    if (!game.hasWon()) {
        game.printGameHUD();
        cout << "\n  💀 OUT OF GUESSES! The word was: \033[1;32m" 
             << game.getTargetWord() << "\033[0m\n";
        leaderboard.recordScore(player, game.getTargetWord(), false, 6);
    }
}

// ============================================================================
// MAIN FUNCTION (CLEAN 4-OPTION MENU)
// ============================================================================

int main() {
    cout << R"(
  ==============================================================
   ██╗    ██╗ ██████╗ ██████╗ ██████╗ ██╗     ███████╗
   ██║    ██║██╔═══██╗██╔══██╗██╔══██╗██║     ██╔════╝
   ██║ █╗ ██║██║   ██║██████╔╝██║  ██║██║     █████╗  
   ██║███╗██║██║   ██║██╔══██╗██║  ██║██║     ██╔══╝  
   ╚███╔███╔╝╚██████╔╝██║  ██║██████╔╝███████╗███████╗
    WORDLE CLI - RETRO TERMINAL EDITION
  ==============================================================
    )" << "\n";

    cout << "Enter Player Name: ";
    string player;
    if (!(cin >> player) || player.empty()) player = "Player1";

    LeaderboardManager leaderboard("wordle_scores.txt");
    StandardWordle standardGame("APPLE");
    HardModeWordle hardGame("CRANE");

    bool running = true;

    while (running) {
        cout << "\n================ [ WORDLE MAIN MENU ] ================\n";
        cout << "  1. Play Standard Wordle (6 Guesses)\n";
        cout << "  2. Play Hard Mode Wordle (Enforced Green Clues)\n";
        cout << "  3. View Match Leaderboard (File Streams: ifstream)\n";
        cout << "  4. Exit Wordle\n";
        cout << "------------------------------------------------------\n";
        cout << "Select Option [1-4]: ";

        int choice = 0;
        if (!(cin >> choice)) {
            if (cin.eof()) break;
            cin.clear();
            string discard;
            getline(cin, discard);
            continue;
        }

        switch (choice) {
            case 1:
                playRound(standardGame, leaderboard, player);
                break;
            case 2:
                playRound(hardGame, leaderboard, player);
                break;
            case 3:
                leaderboard.showLeaderboard();
                break;
            case 4:
                cout << "\nThanks for playing Wordle CLI! Goodbye!\n";
                running = false;
                break;
            default:
                cout << "Invalid choice! Enter a number between 1 and 4.\n";
                break;
        }
    }

    return 0;
}