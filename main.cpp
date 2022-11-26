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
const int unitbonus = 2;
const int Infinity = numeric_limits<int>::max();

struct Piece{
    int x, y, color;
    Piece(int a, int b, int c): x(a), y(b), color(c) { }
};

int iteratingdepth = 1;

vector<vector<int>> board(maxsize, vector<int>(maxsize, 0));
vector<Piece> PlacedPieces;
unordered_map<string, pair<int, int>> TranspositionTable;
vector<vector<vector<int>>> bonusEvaluations(maxsize + 1, board);

int ChaosInput;     // Input for chip colour
string InputStr;

std::string BestMoveForChaos();
std::string BestMoveForOrder();
int evaluate();
int minimaxAlg(int depth, bool isOrder, int alpha, int beta);

bool compPiece( Piece const& a,  Piece const& b)
{
    if (a.x != b.x) return a.x < b.x;
    else if (a.y != b.y) return a.y < b.y;
    else return a.color <= b.color;
}

string VPtoString(vector<Piece> VP)
{
    string s;
    for (int i = 0; i < (int)VP.size(); i++)
    {
        s += string{VP[i].color + '0', VP[i].x + '0', VP[i].y + '0'};
    }
    return s;
}

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
                for (int i = 0; i < PlacedPieces.size(); i++)
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
                PlacedPieces.push_back(Piece(InputStr[1] - 'A', InputStr[2] - 'a', InputStr[0] - '0'));
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
    return max(eval[2] + eval[3]*2 + eval[4]*3 + eval[5]*3 + eval[6]*4 + eval[7]*4, 1);
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
        PlacedPieces.push_back(Piece(i, j, ChaosInput));
        int score = minimaxAlg(1, true, -Infinity, Infinity);
        PlacedPieces.pop_back();
        board[i][j] = 0;
        if (score < bestScore)
        {
            bestScore = score;
            bestX = i;
            bestY = j;
        }
    }

    board[bestX][bestY] = ChaosInput;
    PlacedPieces.push_back(Piece(bestX, bestY, ChaosInput));

    // Generate Bonuses
    for (int i = bestX%unitbonus; i < maxsize; i+=unitbonus)
    for (int j = bestY%unitbonus; j < maxsize; j+=unitbonus)
    {
        bonusEvaluations[ChaosInput][i][j] += (maxsize*2 - 1)/unitbonus - (abs(i - bestX) + abs(j - bestY))/unitbonus;
    }

    return string{bestX + 'A', bestY + 'a'};
}

// ORDER'S MOVE
string BestMoveForOrder()
{
    int bestScore = -Infinity;
    char orientation = 'x';
    int toMove = 0;
    int newPosition = 0;
    for (int k = 0; k < PlacedPieces.size(); k++)
    {
        if (!PlacedPieces[k].color) continue;
        int thisX = PlacedPieces[k].x;
        int thisY = PlacedPieces[k].y;
        int thisChip = PlacedPieces[k].color;
        for (int i = thisX; i < maxsize; i++)
        {
            if (board[i][thisY] && thisX != i) break;
            Slide('x', k, i);
            int score = minimaxAlg(1, false, -Infinity, Infinity);
            Slide('x', k, thisX);
            if (bestScore <= score)
            {
                bestScore = score;
                orientation = 'x';
                toMove = k;
                newPosition = i;
            }
        }
        for (int i = thisX - 1; i >= 0; i--)
        {
            if (board[i][thisY]) break;
            Slide('x', k, i);
            int score = minimaxAlg(1, false, -Infinity, Infinity);
            Slide('x', k, thisX);
            if (bestScore <= score)
            {
                bestScore = score;
                orientation = 'x';
                toMove = k;
                newPosition = i;
            }
        }
        for (int j = thisY + 1; j < maxsize; j++)
        {
            if (board[thisX][j]) break;
            Slide('y', k, j);
            int score = minimaxAlg(1, false, -Infinity, Infinity);
            Slide('y', k, thisY);
            if (bestScore <= score)
            {
                bestScore = score;
                orientation = 'y';
                toMove = k;
                newPosition = j;
            }

        }
        for (int j = thisY - 1; j >= 0; j--)
        {
            if (board[thisX][j]) break;
            Slide('y', k, j);
            int score = minimaxAlg(1, false, -Infinity, Infinity);
            Slide('y', k, thisY);
            if (bestScore <= score)
            {
                orientation = 'y';
                toMove = k;
                newPosition = j;
            }
        }
    }
    // Deleting past bonuses
    for (int i = PlacedPieces[toMove].x%unitbonus; i < maxsize; i+=unitbonus)
    for (int j = PlacedPieces[toMove].y%unitbonus; j < maxsize; j+=unitbonus)
    {
        bonusEvaluations[PlacedPieces[toMove].color][i][j] -= (maxsize*2 - 1)/unitbonus - (abs(i - PlacedPieces[toMove].x) + abs(j - PlacedPieces[toMove].y))/unitbonus;
    }

    string res{PlacedPieces[toMove].x + 'A', PlacedPieces[toMove].y + 'a'};
    if (orientation == 'x')
        res += string{newPosition + 'A', PlacedPieces[toMove].y + 'a'};
    else{
        res += string{PlacedPieces[toMove].x + 'A', newPosition + 'a'};
    }
    Slide(orientation, toMove, newPosition);

    // Generating new bonuses
    for (int i = PlacedPieces[toMove].x%unitbonus; i < maxsize; i+=unitbonus)
    for (int j = PlacedPieces[toMove].y%unitbonus; j < maxsize; j+=unitbonus)
    {
        bonusEvaluations[PlacedPieces[toMove].color][i][j] += (maxsize*2 - 1)/unitbonus - (abs(i - PlacedPieces[toMove].x) + abs(j - PlacedPieces[toMove].y))/unitbonus;
    }
    return res;
}


/// MAIN SEARCH ALGORITHM
// implemented:
//      minimax
//      alpha - beta pruning


int minimaxAlg(int depth, bool isOrder, int alpha, int beta)
{
    vector<Piece> PiecesOnBoard = PlacedPieces;
    sort(PiecesOnBoard.begin(), PiecesOnBoard.end(), compPiece);
    string s = VPtoString(PiecesOnBoard);
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
            PlacedPieces.push_back(Piece(i, j, k));
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
