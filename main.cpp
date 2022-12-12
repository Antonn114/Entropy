#pragma GCC optimize("Ofast,unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

#include <iostream>
#include <vector>
#include <map>
#include <time.h>
#include <string>

using namespace std;

///             === VARIABLES ===

const int maxsize = 7;
int maxdepth = 3;
int unitbonus = 2;
const double Infinity = 1e9;

typedef struct Piece
{
    int x, y;   // grid coordinates
    int color;
    int k;      // piece index in PlacedPieces
    Piece(int a, int b, int c, int _k) : x(a), y(b), color(c), k(_k) {}
} Piece;

typedef struct Move
{
    string content;
    int ChaosColor;
    int OrderIndex;
    // ! a bit arbitrary to have ChaosColor and OrderIndex in the same area.

    Move() : content(string()), ChaosColor(-1), OrderIndex(-1) {}
    Move(string m, int c, int o): content(m), ChaosColor(c), OrderIndex(o) {}
} Move;

typedef struct TTEntry
{
    double value;
    int depth;
    bool isValidEntry;
    // add more flags here

    TTEntry() : value(-1), depth(-1), isValidEntry(false) {}
    TTEntry(double v, int d) : value(v), depth(d), isValidEntry(true) {}
} TTEntry;

typedef struct Flag
{
    // A generalised collection of flags for evaluation.
    bool isSimulating;
    Flag(): isSimulating(false) {}
} Flag;

vector<vector<int>> board;
vector<Piece> PlacedPieces;

map<vector<vector<int>>, TTEntry> TranspositionTable;

int colourInput;
string RawInput;

void doMove(int color, Move move);
void undoMove(int color, Move move);

string BestMoveForSide(int color);
double evaluate(bool GetHeuristic = true);
double Search(int depth, int color, double alpha, double beta, Flag flag);


void showBoard();

vector<Move> returnMoves;
double coeff[] = {0, 0, 1.21249, 1.8809, 2.45611, 2.64828, 3.47708, 3.37076};

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

///             === DRIVER CODE ===

int main()
{
    ios_base::sync_with_stdio(false);
    srand(time(NULL));

    board.assign(maxsize, vector<int>(maxsize, 0));

    while (getline(cin, RawInput))
    {
        if (PlacedPieces.size() == 22)
            maxdepth = 2;
        returnMoves.clear();
        if (RawInput == "Start")
        {
            //maxdepth = 2;
            continue;
        }
        else if (RawInput == "Quit")
            break;
        else
        {
            if (RawInput.length() == 1)
            {
                colourInput = RawInput[0] - '0';
                string move = BestMoveForSide(-1);
                cout << move << endl;
            }
            else if (RawInput.length() == 4)
            {
                int foo = board[RawInput[0] - 'A'][RawInput[1] - 'a'];
                for (int i = 0; i < (int)PlacedPieces.size(); i++)
                {
                    if (PlacedPieces[i].x == RawInput[0] - 'A' && PlacedPieces[i].y == RawInput[1] - 'a')
                    {
                        doMove(1, Move(RawInput, -1, i));
                        break;
                    }
                }
            }
            else if (RawInput.length() == 3)
            {
                doMove(-1, Move(RawInput.substr(1, 2), RawInput[0] - '0', -1));
                string move = BestMoveForSide(1);
                cout << move << endl;
            }
        }
    }

    return 0;
}

double evaluate(bool GetHeuristic)
{
    double eval = 0;

    vector<vector<int>> dp(maxsize, vector<int>(maxsize, 0));
    for (int len = 2; len <= maxsize; len++)
    {
        for (int i = 0; i < maxsize; i++)
        {
            for (int j = 0; j + len - 1 < maxsize; j++)
            {
                bool flag = 1;
                int l = j;
                int u = j + len - 1;
                int inflatedAreas = 0;
                while (l < u)
                {
                    if (!board[i][l] || !board[i][u] || board[i][l] != board[i][u])
                    {
                        flag = 0;
                        break;
                    }

                    if (l + 1 == board[i][l])
                    {
                        inflatedAreas++;
                    }
                    if (u + 1 == board[i][u])
                    {
                        inflatedAreas++;
                    }
                    l++;
                    u--;
                }
                if (flag)
                    eval += len * (GetHeuristic ? coeff[len] : 1) + (GetHeuristic && inflatedAreas >= len - 2 ? inflatedAreas * 0.25 : 0);
            }
        }

        for (int j = 0; j < maxsize; j++)
        {
            for (int i = 0; i + len - 1 < maxsize; i++)
            {
                bool flag = 1;
                int l = i;
                int u = i + len - 1;
                int inflatedAreas = 0;
                while (l < u)
                {
                    if (!board[l][j] || !board[u][j] || board[l][j] != board[u][j])
                    {
                        flag = 0;
                        break;
                    }
                    if (j + 1 == board[l][j])
                    {
                        inflatedAreas++;
                    }
                    if (j + 1 == board[u][j])
                    {
                        inflatedAreas++;
                    }
                    l++;
                    u--;
                }
                if (flag)
                    eval += len * (GetHeuristic ? coeff[len] : 1) + (GetHeuristic && inflatedAreas >= len - 2 ? inflatedAreas * 0.25 : 0);
            }
        }
    }
    return eval;
}

void doMove(int side, Move move)
{

    if (side > 0)
    {
        pair<int, int> oldPosition = {move.content[0] - 'A', move.content[1] - 'a'};
        pair<int, int> newPosition = {move.content[2] - 'A', move.content[3] - 'a'};
        swap(board[oldPosition.first][oldPosition.second], board[newPosition.first][newPosition.second]);
        PlacedPieces[move.OrderIndex].x = newPosition.first;
        PlacedPieces[move.OrderIndex].y = newPosition.second;
    }
    else
    {
        int moveX = move.content[0] - 'A';
        int moveY = move.content[1] - 'a';
        board[moveX][moveY] = move.ChaosColor;
        PlacedPieces.push_back(Piece(moveX, moveY, move.ChaosColor, PlacedPieces.size()));
    }
    return;
}

void undoMove(int side, Move move)
{
    if (side > 0)
    {
        Move newMove = Move(move.content.substr(2, 2) + move.content.substr(0, 2), -1, move.OrderIndex);
        doMove(side, newMove);
    }
    else
    {
        int moveX = move.content[0] - 'A';
        int moveY = move.content[1] - 'a';
        board[moveX][moveY] = 0;
        PlacedPieces.pop_back();
    }
}

void showBoard()
{
    for (int i = 0; i < maxsize; i++)
    {
        for (int j = 0; j < maxsize; j++)
            cout << board[i][j];
        cout << endl;
    }
    cout << endl;
    return;
}

string BestMoveForSide(int color)
{
    Search(0, color, -Infinity, Infinity, Flag());
    const auto returnMove = returnMoves[0];
    doMove(color, returnMove);
    return returnMove.content;
}

/// MAIN SEARCH ALGORITHM

// todo: iterative deepening and expected value algorithms

void registerLegalMoves(vector<Move> &LegalMoves, int color, int depth)
{
    LegalMoves.clear();
    if (color > 0)
    {
        for (int k = 0; k < max((int)PlacedPieces.size(), 1); k++)
        { 
            string oldPosition = string{char(PlacedPieces[k].x + 'A'), char(PlacedPieces[k].y + 'a')};
            for (int i = PlacedPieces[k].x; i < maxsize && (!board[i][PlacedPieces[k].y] || PlacedPieces[k].x == i); ++i)
                LegalMoves.push_back(Move(oldPosition + string{char(i + 'A'), char(PlacedPieces[k].y + 'a')}, -1, k));
            for (int i = (PlacedPieces[k].x - 1); i >= 0 && !board[i][PlacedPieces[k].y]; --i)
                LegalMoves.push_back(Move(oldPosition + string{char(i + 'A'), char(PlacedPieces[k].y + 'a')}, -1, k));
            for (int j = (PlacedPieces[k].y + 1); j < maxsize && !board[PlacedPieces[k].x][j]; ++j)
                LegalMoves.push_back(Move(oldPosition + string{char(PlacedPieces[k].x + 'A'), char(j + 'a')}, -1, k));
            for (int j = (PlacedPieces[k].y - 1); j >= 0 && !board[PlacedPieces[k].x][j]; --j)
                LegalMoves.push_back(Move(oldPosition + string{char(PlacedPieces[k].x + 'A'), char(j + 'a')}, -1, k));
        }
    }
    else
    {
        for (int i = 0; i < maxsize; i++)
        {
            for (int j = 0; j < maxsize; j++)
            {
                if (board[i][j])
                    continue;
                if (depth == 0)
                    LegalMoves.push_back(Move(string{char(i + 'A'), char(j + 'a')}, colourInput, -1));
                else
                {
                    for (int k = 1; k <= 7; k++)
                        LegalMoves.push_back(Move(string{char(i + 'A'), char(j + 'a')}, k, -1));
                }
            }
        }
    }
    return;
}

double Search(int depth, int color, double alpha, double beta, Flag flag)
{
    if (TranspositionTable[board].isValidEntry && depth >= TranspositionTable[board].depth)
        return color * TranspositionTable[board].value;

    if (depth >= maxdepth || PlacedPieces.size() >= 49)
    {
        TranspositionTable[board] = TTEntry(evaluate(), depth);
        return color * TranspositionTable[board].value;
    }
    double score;
    vector<Move> LegalMoves;

    registerLegalMoves(LegalMoves, color, depth);

    if (color == 1)
        colourInput = 0;

    for (const auto &move : LegalMoves)
    {
        doMove(color, move);
        score = -Search(depth + 1, -color, -beta, -alpha, flag);
        undoMove(color, move);

        // fail hard beta cutoff
        if (score >= beta)
            return beta;
        
        if (score == alpha && depth == 0)
        {
            returnMoves.push_back(move);
        }
        
        if (score > alpha)
        {
            if (depth == 0)
            {
                returnMoves = {move};
            }
            alpha = score;
        }
    }
    return alpha;
}
