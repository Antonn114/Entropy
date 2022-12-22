#pragma GCC optimize("Ofast,unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <map>
#include <string>
#include <array>
#include <queue>
#include <cmath>
#include <ctime>
#include <chrono>
#include <random>

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

#define boardPosition array<int, boardSize>

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

typedef struct State
{
    boardPosition board;
    vector<Piece> pieces;
    bool randomPlayout = false;
    State() : board(), pieces(), randomPlayout() {}
    State(boardPosition _b, vector<Piece> _p, bool _f) : board(_b), pieces(_p), randomPlayout(false) {}
} State;

State gameState(boardPosition{0}, vector<Piece>(), false);

int colourInput;
string RawInput;

mt19937 rng;

void doMove(State &state, Move move);
void undoMove(State &state, Move move);

void registerMovesForChaos(State state, vector<Move> &LegalMoves);
void registerMovesForOrder(State state, vector<Move> &LegalMoves);
string BestMoveForSide(int side);
double evaluate(State state, bool GetHeuristic = true);

void showBoard(State state);

int checkWinner(int eval)
{
    if (eval > 80)
        return ORDER;
    if (eval < 80)
        return CHAOS;
    return 0;
}

class Node
{
public:
    int toPlay;
    int playProbability;

    vector<Node *> *children;
    Move action;
    int visitCount;
    double valueSum;
    State state;

    Node(int _p, int _s)
    {
        playProbability = _p;
        toPlay = _s;

        children = new vector<Node *>();
        action = Move();
        visitCount = 0;
        valueSum = 0;
        state = State();
    }

    double value()
    {
        return valueSum / (double)visitCount;
    }

    void expand()
    {
        vector<Move> legalMoves;
        if (toPlay == ORDER)
            registerMovesForOrder(state, legalMoves);
        else
            registerMovesForChaos(state, legalMoves);

        for (const auto &move : legalMoves)
        {
            Node *child = new Node(1, toPlay);
            child->state = state;
            child->action = move;
            doMove(child->state, move);
            children->push_back(child);
        }
        return;
    }

    double UpperConfidenceBound(Node &parent, Node &child)
    {
        double probabilityScore = child.playProbability * sqrt(parent.visitCount) / (child.visitCount + 1);
        double valueScore = 0;
        if (child.visitCount > 0)
        {
            valueScore = -child.value();
        }
        return valueScore + probabilityScore;
    }

    Node *select()
    {
        Node *node = new Node(1, toPlay);
        double maxUCB;
        for (int i = 0; i < children->size(); i++)
        {
            double childUCBScore = UpperConfidenceBound(*this, *children->at(i));
            if (i == 0 || (i > 0 && maxUCB < childUCBScore))
            {
                maxUCB = childUCBScore;
                node = children->at(i);
            }
        }
        return node;
    }

    void playoutPolicy()
    {
        vector<Move> legalMoves;
        state.randomPlayout = true;
        if (toPlay == ORDER)
            registerMovesForOrder(state, legalMoves);
        else
            registerMovesForChaos(state, legalMoves);
        int it = int(rng()) % legalMoves.size();
        action = legalMoves[it];
        doMove(state, action);
        return;
    }

    double randomPlayout(Node node)
    {
        int score = 0;
        Node _node = node;
        int k = _node.state.pieces.size();
        int rollDepth = 4;
        // vector<Move> PVLINE[rollDepth];
        for (int i = 0; i < rollDepth; i++)
        {
            _node = node;
            // PVLINE[i].push_back(_node.action);
            while (_node.state.pieces.size() < min(k + rollDepth, 49))
            {
                _node.toPlay = -_node.toPlay;
                _node.playoutPolicy();
                // PVLINE[i].push_back(_node.action);
            }
            score += evaluate(_node.state, false);
        }
        // cout << "score: " << score / (rollDepth * 1.0) << "\n";
        // for (int i = 0; i < rollDepth; i++)
        // {
        //     for (const auto &move : PVLINE[i])
        //     {
        //         cout << move.content << " ";
        //     }
        //     cout << endl;
        // }

        return score / rollDepth;
    }

    Node *MonteCarlo(State _state, int simulationDepth)
    {
        Node *root = new Node(1, toPlay);
        root->state = _state;
        root->expand();
        for (int i = 0; i < simulationDepth; i++)
        {
            Node *node = root;
            vector<Node *> searchPath;
            searchPath.push_back(node);
            // select
            while (node->children->size() > 0)
            {
                node = node->select();
                searchPath.push_back(node);
            }
            double score = randomPlayout(*node);
            if (node->state.pieces.size() < 49)
            {
                node->expand();
            }
            for (int j = searchPath.size() - 1; j >= 0; j--)
            {
                searchPath[j]->visitCount++;
                searchPath[j]->valueSum += (searchPath[j]->toPlay) * score / 373.0;
            }
        }
        return root;
    }
};

int main()
{
    rng = mt19937(chrono::steady_clock::now().time_since_epoch().count());
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
            doMove(gameState, Move(RawInput.substr(1, 2), RawInput[0] - '0'));
            string move = BestMoveForSide(ORDER);
            cout << move << endl;
            continue;
        }
        if (RawInput.length() == 4)
        {
            auto isPiece = [](const Piece &p)
            { return p.pos == CodeToPosition(RawInput); };
            int PieceIndex = find_if(gameState.pieces.begin(), gameState.pieces.end(), isPiece) - gameState.pieces.begin();
            doMove(gameState, Move(RawInput, PieceIndex));
            continue;
        }
    }

    return 0;
}

string BestMoveForSide(int side)
{
    // e.g: score: -5 pv: 1. CcCb 2Aa 2. EcEb
    Node *game = new Node(1, side);
    game->state = gameState;
    game = game->MonteCarlo(gameState, 400);
    game = game->select();
    doMove(gameState, game->action);
    return game->action.content;
}

double evaluate(State state, bool GetHeuristic)
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
            if (col(i) + k <= n && state.board[i] && state.board[i + k - 1] && state.board[i] == state.board[i + k - 1])
                horizontal[i][k] = (k > 2 ? horizontal[i + 1][k - 2] : 1);
            else
                horizontal[i][k] = 0;

            if (row(i) + k <= n && state.board[i] && state.board[i + (k - 1) * n] && state.board[i] == state.board[i + (k - 1) * n])
                vertical[i][k] = (k > 2 ? vertical[i + n][k - 2] : 1);
            else
                vertical[i][k] = 0;

            if (horizontal[i][k])
                eval += k;
            if (vertical[i][k])
                eval += k;
        }
    }
    return eval;
}

void doMove(State &state, Move move)
{
    if (move.content.length() == 4)
    {
        int oldPos = CodeToPosition(move.content.substr(0, 2));
        int newPos = CodeToPosition(move.content.substr(2, 2));
        swap(state.board[oldPos], state.board[newPos]);
        state.pieces[move.flag].pos = newPos;
    }
    else
    {
        int movePos = CodeToPosition(move.content);
        state.board[movePos] = move.flag;
        state.pieces.push_back(Piece(movePos, move.flag, state.pieces.size()));
    }
    return;
}

void undoMove(State &state, Move move)
{
    if (move.content.length() == 4)
    {
        Move newMove = Move(move.content.substr(2, 2) + move.content.substr(0, 2), move.flag);
        doMove(state, newMove);
    }
    else
    {
        state.board[CodeToPosition(move.content)] = 0;
        state.pieces.pop_back();
    }
    return;
}

void showBoard(State state)
{
    for (int i = 1; i < boardSize; i++)
    {
        if (i % 7 == 1)
            cerr << "\n";
        cerr << state.board[i] << " ";
    }
    cerr << "\n";
    return;
}

void registerMovesForChaos(State state, vector<Move> &LegalMoves)
{
    for (int i = 1; i < boardSize; i++)
    {
        if (state.board[i])
            continue;
        if (!state.randomPlayout)
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

void registerMovesForOrder(State state, vector<Move> &LegalMoves)
{
    for (int k = 0; k < state.pieces.size(); k++)
    {
        int fromPosition = state.pieces[k].pos;
        LegalMoves.push_back(Move(PositionToCode[fromPosition] + PositionToCode[fromPosition], k));
        for (int i = 0; i < 4; i++)
        {
            int toPosition = fromPosition + dpos[i];
            while (!state.board[toPosition] && toPosition < boardSize && toPosition >= 1)
            {
                if (row(fromPosition) != row(toPosition))
                    break;
                LegalMoves.push_back(Move(PositionToCode[fromPosition] + PositionToCode[toPosition], k));
                toPosition += dpos[i];
            }
        }
    }
    return;
}
