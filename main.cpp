#pragma GCC optimize("Ofast,unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <array>
#include <time.h>

using namespace std;

///             === VARIABLES ===

const int n = 7;
const int boardSize = n*n + 1;
int maxdepth = 3;
const double Infinity = 1e9;
int dpos[] = {-7, 7, -1, 1};

int row(int i) {
    return ((i - 1)/7);
}
int col(int i) {
    return ((i - 1)%7);
}
typedef struct Piece
{
    int pos;	// board position (resp. to the above)
    int color;
    int k;      // piece index in PlacedPieces
    Piece(int a, int c, int _k) : pos(a), color(c), k(_k) {}
} Piece;

typedef struct Move
{
    string content;
    int ChaosColor;
    int OrderIndex;

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
    // A generalised collection of flags for search and pruning.
    bool unknownColour = true;
} Flag;

array<int, boardSize> board = {0};
vector<Piece> PlacedPieces;

map<array<int, boardSize>, TTEntry> TranspositionTable;

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
    cin.tie(0);
    srand(time(NULL));
    while (getline(cin, RawInput))
    {
        returnMoves.clear();
        if (RawInput == "Start")
            continue;
        if (RawInput == "Quit")
            break;
        if (RawInput.length() == 1)
        {
            colourInput = RawInput[0] - '0';
            string move = BestMoveForSide(-1);
            cout << move << endl;
            continue;
        }
        if (RawInput.length() == 3)
        {
            doMove(-1, Move(RawInput.substr(1, 2), RawInput[0] - '0', -1));
            string move = BestMoveForSide(1);
            cout << move << endl;
            continue;
        }
        if (RawInput.length() == 4)
        {
            auto isPiece = [](const Piece& p ){ return p.pos == (RawInput[0] - 'A')*7 + RawInput[1] - 'a' + 1; };
            int PieceIndex = find_if(PlacedPieces.begin(), PlacedPieces.end(), isPiece) - PlacedPieces.begin();
            doMove(1, Move(RawInput, -1, PieceIndex));
            continue;
        }
    }

    return 0;
}

double evaluate(bool GetHeuristic)
{
    double eval = 0;
    bool horizontal[boardSize][n + 1] = {0};
    bool vertical[boardSize][n + 1] = {0};
    for (int i = 1; i < boardSize; i++)
    {
        horizontal[i][1] = 1;
        vertical[i][1] = 1;
    }
    for (int k = 2; k <= n; k++)
    {
        for (int i = 1; i < boardSize; i++)
        {
            if (col(i) + k <= n && board[i] && board[i + k - 1] && board[i] == board[i + k - 1])
                horizontal[i][k] = (k > 2 ? horizontal[i + 1][k - 2] : 1);
            else
                horizontal[i][k] = 0;

            if (row(i) + k <= n && board[i] && board[i + (k - 1) * n] && board[i] == board[i + (k - 1) * n])
                vertical[i][k] = (k > 2 ? vertical[i + n][k - 2] : 1);
            else
                vertical[i][k] = 0;

            if (horizontal[i][k])
                eval += k * (GetHeuristic ? coeff[k] : 1);
            if (vertical[i][k])
                eval += k * (GetHeuristic ? coeff[k] : 1);
        }
    }
    return eval;
}

void doMove(int side, Move move)
{
    if (side > 0)
    {
        int oldPos = int(move.content[0] - 'A')*7 + int(move.content[1] - 'a') + 1;
        int newPos = int(move.content[2] - 'A')*7 + int(move.content[3] - 'a') + 1;
        swap(board[oldPos], board[newPos]);
        PlacedPieces[move.OrderIndex].pos = newPos;
    }
    else
    {
        int movePos = int(move.content[0] - 'A')*7 + int(move.content[1] - 'a') + 1;
        board[movePos] = move.ChaosColor;
        PlacedPieces.push_back(Piece(movePos, move.ChaosColor, PlacedPieces.size()));
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
        int movePos = (move.content[0] - 'A')*7 + (move.content[1] - 'a') + 1;
        board[movePos] = 0;
        PlacedPieces.pop_back();
    }
}

void showBoard()
{
    for (int i = 1; i < boardSize; i++)
    {
        if (i % 7 == 1) cout << "\n";
        cout << board[i] << " ";
    }
    return;
}

string BestMoveForSide(int color)
{
    Flag flag;
    flag.unknownColour = (color != -1);
    Search(0, color, -Infinity, Infinity, flag);
    const auto returnMove = returnMoves[0];
    doMove(color, returnMove);
    return returnMove.content;
}

void registerLegalMoves(vector<Move> &LegalMoves, int color, Flag flag)
{
    LegalMoves.clear();
    if (color > 0)
    {
        for (int k = 0; k < max((int)PlacedPieces.size(), 1); k++)
        {
            int thisPos = PlacedPieces[k].pos;
            string oldPos = string{char(row(thisPos) + 'A'), char(col(thisPos) + 'a')};

            LegalMoves.push_back(Move(oldPos + oldPos, -1, k));
            for (int i = 0; i < 4; i++)
            {
                for (int j = 1; thisPos + dpos[i] * j < boardSize && thisPos + dpos[i] * j >= 1; j++)
                {
                    if (board[thisPos + dpos[i] * j] || (abs(dpos[i]) == 1 && row(thisPos + dpos[i] * j) != row(thisPos + dpos[i] * (j - 1))))
                        break;
                    LegalMoves.push_back(Move(oldPos + string{char(row(thisPos + dpos[i] * j) + 'A'), char(col(thisPos + dpos[i] * j) + 'a')}, -1, k));
                }
            }
        }
    }
    else
    {
        for (int i = 1; i < boardSize; i++)
        {
            if (board[i])
                continue;
            if (!flag.unknownColour)
            {
                LegalMoves.push_back(Move(string{char(row(i) + 'A'), char(col(i) + 'a')}, colourInput, -1));
                continue;
            }
            for (int k = 1; k <= n; k++)
            {
                LegalMoves.push_back(Move(string{char(row(i) + 'A'), char(col(i) + 'a')}, k, -1));
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
        TranspositionTable[board] = TTEntry(evaluate(true), depth);
        return color * TranspositionTable[board].value;
    }

    double score = 0;
    vector<Move> LegalMoves;
    registerLegalMoves(LegalMoves, color, flag);

    flag.unknownColour = true;
    bool searchPVS = true;

    for (const auto &move : LegalMoves)
    {
        doMove(color, move);
        if (searchPVS)
        {
            score = -Search(depth + 1, -color, -beta, -alpha, flag);
        }else{
            score = -Search(depth + 1, -color, -alpha - 1, -alpha, flag);   // null window
            if (score > alpha)  // fail soft alpha re-search
                score = -Search(depth + 1, -color, -beta, -alpha, flag);
        }
        undoMove(color, move);

        // fail hard beta cutoff
        if (score >= beta)
            return beta;
        
        if (score > alpha)
        {
            if (depth == 0)
            {
                returnMoves = {move};
            }
            alpha = score;
            searchPVS = false;
        }
    }
    return alpha;
}
