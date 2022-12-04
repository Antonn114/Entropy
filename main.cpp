/*
 * Bot for Entropy (Codecup 2023)
 *
 * Author: D. Ngoc Anh Trung and N. Dang Dung
 *
 * Last updated: 02/12/2022
 */

#include <bits/stdc++.h>

#pragma GCC target("avx,avx2,fma")
#pragma GCC optimization("Ofast")
#pragma GCC optimization("unroll-loops")

using namespace std;

///             === VARIABLES ===

const int maxsize = 7;
int maxdepth = 3;
int unitbonus = 2;
const double Infinity = 1e9;

struct Piece
{
    int x, y, color, k;
    Piece(int a, int b, int c, int _k) : x(a), y(b), color(c), k(_k) {}
};

vector<vector<int>> board;
vector<Piece> PlacedPieces;

unordered_map<string, pair<double, int>> TranspositionTable;
unordered_map<string, double> TTMonteCarlo;
vector<vector<vector<int>>> bonusEvaluations;

int colourInput;
string InputStr;

string BestMoveForChaos();
string BestMoveForOrder();
double Search(int depth, int color, double alpha, double beta);
double evaluate();

void doMove(int color, string move);
void undoMove(int color, string move);
string hashBoard();

void showBoard();

pair<string, int> returnMove;

///             === DRIVER CODE ===

int main()
{
    ios_base::sync_with_stdio(false);
    srand(time(NULL));

    board.assign(maxsize, vector<int>(maxsize, 0));
    bonusEvaluations.assign(maxsize + 1, board);

    while (getline(cin, InputStr))
    {
        if (InputStr == "Start")
        {
            // chaos enhancements
            continue;
        }
        else if (InputStr == "Quit")
        {
            break;
        }
        else
        {
            if (InputStr.length() == 1)
            {
                colourInput = InputStr[0] - '0';
                string move = BestMoveForChaos();
                cout << move << endl;
            }
            else if (InputStr.length() == 4)
            {
                int foo = board[InputStr[0] - 'A'][InputStr[1] - 'a'];
                for (int i = 0; i < (int)PlacedPieces.size(); i++)
                {
                    if (PlacedPieces[i].x == InputStr[0] - 'A' && PlacedPieces[i].y == InputStr[1] - 'a')
                    {
                        PlacedPieces[i].x = InputStr[2] - 'A';
                        PlacedPieces[i].y = InputStr[3] - 'a';
                        break;
                    }
                }
                board[InputStr[0] - 'A'][InputStr[1] - 'a'] = 0;
                board[InputStr[2] - 'A'][InputStr[3] - 'a'] = foo;
            }
            else if (InputStr.length() == 3)
            {
                board[InputStr[1] - 'A'][InputStr[2] - 'a'] = InputStr[0] - '0';
                PlacedPieces.push_back(Piece(InputStr[1] - 'A', InputStr[2] - 'a', InputStr[0] - '0', PlacedPieces.size()));
                string move = BestMoveForOrder();
                cout << move << endl;
            }
        }
    }

    return 0;
}

double checkWinner(int eval)
{
    if (120 + eval > 280 - eval)
        return 1.0;
    if (120 + eval < 280 - eval)
        return -1.0;
    return 0.5;
}

double evaluate()
{
    // Distribution of coefficients on the values of each length of palindromes

    array<double, maxsize + 1> eval = {0, 0, 0, 0, 0, 0, 0, 0};
    array<double, maxsize + 1> coeff = {0, 0, 1, 2, 3, 3, 4, 4};

    for (int len = 2; len <= maxsize; len++)
    {
        for (int i = 0; i < maxsize; i++)
        {
            for (int j = 0; j + len - 1 < maxsize; j++)
            {
                bool flag = 1;
                int bonusScore = 0;
                int l = j;
                int u = j + len - 1;
                while (l <= u)
                {
                    if (!board[i][l] || !board[i][u] || board[i][l] != board[i][u])
                    {
                        flag = 0;
                        break;
                    }
                    bonusScore += (1 - (len & 1)) * bonusEvaluations[board[i][l]][i][l];
                    l++;
                    u--;
                }
                if (flag)
                    eval[len] += len + bonusScore;
            }
        }

        for (int j = 0; j < maxsize; j++)
        {
            for (int i = 0; i + len - 1 < maxsize; i++)
            {
                bool flag = 1;
                int bonusScore = 0;
                int l = i;
                int u = i + len - 1;
                while (l <= u)
                {
                    if (!board[l][j] || !board[u][j] || board[l][j] != board[u][j])
                    {
                        flag = 0;
                        break;
                    }
                    bonusScore += (1 - (len & 1)) * bonusEvaluations[board[l][j]][l][j];
                    l++;
                    u--;
                }
                if (flag)
                    eval[len] += len + bonusScore;
            }
        }
    }
    double value = 0;
    for (int i = 2; i < eval.size(); i++)
    {
        value += eval[i] * coeff[i];
    }

    return value;
}

void doMove(int side, pair<string, int> move)
{
    if (side > 0)
    {
        pair<int, int> oldPosition = {move.first[0] - 'A', move.first[1] - 'a'};
        pair<int, int> newPosition = {move.first[2] - 'A', move.first[3] - 'a'};
        board[oldPosition.first][oldPosition.second] = 0;
        board[newPosition.first][newPosition.second] = PlacedPieces[move.second].color;
        PlacedPieces[move.second].x = newPosition.first;
        PlacedPieces[move.second].y = newPosition.second;
    }
    else
    {
        board[move.first[0] - 'A'][move.first[1] - 'a'] = move.second;
        PlacedPieces.push_back(Piece(move.first[0] - 'A', move.first[1] - 'a', move.second, PlacedPieces.size()));
    }
    return;
}

void undoMove(int side, pair<string, int> move)
{
    if (side > 0)
    {
        pair<string, int> newMove = make_pair(move.first.substr(2, 2) + move.first.substr(0, 2), move.second);
        doMove(side, newMove);
    }
    else
    {
        board[move.first[0] - 'A'][move.first[1] - 'a'] = 0;
        PlacedPieces.pop_back();
    }
}

void showBoard()
{
    for (int i = 0; i < maxsize; i++)
    {
        for (int j = 0; j < maxsize; j++)
        {
            cout << board[i][j];
        }
        cout << endl;
    }
    cout << endl;
    return;
}

string hashBoard()
{
    string s;
    for (int i = 0; i < maxsize; i++)
    {
        for (int j = 0; j < maxsize; j++)
        {
            s += board[i][j] + '0';
        }
    }

    return s;
}

string BestMoveForChaos()
{
    Search(0, -1, -Infinity, Infinity);
    doMove(-1, returnMove);

    int bestX = returnMove.first[0] - 'A';
    int bestY = returnMove.first[1] - 'a';

    for (int i = bestX % unitbonus; i < maxsize; i += unitbonus)
    {
        for (int j = bestY % unitbonus; j < maxsize; j += unitbonus)
        {
            bonusEvaluations[colourInput][i][j]++;
        }
    }

    return string{char(bestX + 'A'), char(bestY + 'a')};
}

string BestMoveForOrder()
{
    Search(0, 1, -Infinity, Infinity);
    doMove(1, returnMove);
    int toMove = returnMove.second;
    for (int i = PlacedPieces[toMove].x % unitbonus; i < maxsize; i += unitbonus)
    {
        for (int j = PlacedPieces[toMove].y % unitbonus; j < maxsize; j += unitbonus)
        {
            bonusEvaluations[PlacedPieces[toMove].color][i][j]--;
        }
    }
    for (int i = PlacedPieces[toMove].x % unitbonus; i < maxsize; i += unitbonus)
    {
        for (int j = PlacedPieces[toMove].y % unitbonus; j < maxsize; j += unitbonus)
        {
            bonusEvaluations[PlacedPieces[toMove].color][i][j]++;
        }
    }

    return returnMove.first;
}

/// MAIN SEARCH ALGORITHM

/*
    implemented:
    Minimax -> Negamax
    alpha - beta pruning
    transposition table
    principal variation search

    todo: iterative deepening and expected value algorithms
*/

void registerLegalMoves(vector<pair<string, int>> &LegalMoves, int color, int depth)
{
    //cerr << "DEPTH = " << depth << " GENERATING MOVES... ";
    LegalMoves.clear();
    if (color > 0)
    {
        for (int k = 0; k < PlacedPieces.size(); k++)
        {
            string oldPosition = string{char(PlacedPieces[k].x + 'A'), char(PlacedPieces[k].y + 'a')};
            for (int i = PlacedPieces[k].x; i < maxsize && (!board[i][PlacedPieces[k].y] || PlacedPieces[k].x == i); ++i)
                LegalMoves.push_back(make_pair(oldPosition + string{char(i + 'A'), char(PlacedPieces[k].y + 'a')}, k));
            for (int i = (PlacedPieces[k].x - 1); i >= 0 && !board[i][PlacedPieces[k].y]; --i)
                LegalMoves.push_back(make_pair(oldPosition + string{char(i + 'A'), char(PlacedPieces[k].y + 'a')}, k));
            for (int j = (PlacedPieces[k].y + 1); j < maxsize && !board[PlacedPieces[k].x][j]; ++j)
                LegalMoves.push_back(make_pair(oldPosition + string{char(PlacedPieces[k].x + 'A'), char(j + 'a')}, k));
            for (int j = (PlacedPieces[k].y - 1); j >= 0 && !board[PlacedPieces[k].x][j]; --j)
                LegalMoves.push_back(make_pair(oldPosition + string{char(PlacedPieces[k].x + 'A'), char(j + 'a')}, k));
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
                {
                    LegalMoves.push_back(make_pair(string{char(i + 'A'), char(j + 'a')}, colourInput));
                }
                else
                {
                    for (int k = 1; k <= maxsize; k++)
                    {
                        LegalMoves.push_back(make_pair(string{char(i + 'A'), char(j + 'a')}, k));
                    }
                }
            }
        }
    }
    //cerr << "GENERATED MOVES! (size = " << LegalMoves.size() << ")" << endl;
    return;
}
double MonteCarlo()
{
    vector<vector<int>> cachedBoard = board;
    for (int i = 0; i < maxsize; i++)
        for (int j = 0; j < maxsize; j++)
        {
            if (!board[i][j])
            {
                board[i][j] = rand() % 7 + 1;
            }
        }
    int score = evaluate();
    board = cachedBoard;
    return score;
}

double Search(int depth, int color, double alpha, double beta)
{
    const auto s = hashBoard();
    if (TranspositionTable[s].first && depth >= TranspositionTable[s].second)
    {
        return color * (TranspositionTable[s].first);
    }

    if (depth >= maxdepth || PlacedPieces.size() == 49)
    {
        TranspositionTable[s] = {evaluate(), depth};
        return color * (TranspositionTable[s].first);
    }

    bool isFirstNode = true;
    double score;
    vector<pair<string, int>> LegalMoves;
    registerLegalMoves(LegalMoves, color, depth);
    int bestVal = -Infinity;

    for (const auto &move : LegalMoves)
    {
        if (depth == 0)
        {
            doMove(color, move);
            score = -Search(depth + 1, -color, -beta, -alpha);
            undoMove(color, move);

            if (score >= alpha)
            {
                returnMove = move;
                alpha = score;
            }
        }
        else
        {
            doMove(color, move);
            if (isFirstNode)
            {
                score = -Search(depth + 1, -color, -beta, -alpha);
            }
            else
            {
                score = -Search(depth + 1, -color, -alpha - 1, -alpha);

                // fail soft and re-searching bestmove
                if (alpha < score && score < beta)
                {
                    score = -Search(depth + 1, -color, -beta, -alpha);
                }
            }
            undoMove(color, move);

            // fail hard beta cutoff
            if (score >= beta)
            {
                return beta;
            }

            if (score > alpha)
            {
                alpha = score;
                isFirstNode = false;
            }
        }
    }
    return alpha;
}
