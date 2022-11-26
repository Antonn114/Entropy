/* Bot for Entropy (Codecup 2023)
 *
 * Author: D. Ngoc Anh Trung and N. Dang Dung
 *
 * November 2022
 *
*/

#include <bits/stdc++.h>
#define maxsize 7

using namespace std;

int maxdepth = 2;
int unitbonus = 2;
const int Infinity = numeric_limits<int>::max();

struct Piece{
    int x, y, color, k;
    Piece(int a, int b, int c, int _k): x(a), y(b), color(c), k(_k) { }
};

vector<vector<int>> board(maxsize, vector<int>(maxsize, 0));
vector<Piece> PlacedPieces;
unordered_map<string, pair<int, int>> TranspositionTable;
vector<vector<vector<int>>> bonusEvaluations(maxsize + 1, board);

int ChaosInput;     // Input for chip colour
string InputStr;

std::string BestMoveForChaos();
std::string BestMoveForOrder();
int minimaxAlg(int depth, bool isOrder, int alpha, int beta);

int evaluate();
void Slide(char orientation, int PieceNumber, int newPosition);
bool compPiece(Piece const& a,  Piece const& b);
bool compBestMoves(pair<Piece, int> const& a,  pair<Piece, int> const& b);
string BoardtoString();


/// Driver code

int main()
{
    ios_base::sync_with_stdio(false);

    PlacedPieces.clear();
    while(getline(cin, InputStr))
    {
        //TranspositionTable.clear();
        if (InputStr == "Start")
        {
            maxdepth = 3;
            continue;
        }else if (InputStr == "Quit")
        {
            break;
        }else
        {
            if (InputStr.length() == 1)
            {
                ChaosInput = InputStr[0] - '0';
                string move = BestMoveForChaos();
                cout << move << endl;
                cout.flush();
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

            }else if (InputStr.length() == 3)
            {
                board[InputStr[1] - 'A'][InputStr[2] - 'a'] = InputStr[0] - '0';
                PlacedPieces.push_back(Piece(InputStr[1] - 'A', InputStr[2] - 'a', InputStr[0] - '0', PlacedPieces.size()));
                string move = BestMoveForOrder();
                // showBoard();
                cout << move << endl;
                cout.flush();
            }
        }
    }

    return 0;
}

/// UTILS

// Evaluation

int evaluate(){
    // Distribution of w on evaluation of each length of palindromes
    int eval[maxsize + 1] = {0};

    for (int len = 2; len <= maxsize; len++)
    {
        for (int i = 0; i < maxsize; i++)
        for (int j = 0; j + len - 1 < maxsize; j++)
        {
            bool flag = 1;
            int bonusScore = 0;
            for (int k = 0; k < len/2 + 1; k++)
            {
                if (board[i][j + k] != board[i][j + len - k - 1] || !board[i][j + k] || !board[i][j + len - k - 1])
                {
                    flag = 0;
                    break;
                }
                bonusScore += bonusEvaluations[board[i][j + k]][i][j + k];
                if (k != len - k - 1) bonusScore += bonusEvaluations[board[i][j + k]][i][j + k];
            }
            if (flag) eval[len]+= len + bonusScore;
        }

        for (int j = 0; j < maxsize; j++)
        for (int i = 0; i + len - 1 < maxsize; i++)
        {
            bool flag = 1;
            int bonusScore = 0;
            for (int k = 0; k < len/2 + 1; k++)
            {
                if (board[i + k][j] != board[i + len - k - 1][j] || !board[i + k][j] || !board[i + len - k - 1][j])
                {
                    flag = 0;
                    break;
                }
                bonusScore += bonusEvaluations[board[i + k][j]][i + k][j];
                if (k != len - k - 1) bonusScore += bonusEvaluations[board[i + k][j]][i + k][j];
            }
            if (flag) eval[len]+= len + bonusScore;
        }
    }
    int finalEval = eval[2] + eval[3]*2 + eval[4]*3 + eval[5]*3 + eval[6]*4 + eval[7]*4;
    return (finalEval == 0 ? 1 : finalEval);
}

// Slide and place commands

void Slide(char orientation, int PieceNumber, int newPosition){
    board[PlacedPieces[PieceNumber].x][PlacedPieces[PieceNumber].y] = 0;
    if (orientation == 'x')
    {
        board[newPosition][PlacedPieces[PieceNumber].y] = PlacedPieces[PieceNumber].color;
        PlacedPieces[PieceNumber].x = newPosition;
    }
    else
    {
        board[PlacedPieces[PieceNumber].x][newPosition] = PlacedPieces[PieceNumber].color;
        PlacedPieces[PieceNumber].y = newPosition;
    }
    return;
}

// Transposition Table

bool compPiece(Piece const& a,  Piece const& b)
{
    if (a.x != b.x) return a.x < b.x;
    else if (a.y != b.y) return a.y < b.y;
    else return a.color <= b.color;
}

string BoardtoString()
{
    string s;
    for (int i = 0; i < maxsize; i++)
    for (int j = 0; j < maxsize; j++)
    {
        s += board[i][j] + '0';
    }
    return s;
}

// Sorting best moves for a depth of 3 for Order

bool compBestMoves(pair<Piece, int> const& a,  pair<Piece, int> const& b)
{
    if (a.second != b.second) return a.second > b.second;
    else return !compPiece(a.first, b.first);
}


/// GAME FUNCTIONS

// CHAOS' MOVE
string BestMoveForChaos()
{
    int bestScore = Infinity;
    int bestX = -1, bestY = -1;
    for (int i = 0; i < maxsize; i++)
    for (int j = 0; j < maxsize; j++)
    {
        if (board[i][j]) continue;
        if (bestX < 0 || bestY < 0)
        {
            bestX = i;
            bestY = j;
        }

        board[i][j] = ChaosInput;
        PlacedPieces.push_back(Piece(i, j, ChaosInput, PlacedPieces.size()));
        int score = minimaxAlg(1, true, -Infinity, Infinity);
        PlacedPieces.pop_back();
        board[i][j] = 0;
        if (score <= bestScore)
        {
            bestScore = score;
            bestX = i;
            bestY = j;
        }
    }

    board[bestX][bestY] = ChaosInput;
    PlacedPieces.push_back(Piece(bestX, bestY, ChaosInput, PlacedPieces.size()));

    // Generate Bonuses
    for (int i = bestX%unitbonus; i < maxsize; i+=unitbonus)
    for (int j = bestY%unitbonus; j < maxsize; j+=unitbonus)
    {
        //bonusEvaluations[ChaosInput][i][j] += (maxsize*2 - 1)/unitbonus - (abs(i - bestX) + abs(j - bestY))/unitbonus;
        bonusEvaluations[ChaosInput][i][j] ++;
    }

    return {char(bestX + 'A'), char(bestY + 'a')};
}

// ORDER'S MOVE
string BestMoveForOrder()
{
    int bestScore = -Infinity;
    char orientation = 'x';
    int toMove = 0;
    int newPosition = 0;
    vector<Piece> PossibleBestMoves;

    for (int k = 0; k < (int)PlacedPieces.size(); k++)
    {
        if (!PlacedPieces[k].color) continue;
        int thisX = PlacedPieces[k].x;
        int thisY = PlacedPieces[k].y;
        for (int i = thisX; i < maxsize; i++)
        {
            if (board[i][thisY] && thisX != i) break;
            Slide('x', k, i);
            int score = minimaxAlg(1, false, -Infinity, Infinity);
            if (bestScore <= score)
            {
                if (bestScore < score)
                {
                    bestScore = score;
                    PossibleBestMoves.clear();
                }
                maxdepth = 3;
                PossibleBestMoves.push_back(PlacedPieces[k]);
                maxdepth = 2;
            }
            Slide('x', k, thisX);
        }
        for (int i = thisX - 1; i >= 0; i--)
        {
            if (board[i][thisY]) break;
            Slide('x', k, i);
            int score = minimaxAlg(1, false, -Infinity, Infinity);
            if (bestScore <= score)
            {
                if (bestScore < score)
                {
                    bestScore = score;
                    PossibleBestMoves.clear();
                }
                maxdepth = 3;
                PossibleBestMoves.push_back(PlacedPieces[k]);
                maxdepth = 2;
            }
            Slide('x', k, thisX);
        }
        for (int j = thisY + 1; j < maxsize; j++)
        {
            if (board[thisX][j]) break;
            Slide('y', k, j);
            int score = minimaxAlg(1, false, -Infinity, Infinity);
            if (bestScore <= score)
            {
                if (bestScore < score)
                {
                    bestScore = score;
                    PossibleBestMoves.clear();
                }
                maxdepth = 3;
                PossibleBestMoves.push_back(PlacedPieces[k]);
                maxdepth = 2;
            }
            Slide('y', k, thisY);
        }
        for (int j = thisY - 1; j >= 0; j--)
        {
            if (board[thisX][j]) break;
            Slide('y', k, j);
            int score = minimaxAlg(1, false, -Infinity, Infinity);
            if (bestScore <= score)
            {
                if (bestScore < score)
                {
                    bestScore = score;
                    PossibleBestMoves.clear();
                }
                PossibleBestMoves.push_back(PlacedPieces[k]);
            }
            Slide('y', k, thisY);
        }
    }
    bestScore = -Infinity;
    maxdepth = 4;
    for (int i = 0; i < PossibleBestMoves.size(); i++)
    {
        char thisOrientation;
        int oldPos;
        int thisPos;
        if (PossibleBestMoves[i].x != PlacedPieces[PossibleBestMoves[i].k].x){
            thisOrientation = 'x';
            oldPos = PlacedPieces[PossibleBestMoves[i].k].x;
            thisPos = PossibleBestMoves[i].x;
        }else{
            thisOrientation = 'y';
            oldPos = PlacedPieces[PossibleBestMoves[i].k].y;
            thisPos = PossibleBestMoves[i].y;
        }
        Slide(thisOrientation, PossibleBestMoves[i].k, thisPos);
        int score = minimaxAlg(1, false, -Infinity, Infinity);
        Slide(thisOrientation, PossibleBestMoves[i].k, oldPos);
        if (bestScore <= score){
            orientation = thisOrientation;
            toMove = PossibleBestMoves[i].k;
            newPosition = thisPos;
        }
    }
    maxdepth = 2;

    // Deleting past bonuses
    for (int i = PlacedPieces[toMove].x%unitbonus; i < maxsize; i+=unitbonus)
    for (int j = PlacedPieces[toMove].y%unitbonus; j < maxsize; j+=unitbonus)
    {
        //bonusEvaluations[PlacedPieces[toMove].color][i][j] -= (maxsize*2 - 1)/unitbonus - (abs(i - PlacedPieces[toMove].x) + abs(j - PlacedPieces[toMove].y))/unitbonus;
        bonusEvaluations[PlacedPieces[toMove].color][i][j]--;
    }
    string res{char(PlacedPieces[toMove].x + 'A'), char(PlacedPieces[toMove].y + 'a')};
    if (orientation == 'x')
        res += string{char(newPosition + 'A'), char(PlacedPieces[toMove].y + 'a')};
    else{
        res += string{char(PlacedPieces[toMove].x + 'A'), char(newPosition + 'a')};
    }
    Slide(orientation, toMove, newPosition);

    // Generating new bonuses
    for (int i = PlacedPieces[toMove].x%unitbonus; i < maxsize; i+=unitbonus)
    for (int j = PlacedPieces[toMove].y%unitbonus; j < maxsize; j+=unitbonus)
    {
       // bonusEvaluations[PlacedPieces[toMove].color][i][j] += (maxsize*2 - 1)/unitbonus - (abs(i - PlacedPieces[toMove].x) + abs(j - PlacedPieces[toMove].y))/unitbonus;
        bonusEvaluations[PlacedPieces[toMove].color][i][j]++;
    }
    return res;
}


/// MAIN SEARCH ALGORITHM
// implemented:
//      minimax
//      alpha - beta pruning
//      transposition table


int minimaxAlg(int depth, bool isOrder, int alpha, int beta)
{
    string s = BoardtoString();
    if (TranspositionTable[s].first && depth >= TranspositionTable[s].second) return TranspositionTable[s].first;

    if (depth >= maxdepth || PlacedPieces.size() == 49){
        TranspositionTable[s] = {evaluate(), depth};
        return TranspositionTable[s].first;
    }

    if (isOrder)
    {
        int score;
        for (int k = 0; k < (int)PlacedPieces.size(); k++)
        {
            if (!PlacedPieces[k].color) continue;
            int thisX = PlacedPieces[k].x;
            int thisY = PlacedPieces[k].y;
            for (int i = thisX; i < maxsize; i++)
            {
                if (board[i][thisY] && thisX != i) break;
                Slide('x', k, i);
                score = minimaxAlg(depth + 1, false, alpha, beta);
                Slide('x', k, thisX);

                // fail hard beta cutoff
                if (score >= beta) return beta;
                alpha = max(alpha, score);
            }
            for (int i = thisX - 1; i >= 0; i--)
            {
                if (board[i][thisY]) break;
                Slide('x', k, i);
                score = minimaxAlg(depth + 1, false, alpha, beta);
                Slide('x', k, thisX);

                // fail hard beta cutoff
                if (score >= beta) return beta;
                alpha = max(alpha, score);
            }
            for (int j = thisY + 1; j < maxsize; j++)
            {
                if (board[thisX][j]) break;
                Slide('y', k, j);
                score = minimaxAlg(depth + 1, false, alpha, beta);
                Slide('y', k, thisY);

                // fail hard beta cutoff
                if (score >= beta) return beta;
                alpha = max(alpha, score);
            }
            for (int j = thisY - 1; j >= 0; j--)
            {
                if (board[thisX][j]) break;
                Slide('y', k, j);
                score = minimaxAlg(depth + 1, false, alpha, beta);
                Slide('y', k, thisY);

                // fail hard beta cutoff
                if (score >= beta) return beta;
                alpha = max(alpha, score);
            }
        }
        return alpha;
    }
    else
    {
        int score;
        for (int k = 1; k <= maxsize; k++)
        for (int i = 0; i < maxsize; i++)
        for (int j = 0; j < maxsize; j++)
        {
            if (board[i][j]) continue;
            board[i][j] = k;
            PlacedPieces.push_back(Piece(i, j, k, PlacedPieces.size()));
            score = minimaxAlg(depth + 1, !isOrder, alpha, beta);
            PlacedPieces.pop_back();
            board[i][j] = 0;

            // fail hard alpha cutoff
            if (score <= alpha) return alpha;
            beta = min(beta, score);
        }
        return beta;
    }
}
