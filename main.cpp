#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <ncurses.h>
using namespace std;

static const char vals[15] = " A23456789TJQK"; // All possible values of a card
static const char suits[5] = "cdhs";           // All possible suits of a card
static const char piles[12][3] =               // Pile names
{"DS", "F1", "F2", "F3", "F4", "P1", "P2", "P3", "P4", "P5", "P6", "P7"};
static int drawtype;             // An int representing the draw type of the game (1 or 3)
static const int pilelens[12] =  // The lengths of each pile in GB
{25, 14, 14, 14, 14, 20, 20, 20, 20, 20, 20, 20};

// Vector struct
typedef struct
{
    int y;
    int x;
} Vector;

// Card class
class Card
{
private:
    int val;          // Number value of the card
    char suit;        // The suit of the card
    bool revealed;    // Is the card face-up?
    bool placeholder; // Is the card a placeholder?
public:
    // Constructor method for a placeholder card
    Card() 
    {
        val = 0;
        suit = ' ';
        revealed = true;
        placeholder = true;
    }

    // Constructor method for a non-placeholder card
    Card(int v, char s, bool r) // v is val, s is suit, r is revealed
    {
        val = v;
        suit = s;
        revealed = r;
        placeholder = false;
    }

    // Flips a card face up
    void reveal() 
    {
        revealed = true;
    }

    // Returns int version of the card's value
    int getIVal()
    {
        return val;
    }

    // Returns suit of the card as a char
    char getSuit() 
    {
        return suit;
    }

    // Returns char version of a card's value for use with user interaction
    char getCVal()
    {
        return vals[val];
    }

    // Returns the color of a card
    char getColor()
    {
        if(suit == 's' || suit == 'c' || getPH() || !getRevealed())
        {
            return 'b';
        }
        else if(suit == 'd' || suit == 'h')
        {
            return 'r';
        }
        return ' ';
    }

    // Returns a boolean representing whether a card is flipped
    bool getRevealed() 
    {
        return revealed;
    }

    // Returns a boolean representing if a card is a placeholder or not
    bool getPH()
    {
        return placeholder;
    }

    // Returns a boolean representing the equality of two cards
    bool equals(Card * card)
    {
        if(card->getCVal() == getCVal() && card->getSuit() == getSuit())
        {
            return true;
        }
        return false;
    }
};

class GameBoard
{
private:
    int points = 0;                     // Current score
    int maxdraw = 23;                   // An inclusive number for the max index of drawn cards
    bool drawncards[25];                // true for drawn cards, false for not
    Card * PH = new Card();             // Placeholder card to prevent segfaults
    Card ** allcards = new Card * [52]; // Original 52 cards
    Card *** GB = new Card ** [12];     // 12 rows of varying length card piles

    // Returns the last card in a given pile
    Card * last(int y)
    {
        if(y != 0)
        {
            for(int x = pilelens[y] - 1; x >= 0; x--)
            {
                if(!GB[y][x]->getPH())
                {
                    return GB[y][x];
                }
            }
            return GB[y][0];
        }
        else
        {
            for(int x = 24; x >=0; x--)
            {
                if(drawncards[x])
                {
                    return GB[0][x];
                }
            }
            return GB[0][0];
        }
    }

    Vector locationOf(Card * card)
    {
        Vector location;
        location.y = -1;
        location.x = -1;
        for(int y = 0; y < 12; y++)
        {
            for(int x = 0; x < pilelens[y]; x++)
            {
                if(GB[y][x]->equals(card))
                {
                    location.y = y;
                    location.x = x;
                    return location;
                }
            }
        }
        return location;
    }

    Vector locationOf(Card * card, int y)
    {
        Vector location;
        location.y = y;
        location.x = -1;
        for(int x = 0; x < pilelens[y]; x++)
        {
            if(GB[y][x]->equals(card))
            {
                location.x = x;
                return location;
            }
        }
        return location;
    }

    // Checks and fixes the gameboard
    void boardRefresh()
    {
        // Checks for null pointers and replaces with placeholder
        for(int y = 0; y < 12; y++)
        {
            for(int x = 0; x < pilelens[y]; x++)
            {
                if(GB[y][x] == nullptr)
                {
                    GB[y][x] = PH;
                }
            }
        }

        // Checks for reveals last cards in tableau
        for(int y = 5; y < 12; y++)
        {
            last(y)->reveal();
        }

        // Shifts all cards in discard to the left if a card is missing/moved
        int startpoint = -1;
        for(int x = 0; x <= maxdraw; x++)
        {
            if(GB[0][x]->getPH() && !GB[0][x + 1]->getPH())
            {
                startpoint = x;
                break;
            }
        }
        if(startpoint != -1)
        {
            for(int x = startpoint; x <= maxdraw; x++)
            {
                GB[0][x] = GB[0][(x + 1)];
                GB[0][(x + 1)] = PH;
            }
        }
    }

    // Makes last in drawncards false and decrease maximum cards by 1
    void decreaseDrawMax()
    {
        if(--maxdraw < -1)
        {
            maxdraw = -1;
        }
        if(maxdraw != -1)
        {
            for(int i = 24; i >= 0; i--)
            {
                if(drawncards[i])
                {
                    drawncards[i] = false;
                    break;
                }
            }
        }
    }

    // Make all items in drawncards false
    void undraw()
    {
        for(int i = 0; i < 25; i++)
        {
            drawncards[i] = false;
        }
    }
public:
    GameBoard()
    {
        // Random seed based on time
        srand(time(NULL));

        // Generate random indices
        bool repeating = true;
        int randnums[52];
        int i;
        for(i = 0; i < 52; i++)
        {
            randnums[i] = rand() % 52;
        }
        while(repeating)
        {
            repeating = false;
            for(i = 0; i < 52; i++)
            {
                for(int j = 0; j < 52; j++)
                {
                    if(j != i && randnums[j] == randnums[i])
                    {
                        randnums[j] = rand() % 52;
                        repeating = true;
                    }
                }
            }
        }

        // Create 52 cards
        i = 0;
        for(int v = 0; v < 13; v++)
        {
            for(int s = 0; s < 4; s++)
            {
                allcards[randnums[i++]] = new Card(v + 1, suits[s], false);
            }
        }

        // Add piles to GB's rows
        for(i = 0; i < 12; i++)
        {
            GB[i] = new Card * [pilelens[i]];
        }

        // Add PH to GB
        for(int y = 0; y < 12; y++)
        {
            for(int x = 0; x < pilelens[y]; x++)
            {
                GB[y][x] = PH;
            }
        }

        // Add cards to tableau
        i = 1;
        int count = 0;
        for(int y = 5; y < 12; y++, i++)
        {
            for(int x = 0; x < i; x++)
            {
                GB[y][x] = allcards[count++];
            }
        }

        // Add cards to discard
        for(i = 28; i < 52; i++)
        {
            allcards[i]->reveal();
            GB[0][i - 28] = allcards[i];
        }

        // Make all cards not drawn
        for(i = 0; i < 25; i++)
        {
            drawncards[i] = false;
        }
    }

    // Returns a bool representing whether a game is won or not
    bool isWon()
    {
        for(int y = 1; y < 5; y++)
        {
            for(int x = 0; x < 13; x++)
            {
                if(GB[y][x]->getPH())
                {
                    return false;
                }
            }
        }
        return true;
    }

    // Moves selected card to selected pile if possible and returns true if successful, false if not
    bool moveCard(int boardy, int boardx, int pileindex)
    {
        // Immediate movement disqualifiers
        if
        (
            pileindex == 0 ||
            boardy == pileindex ||
            GB[boardy][boardx]->getPH() ||
            !GB[boardy][boardx]->getRevealed() ||
            (boardy >= 1 && boardy <= 4 && boardx != 0) ||
            (boardy == 0 && !drawncards[boardx])
        )
        {
            return false;
        }
        // Checks for movement based on valid locations
        if(boardy == 0) // Movement from discard
        {
            int lastcard = -1;
            for(int i = 24; i >= 0; i--)
            {
                if(GB[0][i]->equals(last(0)))
                {
                    lastcard = i;
                }
            }
            if((lastcard > 2 && boardx == 2) || lastcard == boardx) // Valid movement from discard
            {
                if(pileindex >= 1 && pileindex <= 4) // Move to foundation
                {
                    if(last(pileindex)->getPH() && last(0)->getIVal() == 1) // Valid move
                    {
                        GB[pileindex][0] = last(0);
                        GB[locationOf(last(0)).y][locationOf(last(0)).x] = PH;
                        decreaseDrawMax();
                        return true;
                    }
                    else if(last(pileindex)->getSuit() == last(0)->getSuit() && last(pileindex)->getIVal() == last(0)->getIVal() - 1)
                    {
                        GB[pileindex][locationOf(last(pileindex)).x + 1] = last(0);
                        GB[locationOf(last(0)).y][locationOf(last(0)).x] = PH;
                        decreaseDrawMax();
                        return true;
                    }
                }
                else if(pileindex > 4) // Movement to tableau
                {
                    if(last(pileindex)->getPH() && last(boardy)->getIVal() == 13)
                    {
                        GB[pileindex][0] = last(boardy);
                        GB[locationOf(last(boardy)).y][locationOf(last(boardy)).x] = PH;
                        decreaseDrawMax();
                        return true;
                    }
                    for(int x = 0; x < pilelens[pileindex]; x++)
                    {
                        if
                        (
                            GB[pileindex][x]->getPH() && 
                            last(boardy)->getColor() != last(pileindex)->getColor() && 
                            last(boardy)->getIVal() == last(pileindex)->getIVal() - 1
                        ) // Valid movement
                        {
                            GB[pileindex][x] = last(boardy);
                            GB[locationOf(last(boardy)).y][locationOf(last(boardy)).x] = PH;
                            decreaseDrawMax();
                            return true;
                        }
                    }
                }
            }
        }
        else if(boardy >= 1 && boardy <= 4) // Movement from foundation
        {
            if(pileindex > 4) // Movement to tableau
            {
                for(int x = 0; x < pilelens[pileindex]; x++)
                {
                    if
                    (
                        GB[pileindex][x]->getPH() && 
                        last(boardy)->getColor() != last(pileindex)->getColor() && 
                        last(boardy)->getIVal() == last(pileindex)->getIVal() - 1
                    ) // Valid movement
                    {
                        GB[pileindex][x] = last(boardy);
                        GB[locationOf(last(boardy)).y][locationOf(last(boardy)).x] = PH;
                        return true;
                    }
                }
            }
        }
        else if(boardy > 4) // Movement from tableau
        {
            if(pileindex >= 1 && pileindex <= 4) // Movement to foundation
            {
                if(last(pileindex)->getPH() && last(boardy)->getIVal() == 1) // Valid move
                    {
                        GB[pileindex][0] = last(boardy);
                        GB[boardy][boardx] = PH;
                        return true;
                    }
                    else if(last(pileindex)->getSuit() == last(boardy)->getSuit() && last(pileindex)->getIVal() == last(boardy)->getIVal() - 1)
                    {
                        GB[pileindex][locationOf(last(pileindex)).x + 1] = last(boardy);
                        GB[boardy][boardx] = PH;
                        return true;
                    }
            }
            else if(pileindex > 4) // Movement to tableau
            {
                bool singlecard = false;
                if(GB[boardy][boardx + 1]->getPH())
                {
                    singlecard = true;
                }
                if(last(pileindex)->getPH() && GB[boardy][boardx]->getIVal() == 13) // Valid movement with King to empty spot
                {
                    for(int x = boardx; x < pilelens[boardy]; x++)
                    {
                        if(!GB[boardy][x]->getPH())
                        {
                            GB[pileindex][x - boardx] = GB[boardy][x];
                            GB[boardy][x] = PH;
                        }
                        else
                        {
                            break;
                        }
                    }
                    return true;
                }
                else if
                (
                    GB[boardy][boardx]->getColor() != last(pileindex)->getColor() &&
                    GB[boardy][boardx]->getIVal() == last(pileindex)->getIVal() - 1
                ) // Valid movement of non-king card
                {
                    if(singlecard)
                    {
                        GB[locationOf(last(pileindex)).y][locationOf(last(pileindex)).x + 1] = GB[boardy][boardx];
                        GB[boardy][boardx] = PH;
                        return true;
                    }
                    else // if moving multiple cards
                    {
                        for(int x = boardx; x < pilelens[boardy]; x++)
                        {
                            if(!GB[boardy][x]->getPH())
                            {
                                GB[pileindex][(locationOf(PH, pileindex).x)] = GB[boardy][x];
                                GB[boardy][x] = PH;
                            }
                            else
                            {
                                break;
                            }
                        }
                        return true;
                    }
                }
            }
        }

        return false;
    }

    // Deallocates all pointers
    void deallocate()
    {
        delete PH;

        for(int i = 0; i < 52; i++)
        {
            delete allcards[i];
        }
        delete[] allcards;

        for(int i = 0; i < 12; i++)
        {
            delete[] GB[i];
        }
        delete[] GB;
    }
    
    // Draws 1 or 3 cards
    void draw()
    {
        int startpoint = -1;
        for(int i = 0; i <= maxdraw; i++) // Find where the undrawn card is
        {
            if(!drawncards[i])
            {
                startpoint = i;
                break;
            }
        }
        if(startpoint != -1) // Draw 1 or 3 cards
        {
            for(int i = startpoint; i < startpoint + drawtype; i++)
            {
                if(i <= maxdraw)
                {
                    drawncards[i] = true;
                }
                else
                {
                    return;
                }
            }
        }
        else // Put all cards back in the deck
        {
            undraw();
        }
    }

    // Prints all items in the gameboard
    void printGB(int boardy, int boardx, bool pilesel, int pileindex)
    {
        boardRefresh();

        // Print discard
        int colorpair = 1;
        if(pilesel && pileindex == 0)
        {
            colorpair = 3;
        }
        attroff(COLOR_PAIR(1));
        attron(COLOR_PAIR(colorpair));
        mvprintw(0, 0, "%s", piles[0]);
        attroff(COLOR_PAIR(colorpair));
        attron(COLOR_PAIR(1));

        printw(" [  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ]");

        Card ** discardprint = new Card * [3];
        for(int i = 24, count = 2; i >= 0 && count >= 0; i--)
        {
            if(drawncards[i] || i < 3)
            {
                if(drawncards[i])
                {
                    discardprint[count--] = GB[0][i];
                }
                else
                {
                    discardprint[count--] = PH;
                }
            }
        }

        for(int i = 0; i < 3; i++)
        {
            if(boardx == i && boardy == 0)
            {
                if(discardprint[i]->getColor() == 'b')
                {
                    colorpair = 3;
                }
                else
                {
                    colorpair = 4;
                }
            }
            else
            {
                if(discardprint[i]->getColor() == 'b')
                {
                    colorpair = 1;
                }
                else
                {
                    colorpair = 2;
                }
            }
            attroff(COLOR_PAIR(1));
            attron(COLOR_PAIR(colorpair));
            mvprintw(0, 4 + 4 * i, "%c%c", discardprint[i]->getCVal(), discardprint[i]->getSuit());
            attroff(COLOR_PAIR(colorpair));
            attron(COLOR_PAIR(1));
        }
        delete[] discardprint;

        for(int i = 3; i < 19; i++)
        {
            if(boardx == i && boardy == 0)
            {
                colorpair = 3;
            }
            else
            {
                colorpair = 1;
            }
            attroff(COLOR_PAIR(1));
            attron(COLOR_PAIR(colorpair));
            mvprintw(0, 4 + 4 * i, "  ");
            attroff(COLOR_PAIR(colorpair));
            attron(COLOR_PAIR(1));
        }

        // Print foundation
        for(int i = 1; i < 5; i++)
        {
            colorpair = 1;
            if(pilesel && pileindex == i)
            {
                colorpair = 3;
            }
            attroff(COLOR_PAIR(1));
            attron(COLOR_PAIR(colorpair));
            mvprintw(i, 0, "%s", piles[i]);
            attroff(COLOR_PAIR(colorpair));
            attron(COLOR_PAIR(1));

            printw(" [  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ]");
            for(int n = 0; n < 19; n++)
            {
                Card * printcard;
                if(n > 0)
                {
                    printcard = PH;
                }
                else
                {
                    printcard = last(i);
                }
                if(n == boardx && i == boardy)
                {
                    if(printcard->getColor() == 'b')
                    {
                        colorpair = 3;
                    }
                    else
                    {
                        colorpair = 4;
                    }
                }
                else
                {
                    if(printcard->getColor() == 'b' || !printcard->getRevealed())
                    {
                        colorpair = 1;
                    }
                    else
                    {
                        colorpair = 2;
                    }
                }
                attroff(COLOR_PAIR(1));
                attron(COLOR_PAIR(colorpair));
                if(printcard->getRevealed())
                {
                    mvprintw(i, 4 + 4 * n, "%c%c", printcard->getCVal(), printcard->getSuit());
                }
                attroff(COLOR_PAIR(colorpair));
                attron(COLOR_PAIR(1));
            }
        }

        // Print tableau
        for(int i = 5; i < 12; i++)
        {
            colorpair = 1;
            if(pilesel && pileindex == i)
            {
                colorpair = 3;
            }
            attroff(COLOR_PAIR(1));
            attron(COLOR_PAIR(colorpair));
            mvprintw(i, 0, "%s", piles[i]);
            attroff(COLOR_PAIR(colorpair));
            attron(COLOR_PAIR(1));

            printw(" [  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ][  ]");
            for(int n = 0; n < 19; n++)
            {
                Card * printcard;
                if(n > pilelens[i] - 1)
                {
                    printcard = PH;
                }
                else
                {
                    printcard = GB[i][n];
                }
                if(n == boardx && i == boardy)
                {
                    if(printcard->getColor() == 'b')
                    {
                        colorpair = 3;
                    }
                    else
                    {
                        colorpair = 4;
                    }
                }
                else
                {
                    if(printcard->getColor() == 'b' || !printcard->getRevealed())
                    {
                        colorpair = 1;
                    }
                    else
                    {
                        colorpair = 2;
                    }
                }
                attroff(COLOR_PAIR(1));
                attron(COLOR_PAIR(colorpair));
                if(printcard->getRevealed())
                {
                    mvprintw(i, 4 + 4 * n, "%c%c", printcard->getCVal(), printcard->getSuit());
                }
                else
                {
                    mvprintw(i, 4 + 4 * n, "--");
                }
                attroff(COLOR_PAIR(colorpair));
                attron(COLOR_PAIR(1));
            }
        }
    }
};

class Cursor 
{
private:
    int xpos = 0;
    int ypos = 0;
    int xmax;
    int ymax;
public:
    Cursor(int ybound, int xbound) // xbound and ybound are non-inclusive maximums of the x and y values
    {
        xmax = --xbound;
        ymax = --ybound;
    }
    void moveDown()
    {
        ypos++;
        if(ypos > ymax)
        {
            ypos = 0;
        }
    }
    void moveUp() 
    {
        ypos--;
        if(ypos < 0)
        {
            ypos = ymax;
        }
    }
    void moveRight()
    {
        xpos++;
        if(xpos > xmax) 
        {
            xpos = 0;
        }
    }
    void moveLeft() 
    {
        xpos--;
        if(xpos < 0)
        {
            xpos = xmax;
        }
    }
    int getX()
    {
        return xpos;
    }
    int getY()
    {
        return ypos;
    }
};

enum Key 
{ 
    uarrow = 259, 
    darrow = 258, 
    larrow = 260, 
    rarrow = 261, 
    spacebar = 32, 
    d = 100, 
    e = 101,
    y = 121,
    Y = 89,
    one = 49, 
    three = 51 
};

int main() 
{
    // Initialize ncurses terminal mode
    initscr();
    if(has_colors() == FALSE)
    {    
        endwin();
        cout << "Your terminal does not support color\n";
        exit(1);
    }
    start_color();
    
    // Initializes the 4 different color modes
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // Default white text on black background
    init_pair(2, COLOR_RED, COLOR_BLACK);   // Used for printing red cards
    init_pair(3, COLOR_BLACK, COLOR_WHITE); // Selected white card
    init_pair(4, COLOR_WHITE, COLOR_RED);   // Selected red card

    attron(COLOR_PAIR(1));
    raw();
    keypad(stdscr, true);
    noecho();
    
    // Create gameboard, cursors, and key input
    GameBoard * board = new GameBoard();
    Cursor * cardcursor = new Cursor(12, 19);
    Cursor * pilecursor = new Cursor(12, 1);
    Key input;

    // Prompt for game type (draw 3 vs draw 1)
    do 
    {
        printw("Would you like to play draw 3 or draw 1? (1/3)");
        refresh();
        input = (Key) getch();
        clear();
    } while(input != Key::one && input != Key::three);

    if(input == Key::one)
    {
        drawtype = 1;
    }
    else
    {
        drawtype = 3;
    }

    input = Key::d;                     // First turn command is draw
    Key checkexit;                      // Check if you want to exit
    char * gamemessage = (char *) "\n"; // Game message that details user or programmer error
    bool first_turn = true;             // Game starts on turn 1
    bool cardmode = true;               // Select cards in gameboard if true, piles if not
    bool firstmove = true;              // Don't move any cards before the first space press
    bool endgame = false;               // false if game is still going, true if foundation is full
    bool win = false;                   // true if you won the game, false if the game ended prematurely
    while(!endgame)
    {
        board->printGB(cardcursor->getY(), cardcursor->getX(), !cardmode, pilecursor->getY());
        mvprintw(12, 0, "%s\n", gamemessage);
        gamemessage = (char *) "";
        refresh();
        if(board->isWon())
        {
            win = true;
            endgame = true;
            break;
        }
        if(!first_turn)
        {
            input = (Key) getch();
        }
        else
        {
            first_turn = false;
        }

        switch(input)
        {
            case Key::y:
            case Key::Y:
            default:
                break;
            case Key::d:
                if(cardmode)
                {
                    board->draw();
                }
                break;
            case Key::e:
                clear();
                printw("Are you sure you want to exit? (y/N)");
                refresh();
                checkexit = (Key) getch();
                if(checkexit == Key::y || checkexit == Key::Y)
                {
                    endgame = true;
                }
                break;
            case Key::uarrow:
                if(cardmode)
                {
                    cardcursor->moveUp();
                }
                else
                {
                    pilecursor->moveUp();
                }
                break;
            case Key::darrow:
                if(cardmode)
                {
                    cardcursor->moveDown();
                }
                else
                {
                    pilecursor->moveDown();
                }
                break;
            case Key::larrow:
                if(cardmode)
                {
                    cardcursor->moveLeft();
                }
                break;
            case Key::rarrow:
                if(cardmode)
                {
                    cardcursor->moveRight();
                }
                break;
            case Key::spacebar:
                if(cardmode)
                {
                    cardmode = false;
                }
                else if(!first_turn)
                {
                    cardmode = true;
                    if(!board->moveCard(cardcursor->getY(), cardcursor->getX(), pilecursor->getY()))
                    {
                        gamemessage = (char *) "Invalid move";
                    }
                    else
                    {
                        gamemessage = (char *) "";
                    }
                }
                break;
        }
    }

    // Ending operations
    endwin();
    board->deallocate();
    delete board;
    delete cardcursor;
    delete pilecursor;

    if(win)
    {
        cout << "Hurray you won!";
    }

    return 0;
}
