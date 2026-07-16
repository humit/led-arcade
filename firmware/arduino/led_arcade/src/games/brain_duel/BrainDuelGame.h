#pragma once
#include <Arduino.h>
#include "../../Config.h"
#include "../../session/PlayerManager.h"

class BrainDuelGame {
public:
  enum Phase : uint8_t { IDLE = 0, QUESTION = 1, REVEAL = 2 };
  enum Kind : uint8_t { MATH = 0, WORD = 1 };
  enum Difficulty : uint8_t { EASY = 0, NORMAL = 1, CHALLENGE = 2 };

  static constexpr uint8_t NO_ANSWER = 255;
  static constexpr uint8_t EVENT_NONE = 0;
  static constexpr uint8_t EVENT_QUESTION = 1;
  static constexpr uint8_t EVENT_ANSWER = 2;
  static constexpr uint8_t EVENT_REVEAL = 3;

  Phase phase = IDLE;
  Kind kind = MATH;
  Difficulty difficulty = EASY;
  uint32_t questionId = 0;
  uint8_t questionNumber = 0;
  uint8_t correctIndex = 0;
  String promptTr;
  String promptEn;
  String optionsTr[BRAIN_DUEL_OPTION_COUNT];
  String optionsEn[BRAIN_DUEL_OPTION_COUNT];
  uint32_t questionEndsAtMs = 0;
  uint32_t revealEndsAtMs = 0;
  uint32_t eventId = 0;
  uint8_t eventType = EVENT_NONE;
  int8_t eventSlot = -1;
  bool eventCorrect = false;
  bool answered[MAX_PLAYERS] = {false};
  bool answerCorrect[MAX_PLAYERS] = {false};
  uint8_t selectedAnswer[MAX_PLAYERS] = {NO_ANSWER};

  void clear() {
    phase = IDLE;
    kind = MATH;
    difficulty = EASY;
    questionId = 0;
    questionNumber = 0;
    correctIndex = 0;
    promptTr = "";
    promptEn = "";
    for (auto& option : optionsTr) option = "";
    for (auto& option : optionsEn) option = "";
    questionEndsAtMs = 0;
    revealEndsAtMs = 0;
    eventId = 0;
    eventType = EVENT_NONE;
    eventSlot = -1;
    eventCorrect = false;
    previousHumanCorrectCount = 0;
    firstCorrectSlot = -1;
    memset(answered, 0, sizeof(answered));
    memset(answerCorrect, 0, sizeof(answerCorrect));
    memset(selectedAnswer, NO_ANSWER, sizeof(selectedAnswer));
    memset(cpuDueMs, 0, sizeof(cpuDueMs));
    memset(cpuAnswer, NO_ANSWER, sizeof(cpuAnswer));
  }

  void prepare(PlayerManager& players) {
    clear();
    for (auto& player : players.players) {
      if (!player.occupied || !player.connected || player.waiting) continue;
      player.score = 0;
      player.lastTapMs = 0;
    }
  }

  void start(PlayerManager& players, uint32_t now) {
    nextQuestion(players, now);
  }

  bool update(PlayerManager& players, uint32_t now) {
    if (phase == IDLE) return false;

    if (phase == QUESTION) {
      updateCpu(players, now);
      if (now >= questionEndsAtMs || allActiveAnswered(players)) {
        beginReveal(players, now);
      }
      return false;
    }

    if (phase == REVEAL && now >= revealEndsAtMs) {
      if (questionNumber >= BRAIN_DUEL_QUESTION_COUNT) {
        phase = IDLE;
        return true;
      }
      nextQuestion(players, now);
    }

    return false;
  }

  bool answer(
      uint8_t slot,
      uint32_t submittedQuestionId,
      uint8_t answerIndex,
      PlayerManager& players,
      uint32_t now
  ) {
    if (phase != QUESTION || slot >= MAX_PLAYERS || answerIndex >= BRAIN_DUEL_OPTION_COUNT) return false;
    if (submittedQuestionId != questionId || answered[slot]) return false;

    PlayerSlot& player = players.players[slot];
    if (!player.occupied || !player.connected || player.waiting || player.isCpu) return false;
    recordAnswer(slot, answerIndex, players, now);
    return true;
  }

  uint32_t remainingMs(uint32_t now) const {
    if (phase == QUESTION) return now >= questionEndsAtMs ? 0 : questionEndsAtMs - now;
    if (phase == REVEAL) return now >= revealEndsAtMs ? 0 : revealEndsAtMs - now;
    return 0;
  }

  int8_t visibleCorrectIndex() const {
    return phase == REVEAL ? int8_t(correctIndex) : -1;
  }

private:
  struct WordEntry {
    const char* tr;
    const char* en;
    uint8_t category;
  };

  static constexpr uint8_t CATEGORY_ANIMAL = 0;
  static constexpr uint8_t CATEGORY_FRUIT = 1;
  static constexpr uint8_t CATEGORY_SCHOOL = 2;
  static constexpr uint8_t CATEGORY_TRANSPORT = 3;
  static constexpr uint8_t CATEGORY_NATURE = 4;
  static constexpr uint8_t CATEGORY_HOME = 5;
  static constexpr uint8_t CATEGORY_COUNT = 6;

  enum : uint8_t { WORD_COUNT = 30 };

  const WordEntry* words() const {
    static const WordEntry entries[WORD_COUNT] = {
      {"kedi", "cat", CATEGORY_ANIMAL}, {"köpek", "dog", CATEGORY_ANIMAL},
      {"aslan", "lion", CATEGORY_ANIMAL}, {"tavşan", "rabbit", CATEGORY_ANIMAL},
      {"balık", "fish", CATEGORY_ANIMAL},
      {"elma", "apple", CATEGORY_FRUIT}, {"muz", "banana", CATEGORY_FRUIT},
      {"armut", "pear", CATEGORY_FRUIT}, {"portakal", "orange", CATEGORY_FRUIT},
      {"çilek", "strawberry", CATEGORY_FRUIT},
      {"kalem", "pencil", CATEGORY_SCHOOL}, {"kitap", "book", CATEGORY_SCHOOL},
      {"silgi", "eraser", CATEGORY_SCHOOL}, {"defter", "notebook", CATEGORY_SCHOOL},
      {"cetvel", "ruler", CATEGORY_SCHOOL},
      {"araba", "car", CATEGORY_TRANSPORT}, {"otobüs", "bus", CATEGORY_TRANSPORT},
      {"tren", "train", CATEGORY_TRANSPORT}, {"bisiklet", "bicycle", CATEGORY_TRANSPORT},
      {"uçak", "plane", CATEGORY_TRANSPORT},
      {"güneş", "sun", CATEGORY_NATURE}, {"yağmur", "rain", CATEGORY_NATURE},
      {"ağaç", "tree", CATEGORY_NATURE}, {"çiçek", "flower", CATEGORY_NATURE},
      {"bulut", "cloud", CATEGORY_NATURE},
      {"masa", "table", CATEGORY_HOME}, {"sandalye", "chair", CATEGORY_HOME},
      {"yatak", "bed", CATEGORY_HOME}, {"lamba", "lamp", CATEGORY_HOME},
      {"kapı", "door", CATEGORY_HOME}
    };
    return entries;
  }

  uint8_t previousHumanCorrectCount = 0;
  int8_t firstCorrectSlot = -1;
  uint32_t cpuDueMs[MAX_PLAYERS] = {0};
  uint8_t cpuAnswer[MAX_PLAYERS] = {NO_ANSWER};

  void publish(uint8_t type, int8_t slot, bool correct) {
    eventId++;
    eventType = type;
    eventSlot = slot;
    eventCorrect = correct;
  }

  uint8_t humanActiveCount(const PlayerManager& players) const {
    uint8_t count = 0;
    for (const auto& player : players.players) {
      if (player.occupied && player.connected && !player.waiting && !player.isCpu) count++;
    }
    return count;
  }

  bool allActiveAnswered(const PlayerManager& players) const {
    uint8_t active = 0;
    uint8_t complete = 0;
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& player = players.players[slot];
      if (!player.occupied || !player.connected || player.waiting) continue;
      active++;
      if (answered[slot]) complete++;
    }
    return active > 0 && complete == active;
  }

  Difficulty chooseDifficulty(const PlayerManager& players) const {
    const uint8_t nextNumber = questionNumber + 1;
    if (nextNumber == 1) return EASY;
    if (nextNumber == 2) return NORMAL;

    const uint8_t active = humanActiveCount(players);
    if (previousHumanCorrectCount == 0) return random(100) < 65 ? EASY : NORMAL;
    if (nextNumber % 3 == 0) return previousHumanCorrectCount >= active ? CHALLENGE : NORMAL;

    const uint8_t roll = uint8_t(random(100));
    if (previousHumanCorrectCount >= active) {
      if (roll < 10) return EASY;
      if (roll < 75) return NORMAL;
      return CHALLENGE;
    }
    if (roll < 20) return EASY;
    if (roll < 85) return NORMAL;
    return CHALLENGE;
  }

  void nextQuestion(PlayerManager& players, uint32_t now) {
    difficulty = chooseDifficulty(players);
    questionNumber++;
    questionId++;
    kind = (questionNumber == 2 || questionNumber == 4 || questionNumber == 7 || questionNumber == 9)
        ? WORD
        : MATH;
    firstCorrectSlot = -1;
    memset(answered, 0, sizeof(answered));
    memset(answerCorrect, 0, sizeof(answerCorrect));
    memset(selectedAnswer, NO_ANSWER, sizeof(selectedAnswer));
    memset(cpuDueMs, 0, sizeof(cpuDueMs));
    memset(cpuAnswer, NO_ANSWER, sizeof(cpuAnswer));

    if (kind == MATH) generateMathQuestion();
    else generateWordQuestion();

    phase = QUESTION;
    questionEndsAtMs = now + (
        difficulty == EASY ? BRAIN_DUEL_EASY_QUESTION_MS
        : difficulty == NORMAL ? BRAIN_DUEL_NORMAL_QUESTION_MS
        : BRAIN_DUEL_CHALLENGE_QUESTION_MS);
    revealEndsAtMs = 0;
    scheduleCpu(players, now);
    publish(EVENT_QUESTION, -1, false);
  }

  void beginReveal(PlayerManager& players, uint32_t now) {
    previousHumanCorrectCount = 0;
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& player = players.players[slot];
      if (!player.occupied || !player.connected || player.waiting || player.isCpu) continue;
      if (answerCorrect[slot]) previousHumanCorrectCount++;
    }
    phase = REVEAL;
    revealEndsAtMs = now + BRAIN_DUEL_REVEAL_MS;
    memset(cpuDueMs, 0, sizeof(cpuDueMs));
    publish(EVENT_REVEAL, -1, false);
  }

  void scheduleCpu(PlayerManager& players, uint32_t now) {
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      const PlayerSlot& cpu = players.players[slot];
      if (!cpu.occupied || !cpu.connected || cpu.waiting || !cpu.isCpu) continue;

      int accuracy = difficulty == EASY ? 82 : difficulty == NORMAL ? 64 : 43;
      uint8_t bestHumanScore = 0;
      for (const auto& player : players.players) {
        if (player.occupied && player.connected && !player.waiting && !player.isCpu) {
          bestHumanScore = max(bestHumanScore, player.score);
        }
      }
      if (cpu.score + 14 < bestHumanScore) accuracy += 10;
      if (cpu.score > bestHumanScore + 14) accuracy -= 10;
      accuracy = constrain(accuracy, 30, 88);

      const bool willBeCorrect = random(100) < accuracy;
      if (willBeCorrect) cpuAnswer[slot] = correctIndex;
      else {
        uint8_t wrong = uint8_t(random(BRAIN_DUEL_OPTION_COUNT - 1));
        if (wrong >= correctIndex) wrong++;
        cpuAnswer[slot] = wrong;
      }

      const uint32_t minDelay = difficulty == EASY ? 1800 : difficulty == NORMAL ? 2300 : 2800;
      const uint32_t maxDelay = difficulty == EASY ? 3900 : difficulty == NORMAL ? 5200 : 6500;
      cpuDueMs[slot] = now + random(minDelay, maxDelay + 1);
    }
  }

  void updateCpu(PlayerManager& players, uint32_t now) {
    for (uint8_t slot = 0; slot < MAX_PLAYERS; slot++) {
      PlayerSlot& cpu = players.players[slot];
      if (!cpu.occupied || !cpu.connected || cpu.waiting || !cpu.isCpu || answered[slot]) continue;
      if (cpuDueMs[slot] == 0 || now < cpuDueMs[slot]) continue;
      recordAnswer(slot, cpuAnswer[slot], players, now);
    }
  }

  void recordAnswer(uint8_t slot, uint8_t answerIndex, PlayerManager& players, uint32_t now) {
    answered[slot] = true;
    selectedAnswer[slot] = answerIndex;
    answerCorrect[slot] = answerIndex == correctIndex;
    cpuDueMs[slot] = 0;

    if (answerCorrect[slot]) {
      uint8_t points = BRAIN_DUEL_CORRECT_POINTS;
      if (firstCorrectSlot < 0) {
        firstCorrectSlot = slot;
        points += BRAIN_DUEL_FIRST_BONUS_POINTS;
      }
      PlayerSlot& player = players.players[slot];
      player.score = uint8_t(min(255, int(player.score) + int(points)));
      player.totalPoints += points;
    }
    publish(EVENT_ANSWER, slot, answerCorrect[slot]);
  }

  void setMathQuestion(const String& prompt, int correct, int spread) {
    promptTr = prompt;
    promptEn = prompt;
    int values[BRAIN_DUEL_OPTION_COUNT] = {correct, correct + spread, correct - spread, correct + spread * 2};
    if (values[2] < 0) values[2] = correct + spread + 1;
    for (uint8_t i = 0; i < BRAIN_DUEL_OPTION_COUNT; i++) {
      for (uint8_t j = 0; j < i; j++) {
        if (values[i] == values[j]) values[i] += i + spread + 1;
      }
    }
    correctIndex = 0;
    for (int i = BRAIN_DUEL_OPTION_COUNT - 1; i > 0; i--) {
      const int j = random(i + 1);
      const int temp = values[i]; values[i] = values[j]; values[j] = temp;
      if (correctIndex == i) correctIndex = j;
      else if (correctIndex == j) correctIndex = i;
    }
    for (uint8_t i = 0; i < BRAIN_DUEL_OPTION_COUNT; i++) {
      optionsTr[i] = String(values[i]);
      optionsEn[i] = optionsTr[i];
    }
  }

  void generateMathQuestion() {
    if (difficulty == EASY) {
      if (random(2) == 0) {
        const int a = random(3, 13);
        const int b = random(2, 21 - a);
        setMathQuestion(String(a) + " + " + String(b) + " = ?", a + b, 1 + random(3));
      } else {
        const int a = random(8, 21);
        const int b = random(2, a);
        setMathQuestion(String(a) + " - " + String(b) + " = ?", a - b, 1 + random(3));
      }
      return;
    }

    if (difficulty == NORMAL) {
      const uint8_t type = uint8_t(random(3));
      if (type == 0) {
        const int a = random(12, 70);
        const int b = random(8, 101 - a);
        setMathQuestion(String(a) + " + " + String(b) + " = ?", a + b, 3 + random(7));
      } else if (type == 1) {
        const int a = random(35, 101);
        const int b = random(8, a - 4);
        setMathQuestion(String(a) + " - " + String(b) + " = ?", a - b, 3 + random(7));
      } else {
        const int factors[] = {2, 5, 10};
        const int a = factors[random(3)];
        const int b = random(2, 10);
        setMathQuestion(String(a) + " × " + String(b) + " = ?", a * b, a);
      }
      return;
    }

    const uint8_t type = uint8_t(random(3));
    if (type == 0) {
      const int a = random(3, 10);
      const int b = random(3, 10);
      setMathQuestion(String(a) + " × " + String(b) + " = ?", a * b, 2 + random(7));
    } else if (type == 1) {
      const int divisor = random(2, 10);
      const int result = random(2, 10);
      setMathQuestion(String(divisor * result) + " ÷ " + String(divisor) + " = ?", result, 1 + random(4));
    } else {
      const int a = random(8, 25);
      const int b = random(5, 18);
      const int c = random(3, min(12, a + b));
      setMathQuestion(String(a) + " + " + String(b) + " - " + String(c) + " = ?", a + b - c, 2 + random(6));
    }
  }

  String categoryQuestionTr(uint8_t category) const {
    static const char* questions[] = {
      "Hangisi bir hayvandır?", "Hangisi bir meyvedir?", "Hangisi okulda kullanılır?",
      "Hangisi bir taşıttır?", "Hangisi doğada bulunur?", "Hangisi evde kullanılır?"
    };
    return questions[category];
  }

  String categoryQuestionEn(uint8_t category) const {
    static const char* questions[] = {
      "Which one is an animal?", "Which one is a fruit?", "Which one is used at school?",
      "Which one is a vehicle?", "Which one is found in nature?", "Which one is used at home?"
    };
    return questions[category];
  }

  uint8_t randomWordFromCategory(uint8_t category, int exceptA = -1, int exceptB = -1) const {
    const WordEntry* entries = words();
    for (uint8_t attempts = 0; attempts < 40; attempts++) {
      const uint8_t index = uint8_t(random(WORD_COUNT));
      if (index != exceptA && index != exceptB && entries[index].category == category) return index;
    }
    for (uint8_t index = 0; index < WORD_COUNT; index++) {
      if (index != exceptA && index != exceptB && entries[index].category == category) return index;
    }
    return 0;
  }

  void shuffleWordOptions(uint8_t indices[BRAIN_DUEL_OPTION_COUNT], uint8_t originalCorrect) {
    correctIndex = originalCorrect;
    for (int i = BRAIN_DUEL_OPTION_COUNT - 1; i > 0; i--) {
      const int j = random(i + 1);
      const uint8_t temp = indices[i]; indices[i] = indices[j]; indices[j] = temp;
      if (correctIndex == i) correctIndex = j;
      else if (correctIndex == j) correctIndex = i;
    }
    for (uint8_t i = 0; i < BRAIN_DUEL_OPTION_COUNT; i++) {
      optionsTr[i] = words()[indices[i]].tr;
      optionsEn[i] = words()[indices[i]].en;
    }
  }

  void generateWordQuestion() {
    if (difficulty == CHALLENGE) {
      const uint8_t commonCategory = uint8_t(random(CATEGORY_COUNT));
      uint8_t differentCategory = uint8_t(random(CATEGORY_COUNT - 1));
      if (differentCategory >= commonCategory) differentCategory++;
      uint8_t indices[BRAIN_DUEL_OPTION_COUNT];
      indices[0] = randomWordFromCategory(commonCategory);
      indices[1] = randomWordFromCategory(commonCategory, indices[0]);
      indices[2] = randomWordFromCategory(commonCategory, indices[0], indices[1]);
      indices[3] = randomWordFromCategory(differentCategory);
      promptTr = "Hangisi diğerlerinden farklıdır?";
      promptEn = "Which one is different from the others?";
      shuffleWordOptions(indices, 3);
      return;
    }

    const uint8_t targetCategory = uint8_t(random(CATEGORY_COUNT));
    uint8_t indices[BRAIN_DUEL_OPTION_COUNT];
    indices[0] = randomWordFromCategory(targetCategory);
    for (uint8_t i = 1; i < BRAIN_DUEL_OPTION_COUNT; i++) {
      uint8_t otherCategory = uint8_t(random(CATEGORY_COUNT - 1));
      if (otherCategory >= targetCategory) otherCategory++;
      indices[i] = randomWordFromCategory(otherCategory);
      for (uint8_t j = 0; j < i; j++) {
        if (indices[i] == indices[j]) {
          i--;
          break;
        }
      }
    }
    promptTr = categoryQuestionTr(targetCategory);
    promptEn = categoryQuestionEn(targetCategory);
    shuffleWordOptions(indices, 0);
  }
};
