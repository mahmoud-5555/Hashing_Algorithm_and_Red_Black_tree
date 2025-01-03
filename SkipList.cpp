#include <iostream>
#include <random>
#include <string>
using namespace std;

template <typename T>
class SkipListNode
{
private:
    int levelCount;
    SkipListNode<T> **forward;

public:
    T value;
    int getLevelCount() const
    {
        return levelCount;
    }
    SkipListNode(T value, int levelCount)
    {
        forward = new SkipListNode<T> *[levelCount];
        this->levelCount = levelCount;
        this->value = value;
    }
    SkipListNode<T> *next(int level = 0) const
    {
        if (level < 0 || level >= levelCount)
        {
            throw out_of_range("Level is out of valid range.");
        }
        return forward[level];
    }
    void insertAfter(SkipListNode<T> *newNode, int level)
    {
        if (level < 0 || level >= levelCount || level >= newNode->levelCount)
        {
            throw out_of_range("Level is out of valid range.");
        }
        newNode->forward[level] = forward[level];
        forward[level] = newNode;
    }
    void setNext(SkipListNode<T> *next, int level)
    {
        if (level < 0 || level >= levelCount)
        {
            throw out_of_range("Level is out of valid range.");
        }
        forward[level] = next;
    }
    operator T() const
    {
        return value;
    }
    ~SkipListNode()
    {
        delete[] forward;
    }
};
template <typename T>
struct SkipListHeadNode
{
    SkipListNode<T> *node = nullptr;
    SkipListHeadNode<T> *nextLevelHead = nullptr;
    SkipListHeadNode<T> *prevLevelHead = nullptr;
};
template <typename T>
class SkipListHead
{
    int levelCount = 1;
    SkipListHeadNode<T> *head;
    SkipListHeadNode<T> *last;
    void addLevel()
    {
        levelCount++;
        last->nextLevelHead = new SkipListHeadNode<T>();
        last->nextLevelHead->prevLevelHead = last;
        last = last->nextLevelHead;
    }
    SkipListHeadNode<T> *getLevel(int level)
    {
        if (level >= levelCount || level < 0)
        {
            throw out_of_range("Level is out of valid range.");
        }
        int currentLevel = 0;
        SkipListHeadNode<T> *current = head;
        while (currentLevel < level)
        {
            currentLevel++;
            current = current->nextLevelHead;
        }
        return current;
    }
    void ensureLevelExist(int level)
    {
        while (this->levelCount <= level)
        {
            addLevel();
        }
    }

public:
    int getLevelCount() const
    {
        return levelCount;
    }
    SkipListHeadNode<T> *getFirstLevelHead() const
    {
        return head;
    }
    SkipListHeadNode<T> *getLastLevelHead() const
    {
        return last;
    }
    SkipListHeadNode<T> *insertToHead(SkipListNode<T> *node, int &level)
    {
        level = node->getLevelCount() - 1;
        ensureLevelExist(level);
        SkipListHeadNode<T> *h = getLevel(level);
        while (h && (!h->node || h->node->value > node->value))
        {
            node->setNext(h->node, level);
            h->node = node;
            level--;
            h = h->prevLevelHead;
        }
        return h;
    }
    SkipListHeadNode<T> *getSearchHead(T value, int &level, bool printPath = false) const
    {
        level = levelCount - 1;
        SkipListHeadNode<T> *searchHead = last;
        while (searchHead && (!searchHead->node || searchHead->node->value > value))
        {
            if (printPath)
            {
                cout << "Level: " << level << ", next: ";
                if (searchHead->node)
                {
                    cout << *(searchHead->node) << endl;
                }
                else
                {
                    cout << "Null" << endl;
                }
            }

            level--;
            searchHead = searchHead->prevLevelHead;
        }
        return searchHead;
    }
    SkipListHeadNode<T> *removeFromHead(T value, int &searchLevel, SkipListNode<T> **deletedNode)
    {
        SkipListHeadNode<T> *searchHead = getSearchHead(value, searchLevel);
        *deletedNode = nullptr;
        while (searchHead && searchHead->node && searchHead->node->value == value)
        {
            *deletedNode = searchHead->node;
            searchHead->node = searchHead->node->next(searchLevel);
            (*deletedNode)->setNext(nullptr, searchLevel);
            searchLevel--;
            searchHead = searchHead->prevLevelHead;
        }
        return searchHead;
    }
    SkipListHead()
    {
        head = new SkipListHeadNode<T>();
        last = head;
    }
    ~SkipListHead()
    {
        SkipListHeadNode<T> *current = head;
        while (current)
        {
            SkipListHeadNode<T> *temp = current->nextLevelHead;
            delete current;
            current = temp;
        }
    }
};

template <typename T>
class SkipList
{
private:
    int count = 0;
    SkipListHead<T> head;
    random_device rd;
    mt19937 gen;
    uniform_int_distribution<> distr;
    bool shouldGoUp()
    {
        return distr(gen) > 500;
    }
    int getRandomLevelCount()
    {
        int levelCount = 1;
        while (shouldGoUp())
        {
            levelCount++;
        }
        return levelCount;
    }

public:
    int getCount() const
    {
        return count;
    }
    bool remove(T value)
    {
        int searchLevel;
        SkipListNode<T> *deletedNode = nullptr;
        SkipListHeadNode<T> *searchHead = head.removeFromHead(value, searchLevel, &deletedNode);
        SkipListNode<T> *searchNode = searchHead ? searchHead->node : nullptr;
        while (searchNode && searchLevel >= 0)
        {
            SkipListNode<T> *nextNode = searchNode->next(searchLevel);
            if (nextNode && nextNode->value == value)
            {

                deletedNode = nextNode;
                searchNode->setNext(nextNode->next(searchLevel), searchLevel);
                deletedNode->setNext(nullptr, searchLevel);
                searchLevel--;
            }

            else if (nextNode && nextNode->value < value)
            {
                searchNode = nextNode;
            }

            else
            {
                searchLevel--;
            }
        }

        if (deletedNode)
        {

            count--;
            delete deletedNode;
            return true;
        }
        return false;
    }
    const SkipListNode<T> *find(T value, bool printPath = false) const
    {
        int searchLevel;
        SkipListHeadNode<T> *searchHead = head.getSearchHead(value, searchLevel, printPath);
        if (!searchHead || searchLevel < 0)
        {
            return nullptr;
        }
        SkipListNode<T> *searchNode = searchHead->node;
        while (searchNode && searchLevel >= 0)
        {

            const int maxSearchLevel = searchNode->getLevelCount() - 1;
            searchLevel = maxSearchLevel < searchLevel ? maxSearchLevel : searchLevel;
            SkipListNode<T> *next = searchNode->next(searchLevel);
            if (printPath)
            {
                cout << "Level: " << searchLevel << ", visiting: " << *searchNode << endl;
                cout << "Level: " << searchLevel << ", next: ";
                if (next)
                {
                    cout << *next << endl;
                }
                else
                {
                    cout << "Null" << endl;
                }
            }
            if (searchNode->value == value)
            {
                return searchNode;
            }
            else if (searchLevel == 0 && next && next->value > value)
            {
                return nullptr;
            }
            while ((searchLevel > 0) && (!next || next->value > value))
            {
                searchLevel--;
                next = searchNode->next(searchLevel);
                if (printPath)
                {
                    if (next)
                    {
                        cout << "Level: " << searchLevel << ", next: " << *next << endl;
                    }
                    else
                    {
                        cout << "Level: " << searchLevel << ", next: Null" << endl;
                    }
                }
            }
            searchNode = next;
        }

        return nullptr;
    }
    void printPathTo(T value) const
    {
        cout << "Searching for " << value << ":" << endl;
        const SkipListNode<T> *r = find(value, true);
        if (r)
        {
            cout << value << " found as " << *r << endl;
        }
        else
        {
            cout << value << " not found!" << endl;
        }
    }

    SkipListNode<T> *insert(T value)
    {
        if (find(value))
        {
            return nullptr;
        }

        count++;
        const int levelCount = getRandomLevelCount();
        int level = levelCount - 1;
        SkipListNode<T> *node = new SkipListNode<T>(value, levelCount);
        SkipListHeadNode<T> *insertionHead = head.insertToHead(node, level);
        if (!insertionHead)
        {
            return node;
        }
        SkipListNode<T> *searchNode = insertionHead->node;
        while (searchNode && (level >= 0))
        {
            while (searchNode->next(level) && searchNode->next(level)->value <= value)
            {
                searchNode = searchNode->next(level);
            }

            while ((level >= 0) && (!searchNode->next(level) || searchNode->next(level)->value > value))
            {
                searchNode->insertAfter(node, level);
                level--;
            }
        }
        return node;
    }
    void printFullList() const
    {
        if (count == 0)
        {
            cout << "Skip list is empty." << endl
                 << endl;
            return;
        }
        cout << "Skip list has: " << endl;
        cout << head.getLevelCount() << " levels" << endl;
        cout << count << " items" << endl
             << endl;
        SkipListHeadNode<T> *currentHead = head.getLastLevelHead();
        int level = head.getLevelCount() - 1;

        while (currentHead)
        {
            cout << "Level " << level << ": ";

            SkipListNode<T> *node = currentHead->node;

            while (node)
            {
                cout << *node << " ";
                node = node->next(level); // Move to the next node at this level
            }

            cout << endl;

            // Move to the next lower level
            currentHead = currentHead->prevLevelHead;
            level--;
        }
        cout << endl;
    }
    void printItems() const
    {
        cout << "List: ";
        SkipListNode<T> *node = head.getFirstLevelHead()->node;
        if (!node)
        {
            cout << "empty" << endl;
            return;
        }
        while (node)
        {
            cout << *node << (node->next() ? ", " : ".");
            node = node->next();
        }
        cout << endl;
    }
    const SkipListNode<T> *getFirstNode()
    {
        return head.getFirstLevelHead()->node;
    }
    bool exists(T value)
    {
        const SkipListNode<T> *node = find(value);
        return node;
    }
    SkipList() : gen(rd()), distr(1, 1000) {}
    template <typename Container>
    SkipList(const Container &array) : gen(rd()), distr(1, 1000)
    {
        for (const auto &value : array)
        {
            insert(value);
        }
    }
    ~SkipList()
    {
        SkipListNode<T> *node = head.getFirstLevelHead()->node;
        while (node)
        {
            SkipListNode<T> *temp = node->next();
            delete node;
            node = temp;
        }
    }
};
void taskA()
{
    cout << "Skip List: " << endl;
    SkipList<int> skipList;
    bool run = true;
    while (run)
    {
        cout << "current skip list:" << endl;
        skipList.printFullList();
        cout << "Select action:" << endl;
        cout << "1. Insert:" << endl;
        cout << "2. Delete:" << endl;
        cout << "3. find:" << endl;
        cout << "4. Exit:" << endl;
        int selection;
        cin >> selection;
        switch (selection)
        {
        case 1:
        {
            int number;
            cout << "Enter a number to insert: ";
            cin >> number;
            SkipListNode<int> *r = skipList.insert(number);
            if (r)
            {
                cout << *r << " is inserted up to level " << r->getLevelCount();
                if (r->next())
                {
                    cout << "  before item: " << *(r->next()) << endl;
                }
                else
                {

                    cout << "  at end of list" << endl;
                }
            }
            else
            {
                cout << number << " alread exists in list!" << endl;
            }
        }
        break;
        case 2:
        {
            int number;
            cout << "Enter a number to delete: ";
            cin >> number;
            if (skipList.remove(number))
            {
                cout << "removed " << number << "." << endl;
            }
            else
            {
                cout << number << " does not exist!" << endl;
            }
        }
        break;
        case 3:
        {
            int number;
            cout << "Enter a number to search for: ";
            cin >> number;
            skipList.printPathTo(number);
        }
        break;
        case 4:
            cout << "Exiting" << endl;
            run = false;
            break;
        default:
            cout << "Invalid selection (" << selection << ")!" << endl;
            break;
        }
    }
}
struct Score
{
    unsigned int score;
    string name;
    bool compareByScore;
    Score(unsigned int score = 0, string name = "", bool compareByScore = true)
        : score(score), name(name), compareByScore(compareByScore) {}
    bool operator==(const Score &other) const
    {
        if (compareByScore && this->score != other.score)
        {
            return false;
        }
        return this->name == other.name;
    }
    bool operator<(const Score &other) const
    {
        if (compareByScore && this->score != other.score)
        {
            return this->score < other.score;
        }
        return this->name < other.name;
    }
    bool operator>(const Score &other) const
    {
        if (compareByScore && this->score != other.score)
        {
            return this->score > other.score;
        }
        return this->name > other.name;
    }
    bool operator<=(const Score &other) const
    {
        return (*this < other) || (*this == other);
    }
    bool operator>=(const Score &other) const
    {
        return (*this > other) || (*this == other);
    }

    bool operator!=(const Score &other) const
    {
        return !(*this == other);
    }
    friend ostream &operator<<(ostream &out, const Score &score);
};
ostream &operator<<(ostream &out, const Score &score)
{
    out << "Name: " << score.name << ", Score: " << score.score;
    return out;
}
class ScoreManager
{
private:
    SkipList<Score> scoresByScore;
    SkipList<Score> scoresByName;

public:
    bool addScore(string name, unsigned int score)
    {
        Score byScore(score, name, true);
        Score byName(score, name, false);
        if (scoresByName.exists(byName))
        {
            cout << "Player " << name << " already exists!" << endl;
            return false;
        }
        scoresByScore.insert(byScore);
        scoresByName.insert(byName);
        return true;
    }
    int getScore(string name)
    {
        Score byName(0, name, false);
        const SkipListNode<Score> *node = scoresByName.find(byName);
        if (!node)
        {
            return -1;
        }
        return node->value.score;
    }
    bool removeScore(string name)
    {
        Score byName(0, name, false);
        const SkipListNode<Score> *record = scoresByName.find(byName);
        if (record)
        {
            int score = record->value.score;
            Score byScore(score, name, true);
            scoresByName.remove(byName);
            scoresByScore.remove(byScore);
            return true;
        }
        return false;
    }
    bool updatePlayeScore(string name, unsigned int newScore)
    {
        if (!removeScore(name))
        {
            return false;
        }
        return addScore(name, newScore);
    }
    void printScores()
    {
        scoresByScore.printItems();
    }
    void printTopPlayers(unsigned int count)
    {
        if (scoresByScore.getCount() == 0)
        {
            cout << "No players found" << endl;
        }

        else if (count > scoresByScore.getCount())
        {
            cout << "Only " << scoresByScore.getCount() << " exists!" << endl;
            cout << "Top " << scoresByScore.getCount() << ": ";
            printScores();
        }
        else
        {
            cout << "Top " << count << ":" << endl;
            const SkipListNode<Score> *node = scoresByScore.getFirstNode();
            const int printIndex = scoresByScore.getCount() - count;
            int index = 0;
            while (node)
            {
                if (index >= printIndex)
                {
                    cout << *node << ",, ";
                }

                node = node->next();
                index++;
            }
            cout << endl;
        }
    }
};

void taskB()
{
    cout << "Score Management: " << endl;
    ScoreManager scoreManager;
    bool run = true;
    while (run)
    {
        cout << "Current scores:" << endl;
        scoreManager.printScores();
        cout << "Select action:" << endl;
        cout << "1. add player:" << endl;
        cout << "2. delete player:" << endl;
        cout << "3. get player score:" << endl;
        cout << "4. update player:" << endl;
        cout << "5. get top players:" << endl;
        cout << "6. Exit:" << endl;
        char choice;
        cin >> choice;
        switch (choice)
        {
        case '1':
        {
            cout << "Enter Player Name: ";
            string name;
            cin >> name;
            cout << "Enter Player Score: ";
            unsigned int score;
            cin >> score;
            if (scoreManager.addScore(name, score))
            {
                cout << "Player inserted." << endl;
            }
            else
            {
                cout << "Player " << name << " not inserted." << endl;
            }
            break;
        }
        case '2':
        {

            cout << "Enter Player Name: ";
            string name;
            cin >> name;
            if (scoreManager.removeScore(name))
            {
                cout << "player removed" << endl;
            }
            else
            {
                cout << "Player " << name << " not removed." << endl;
            }
            break;
        }
        case '3':
        {
            cout << "Enter Player Name: ";
            string name;
            cin >> name;
            int score = scoreManager.getScore(name);
            if (score >= 0)
            {
                cout << "Score: " << score << endl;
            }
            else
            {
                cout << "Player " << name << " does not exist!" << endl;
            }
            break;
        }
        case '4':
        {
            cout << "Enter Player Name: ";
            string name;
            cin >> name;
            cout << "Enter new Score: ";
            unsigned int newScore;
            cin >> newScore;
            if (scoreManager.updatePlayeScore(name, newScore))
            {
                cout << "Player Score Updated" << endl;
            }
            else
            {
                cout << "Player " << name << " not updated." << endl;
            }
            break;
        }
        case '5':
        {
            cout << "Enter Number of Players: ";
            unsigned int num;
            cin >> num;
            scoreManager.printTopPlayers(num);
            break;
        }
        case '6':
            cout << "Exiting" << endl;
            run = false;
            break;

        default:
            cout << "Invalid selection!" << endl;
            break;
        }
    }
}
int main()
{
    cout << "Task 3: " << endl;
    cout << "Select:" << endl;
    cout << "A. Skip List" << endl;
    cout << "B. Score Management" << endl;
    char choice;
    cin >> choice;
    switch (choice)
    {
    case 'A':
    case 'a':
        taskA();
        break;
    case 'B':
    case 'b':
        taskB();
        break;
    default:
        main();
        break;
    }
}