/* Bot for Entropy (Codecup 2023)
 *
 * Author: D. Ngoc Anh Trung and N. Dang Dung
 *
 * November 2022
 *
*/

#include <bits/stdc++.h>
#define maxsize 7
int maxdepth = 2;
int iteratingdepth = 1;
int unitbonus = 2;

using namespace std;
const int Infinity = numeric_limits<int>::max();

vector<vector<int>> board(maxsize, vector<int>(maxsize, 0));
int ChaosInput;     // Input for chip colour
string InputStr;    // Start, End, Input for Chaos and Order

std::string BestMoveForChaos();
std::string BestMoveForOrder();
void showBoard(vector<vector<int>> board);
int evaluate(vector<vector<int>> board);
int minimaxAlg(vector<vector<int>> board, int depth, bool isOrder, int alpha, int beta);

//vector<vector<vector<int>>> bonusEvaluations(maxsize + 1, board);
//unordered_map<string, int> TranspositionalTable;

struct Piece{
    int x, y, color;
    Piece(int a, int b, int c): x(a), y(b), color(c) { }
};

vector<Piece> PlacedPieces;

/// Driver code

int main()
{
    ios_base::sync_with_stdio(false);
    PlacedPieces.clear();
    while(getline(cin, InputStr))
    {
        //cerr << InputStr << "\n";
        if (InputStr == "Start")
        {
            continue;
            // Play as Chaos
            // maxdepth = 3; <- also make sure other alg works fast before increasing depth
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
                // Input format: AbAc
                int foo = board[InputStr[0] - 'A'][InputStr[1] - 'a'];
                board[InputStr[0] - 'A'][InputStr[1] - 'a'] = 0;
                board[InputStr[2] - 'A'][InputStr[3] - 'a'] = foo;
            }else if (InputStr.length() == 3)
            {
                // Input format: 5Aa
                board[InputStr[1] - 'A'][InputStr[2] - 'a'] = InputStr[0] - '0';
                PlacedPieces.push_back(Piece(InputStr[1] - 'A', InputStr[2] - 'a', InputStr[0] - '0'));
                string move = BestMoveForOrder();
                cout << move << endl;
                cout.flush();
            }
        }
    }

    return 0;
}

int evaluate(vector<vector<int>> board){

    // Distribution of w on evaluation of each length of palindromes
    int eval[maxsize + 1] = {0};

    for (int len = 2; len <= maxsize; len++)
    {
        // Horizontal palindromes
        for (int i = 0; i < maxsize; i++)
        for (int j = 0; j + len - 1 < maxsize; j++)
        {
            bool flag = 1;
            for (int k = 0; k < len/2 + 1; k++)
            {
                if (board[i][j + k] != board[i][j + len - k - 1] || !board[i][j + k] || !board[i][j + len - k - 1])
                {
                    flag = 0;
                    break;
                }
            }
            if (flag) eval[len]+= len;
        }

        // Vertical palindromes
        for (int j = 0; j < maxsize; j++)
        for (int i = 0; i + len - 1 < maxsize; i++)
        {
            bool flag = 1;
            for (int k = 0; k < len/2 + 1; k++)
            {
                if (board[i + k][j] != board[i + len - k - 1][j] || !board[i + k][j] || !board[i + len - k - 1][j])
                {
                    flag = 0;
                    break;
                }
            }
            if (flag) eval[len]+= len;
        }
    }
    return eval[2] + eval[3]*2 + eval[4]*2 + eval[5]*3 + eval[6]*3 + eval[7]*4;
}

/// CHAOS' MOVE
string BestMoveForChaos()
{
    int bestScore = Infinity;
    int bestX = -1, bestY = -1;
    for (iteratingdepth = 1; iteratingdepth <= maxdepth; iteratingdepth++)
    {
        for (int i = 0; i < maxsize; i++)
        for (int j = 0; j < maxsize; j++)
        {
            if (board[i][j]) continue;
            if (bestX < 0 && bestY < 0)
            {
                bestX = i;
                bestY = j;
            }

            board[i][j] = ChaosInput;
            PlacedPieces.push_back(Piece(i, j, ChaosInput));

            int score = minimaxAlg(board, 1, true, -Infinity, Infinity);

            PlacedPieces.pop_back();
            board[i][j] = 0;
            if (score < bestScore)
            {
                bestScore = score;
                bestX = i;
                bestY = j;
            }
        }
    }

    board[bestX][bestY] = ChaosInput;
    PlacedPieces.push_back(Piece(bestX, bestY, ChaosInput));

    string res;
    res.push_back(bestX + 'A');
    res.push_back(bestY + 'a');
    return res;
}

/// ORDER'S MOVE
string BestMoveForOrder()
{
    int bestScore = -Infinity;
    int chosenK = 0;
    pair<int, int> bestMove;
    for (iteratingdepth = 1; iteratingdepth <= maxdepth; iteratingdepth++)
    {
        for (int k = 0; k < PlacedPieces.size(); k++)
        {
            if (PlacedPieces[k].color == 0) continue;
            int thisX = PlacedPieces[k].x;
            int thisY = PlacedPieces[k].y;
            int thisChip = PlacedPieces[k].color;
            for (int i = thisX; i < maxsize; i++)
            {
                if (board[i][thisY] && thisX != i) break;

                board[thisX][thisY] = 0;
                board[i][thisY] = thisChip;
                PlacedPieces[k].x = i;

                int score = minimaxAlg(board, 1, false, -Infinity, Infinity);
                if (bestScore <= score)
                {
                    bestScore = score;
                    chosenK = k;
                    bestMove = {i, thisY};
                }

                PlacedPieces[k].x = thisX;
                board[i][thisY] = 0;
                board[thisX][thisY] = thisChip;
            }
            for (int i = thisX - 1; i >= 0; i--)
            {
                if (board[i][thisY]) break;

                board[thisX][thisY] = 0;
                board[i][thisY] = thisChip;
                PlacedPieces[k].x = i;

                int score = minimaxAlg(board, 1, false, -Infinity, Infinity);
                if (bestScore <= score)
                {
                    bestScore = score;
                    chosenK = k;
                    bestMove = {i, thisY};
                }

                PlacedPieces[k].x = thisX;
                board[i][thisY] = 0;
                board[thisX][thisY] = thisChip;

            }
            for (int j = thisY + 1; j < maxsize; j++)
            {
                if (board[thisX][j]) break;

                board[thisX][thisY] = 0;
                board[thisX][j] = thisChip;
                PlacedPieces[k].y = j;

                int score = minimaxAlg(board, 1, false, -Infinity, Infinity);
                if (bestScore <= score)
                {
                    bestScore = score;
                    chosenK = k;
                    bestMove = {thisX, j};
                }

                PlacedPieces[k].y = thisY;
                board[thisX][j] = 0;
                board[thisX][thisY] = thisChip;
            }
            for (int j = thisY - 1; j >= 0; j--)
            {
                if (board[thisX][j]) break;

                board[thisX][thisY] = 0;
                board[thisX][j] = thisChip;
                PlacedPieces[k].y = j;

                int score = minimaxAlg(board, 1, false, -Infinity, Infinity);
                if (bestScore <= score)
                {
                    bestScore = score;
                    chosenK = k;
                    bestMove = {thisX, j};
                }

                PlacedPieces[k].y = thisY;
                board[thisX][j] = 0;
                board[thisX][thisY] = thisChip;
            }
        }
    }
    board[PlacedPieces[chosenK].x][PlacedPieces[chosenK].y] = 0;
    board[bestMove.first][bestMove.second] = PlacedPieces[chosenK].color;
    string res;
    res.push_back(PlacedPieces[chosenK].x + 'A');
    res.push_back(PlacedPieces[chosenK].y + 'a');
    res.push_back(bestMove.first + 'A');
    res.push_back(bestMove.second + 'a');
    PlacedPieces[chosenK].x = bestMove.first;
    PlacedPieces[chosenK].y = bestMove.second;
    return res;
}


/// MAIN SEARCH ALGORITHM
// implemented:
//      minimax
//      alpha - beta pruning


int minimaxAlg(vector<vector<int>> board, int depth, bool isOrder, int alpha, int beta)
{
    if (depth >= iteratingdepth || PlacedPieces.size() == 49){
        return evaluate(board);
    }

    if (isOrder)
    {
        int score;
        for (int k = 0; k < PlacedPieces.size(); k++)
        {
            if (PlacedPieces[k].color == 0) continue;
            int thisX = PlacedPieces[k].x;
            int thisY = PlacedPieces[k].y;
            int thisChip = PlacedPieces[k].color;
            for (int i = thisX; i < maxsize; i++)
            {
                if (board[i][thisY] && thisX != i) break;

                board[thisX][thisY] = 0;
                board[i][thisY] = thisChip;
                PlacedPieces[k].x = i;

                score = minimaxAlg(board, depth + 1, false, alpha, beta);

                PlacedPieces[k].x = thisX;
                board[i][thisY] = 0;
                board[thisX][thisY] = thisChip;

                // fail hard beta cutoff
                if (score >= beta) return beta;
                alpha = max(alpha, score);
            }
            for (int i = thisX - 1; i >= 0; i--)
            {
                if (board[i][thisY] && thisX != i) break;

                board[thisX][thisY] = 0;
                board[i][thisY] = thisChip;
                PlacedPieces[k].x = i;

                score = minimaxAlg(board, depth + 1, false, alpha, beta);

                PlacedPieces[k].x = thisX;
                board[i][thisY] = 0;
                board[thisX][thisY] = thisChip;

                // fail hard beta cutoff
                if (score >= beta) return beta;
                alpha = max(alpha, score);
            }
            for (int j = thisY + 1; j < maxsize; j++)
            {
                if (board[thisX][j]) break;

                board[thisX][thisY] = 0;
                board[thisX][j] = thisChip;
                PlacedPieces[k].y = j;

                score = minimaxAlg(board, depth + 1, false, alpha, beta);

                PlacedPieces[k].y = thisY;
                board[thisX][j] = 0;
                board[thisX][thisY] = thisChip;

                // fail hard beta cutoff
                if (score >= beta) return beta;
                alpha = max(alpha, score);
            }
            for (int j = thisY - 1; j >= 0; j--)
            {
                if (board[thisX][j]) break;

                board[thisX][thisY] = 0;
                board[thisX][j] = thisChip;
                PlacedPieces[k].y = j;

                score = minimaxAlg(board, depth + 1, false, alpha, beta);

                PlacedPieces[k].y = thisY;
                board[thisX][j] = 0;
                board[thisX][thisY] = thisChip;

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

            score = minimaxAlg(board, depth + 1, !isOrder, alpha, beta);

            PlacedPieces.pop_back();
            board[i][j] = 0;

            // fail hard alpha cutoff
            if (score <= alpha) return alpha;
            beta = min(beta, score);
        }
        return beta;
    }
}
