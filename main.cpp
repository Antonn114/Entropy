#pragma GCC optimize("Ofast,unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <array>

#define CHAOS -1
#define ORDER 1

using namespace std;

const int n = 7;
const int boardSize = n * n + 1;
const double Infinity = 1e9;
const int dpos[] = {-7, 7, -1, 1};
const int maxdepth = 3;

int row(int i)
{
    return ((i - 1) / 7);
}
int col(int i)
{
    return ((i - 1) % 7);
}
int CodeToPosition(string code)
{
    return int(code[0] - 'A') * 7 + int(code[1] - 'a') + 1;
}

array<string, boardSize> PositionToCode{"NIL", "Aa", "Ab", "Ac", "Ad", "Ae", "Af", "Ag", "Ba", "Bb", "Bc", "Bd", "Be", "Bf", "Bg", "Ca", "Cb", "Cc", "Cd", "Ce", "Cf", "Cg", "Da", "Db", "Dc", "Dd", "De", "Df", "Dg", "Ea", "Eb", "Ec", "Ed", "Ee", "Ef", "Eg", "Fa", "Fb", "Fc", "Fd", "Fe", "Ff", "Fg", "Ga", "Gb", "Gc", "Gd", "Ge", "Gf", "Gg"};

typedef struct Piece
{
    int pos; // board position (resp. to the above)
    int side;
    int k; // piece index in PlacedPieces

    Piece(int a, int c, int _k) : pos(a), side(c), k(_k) {}
} Piece;

typedef struct Move
{
    string content;
    int flag;

    Move() : content(string()), flag(-1) {}
    Move(string m, int f) : content(m), flag(f) {}
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
    bool isRandomColor = true;
} Flag;

array<int, boardSize> board = {0};
vector<Piece> PlacedPieces;

int colourInput;
string RawInput;

void doMove(int side, Move move);
void undoMove(int side, Move move);

string BestMoveForSide(int side);
double evaluate(bool GetHeuristic = true);
double Search(int depth, int side, double alpha, double beta, Flag flag);

int PVLength[boardSize];
Move PVTable[boardSize][boardSize];

void showBoard();

double coeff[] = {0, 0, 1.21249, 1.8809, 2.45611, 2.64828, 3.47708, 3.77076};

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    while (getline(cin, RawInput))
    {
        if (RawInput == "Start")
            continue;
        if (RawInput == "Quit")
            break;
        if (RawInput.length() == 1)
        {
            colourInput = RawInput[0] - '0';
            string move = BestMoveForSide(CHAOS);
            cout << move << endl;
            continue;
        }
        if (RawInput.length() == 3)
        {
            doMove(CHAOS, Move(RawInput.substr(1, 2), RawInput[0] - '0'));
            string move = BestMoveForSide(ORDER);
            cout << move << endl;
            continue;
        }
        if (RawInput.length() == 4)
        {
            auto isPiece = [](const Piece &p)
            { return p.pos == CodeToPosition(RawInput); };
            int PieceIndex = find_if(PlacedPieces.begin(), PlacedPieces.end(), isPiece) - PlacedPieces.begin();
            doMove(ORDER, Move(RawInput, PieceIndex));
            continue;
        }
    }

    return 0;
}

string BestMoveForSide(int side)
{
    Flag flag;
    flag.isRandomColor = (side != -1);
    double score = Search(0, side, -Infinity, Infinity, flag);
    // cerr << "score: " << score << " "
    //      << "pv: ";
    // for (int i = 0; i < PVLength[0]; i++)
    // {
    //     cerr << PVTable[0][i].flag << PVTable[0][i].content << " ";
    // }
    // // e.g: score: -5 pv: 12CcCb 2Aa 2EcEb
    // cerr << endl;
    doMove(side, PVTable[0][0]);
    return PVTable[0][0].content;
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
    if (side == ORDER)
    {
        int oldPos = CodeToPosition(move.content.substr(0, 2));
        int newPos = CodeToPosition(move.content.substr(2, 2));
        swap(board[oldPos], board[newPos]);
        PlacedPieces[move.flag].pos = newPos;
    }
    else
    {
        int movePos = CodeToPosition(move.content);
        board[movePos] = move.flag;
        PlacedPieces.push_back(Piece(movePos, move.flag, PlacedPieces.size()));
    }
    return;
}

void undoMove(int side, Move move)
{
    if (side == ORDER)
    {
        Move newMove = Move(move.content.substr(2, 2) + move.content.substr(0, 2), move.flag);
        doMove(side, newMove);
    }
    else
    {
        board[CodeToPosition(move.content)] = 0;
        PlacedPieces.pop_back();
    }
    return;
}

void showBoard()
{
    for (int i = 1; i < boardSize; i++)
    {
        if (i % 7 == 1)
            cerr << "\n";
        cerr << board[i] << " ";
    }
    cerr << "\n";
    return;
}

void registerMovesForChaos(vector<Move> &LegalMoves, Flag flag)
{
    for (int i = 1; i < boardSize; i++)
    {
        if (board[i])
            continue;
        if (!flag.isRandomColor)
        {
            LegalMoves.push_back(Move(PositionToCode[i], colourInput));
            continue;
        }
        for (int k = 1; k <= n; k++)
        {
            LegalMoves.push_back(Move(PositionToCode[i], k));
        }
    }
    return;
}

void registerMovesForOrder(vector<Move> &LegalMoves, Flag flag)
{
    for (int k = 0; k < (int)PlacedPieces.size(); k++)
    {
        int fromPosition = PlacedPieces[k].pos;
        LegalMoves.push_back(Move(PositionToCode[fromPosition] + PositionToCode[fromPosition], k));
        for (int i = 0; i < 4; i++)
        {
            int toPosition = fromPosition + dpos[i];
            while (!board[toPosition] && toPosition < boardSize && toPosition >= 1)
            {
                if (abs(dpos[i]) == 1 && row(toPosition) != row(toPosition - dpos[i]))
                    break;
                LegalMoves.push_back(Move(PositionToCode[fromPosition] + PositionToCode[toPosition], k));
                toPosition += dpos[i];
            }
        }
    }
    return;
}

double Search(int depth, int side, double alpha, double beta, Flag flag)
{
    PVLength[depth] = depth;
    if (depth >= maxdepth || PlacedPieces.size() >= 49)
    {
        return side * evaluate();
    }

    vector<Move> LegalMoves;
    if (side == ORDER)
        registerMovesForOrder(LegalMoves, flag);
    else
        registerMovesForChaos(LegalMoves, flag);

    double score = 0;
    flag.isRandomColor = true;
    bool searchPVS = true;

    for (const auto &move : LegalMoves)
    {
        doMove(side, move);
        if (searchPVS)
        {
            score = -Search(depth + 1, -side, -beta, -alpha, flag);
        }
        else
        {
            score = -Search(depth + 1, -side, -alpha - 1, -alpha, flag);
            if (score > alpha)
                score = -Search(depth + 1, -side, -beta, -alpha, flag);
        }
        undoMove(side, move);

        if (score >= beta)
            return beta;

        if (score > alpha)
        {
            alpha = score;

            PVTable[depth][depth] = move;
            for (int ply = depth + 1; ply < PVLength[depth + 1]; ply++)
                PVTable[depth][ply] = PVTable[depth + 1][ply];
            PVLength[depth] = PVLength[depth + 1];

            searchPVS = false;
        }
    }
    return alpha;
}
