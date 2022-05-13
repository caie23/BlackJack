#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;

// initial percentage of winning is 50%
int game_ctr = 9;
int win_ctr = 5;

enum Rank {
    ACE = 1,  // or 11
    TWO = 2,
    THREE = 3,
    FOUR = 4,
    FIVE = 5,
    SIX = 6,
    SEVEN = 7,
    EIGHT = 8,
    NINE = 9,
    TEN = 10,
    JACK = 10,
    QUEEN = 10,
    KING = 10
};
static const Rank ranks[] = {ACE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING};

enum Type {
    CLUBS = 'C',
    DIAMONDS = 'D',
    HEARTS = 'H',
    SPADES = 'S'
};
static const Type types[] = {CLUBS, DIAMONDS, HEARTS, SPADES};

class Card {
   public:
    Card(Rank r, Type t) : rank(r), type(t) {}
    int getValue() {
        return rank;
    }
    Type getType() {
        return type;
    }
    Rank getRank() {
        return rank;
    }
    void displayCard() {
        cout << rank << (char)type << " ";
    }

   private:
    Rank rank;
    Type type;
};

class Hand {
   public:
    void add(Card c) {
        cards.push_back(c);
        total += c.getValue();
    }
    void addValue(int value) {
        total += value;
    }
    void clear() {
        cards.clear();
    }
    int getTotal() const {
        return total;
    }
    void displayCards() {
        for (Card c : cards)
            c.displayCard();
    }

   protected:
    vector<Card> cards;
    int total = 0;
};

class Deck : public Hand {
   public:
    void populate() {
        for (const auto r : ranks) {
            for (const auto k : types) {
                Card newCard(r, k);
                cards.push_back(newCard);
            }
        }
    }
    void shuffle() {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(cards.begin(), cards.end(), g);
    }
    void deal(Hand* h) {
        Card c = cards.back();
        cards.pop_back();
        h->add(c);
        if (c.getRank() == ACE) {
            if (h->getTotal() + 11 <= 21)
                h->addValue(10);  // treat Ace as 11 (add 10 more to total)
        }
    }
    void cheat(Hand* h) {
        // the computer cheats when pertage of winning < 55%
        // secretly throw away cards with large value that may bust
        Card c = cards.back();
        while (c.getRank() > 3) {
            cards.pop_back();
            c = cards.back();
        }
        h->add(c);
        cards.pop_back();
        if (c.getRank() == ACE) {
            if (h->getTotal() + 11 <= 21)
                h->addValue(10);  // treat Ace as 11 (add 10 more to total)
        }
    }
};

class AbstractPlayer : public Hand {
   public:
    virtual bool isDrawing() const = 0;  // another card?
    bool isBusted() {
        if (getTotal() > 21)
            return true;
        else
            return false;
    }
};

class ComputerPlayer : public AbstractPlayer {
   public:
    // virtual bool isDrawing() {
    virtual bool isDrawing() const override {
        if (getTotal() <= limit)
            return true;
        else
            return false;
    }
    void setLimit(int l) { this->limit = l; }

   private:
    int limit = 16;
};

class HumanPlayer : public AbstractPlayer {
   public:
    HumanPlayer(ComputerPlayer* m_casino) {
        this->m_casino = m_casino;
    }
    virtual bool isDrawing() const override {
        bool draw;
        char answer;
        cout << "Do you want to draw? (y/n): ";
        cin >> answer;
        draw = (answer == 'y' ? true : false);
        return draw;
    }
    void announce() {
        // has a push
        if (this->getTotal() == m_casino->getTotal())
            cout << "Push: No one wins." << endl;

        // wins
        else if (this->getTotal() > m_casino->getTotal())
            cout << "Player wins." << endl;

        // loses
        else {
            cout << "Casino wins." << endl;
            win_ctr++;
        }
    }

   private:
    ComputerPlayer* m_casino;
};

class BlackJackGame {
   public:
    BlackJackGame() {
        m_deck = new Deck();
        m_casino = new ComputerPlayer();
    }
    ~BlackJackGame() {
        delete m_deck;
        delete m_casino;
    }
    void play() {
        char answer;
        int numPlayer;
        cout << "How many players would you like to have? (1/2/3): ";
        cin >> answer;
        numPlayer = answer - '0';

        for (int i = 0; i < numPlayer; i++) {
            game_ctr++;

            cout << endl
                 << "Playing as player #" << i + 1 << endl
                 << endl;

            m_casino = new ComputerPlayer();
            HumanPlayer* m_human = new HumanPlayer(m_casino);

            m_deck->clear();
            m_deck->populate();
            m_deck->shuffle();

            // eacher computer is initially dealt with 1 card
            m_deck->deal(m_casino);
            cout << "Casino: ";
            m_casino->displayCards();
            cout << "[" << m_casino->getTotal() << "]" << endl;

            // each player is initially dealt with 2 cards
            m_deck->deal(m_human);
            m_deck->deal(m_human);
            cout << "Player: ";
            m_human->displayCards();
            cout << "[" << m_human->getTotal() << "]" << endl;

            // players drawing
            bool toDraw;
            char asr;
            cout << "Do you want to draw? (y/n): ";
            cin >> asr;
            toDraw = (asr == 'y' ? true : false);
            while (toDraw) {
                m_deck->deal(m_human);
                cout << "Player: ";
                m_human->displayCards();
                cout << "[" << m_human->getTotal() << "]" << endl;
                if (m_human->isBusted()) break;

                cout << "Do you want to draw? (y/n): ";
                cin >> asr;
                toDraw = (asr == 'y' ? true : false);
            }
            if (m_human->isBusted()) {
                cout << "Player busts." << endl
                     << "Casino wins." << endl;
                win_ctr++;
                continue;
            }

            // casino drawing
            if ((double)win_ctr / game_ctr < 0.55) {
                m_casino->setLimit(18);
            } else {
                m_casino->setLimit(16);
            }
            while (m_casino->isDrawing()) {
                if ((double)win_ctr / game_ctr < 0.55)
                    m_deck->cheat(m_casino);
                else
                    m_deck->deal(m_casino);

                cout << "Casino: ";
                m_casino->displayCards();
                cout << "[" << m_casino->getTotal() << "]" << endl;
            }
            if (m_casino->isBusted()) {
                cout << "Casino busts." << endl
                     << "Player wins." << endl;
                continue;
            }

            m_human->announce();
        }
    }

   private:
    Deck* m_deck;
    ComputerPlayer* m_casino;
};

int main() {
    cout << endl
         << "\tWelcome to the Comp322 Blackjack game!" << endl
         << endl;

    BlackJackGame game;

    // The main loop of the game
    bool playAgain = true;
    char answer = 'y';
    while (playAgain) {
        game.play();

        // Check whether the player would like to play another round
        cout << endl
             << "Would you like another round? (y/n): ";
        cin >> answer;
        cout << endl
             << endl;
        playAgain = (answer == 'y' ? true : false);
    }

    cout << "Gave over!" << endl
         << endl;
    return 0;
}