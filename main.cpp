/* Bot for Entropy (Codecup 2023)
 *
 * Author: D. Ngoc Anh Trung and N. Dang Dung
 *
 * November 2022
 *
*/

#include <bits/stdc++.h>
#define size 7
int maxdepth = 2;
int unitbonus = 2;

using namespace std;
const int Infinity = numeric_limits<int>::max();

vector<vector<int>> board(size, vector<int>(size, 0));
int ChaosInput;     // Input for chip colour
string InputStr;    // Start, End, Input for Chaos and Order

std::string BestMoveForChaos();
std::string BestMoveForOrder();
void showBoard(vector<vector<int>> board);
int evaluate(vector<vector<int>> board);
int minimaxAlg(vector<vector<int>> board, int depth, bool isOrder, int alpha, int beta);

vector<vector<vector<int>>> bonusEvaluations(size + 1, board);
unordered_map<string, int> TranspositionalTable;

/// Driver code

int main()
{
    ios_base::sync_with_stdio(NULL); cin.tie(0); cout.tie(0);
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
    int eval[size + 1] = {0};

    for (int len = 2; len <= size; len++)
    {
        // Horizontal palindromes
        for (int i = 0; i < size; i++)
        for (int j = 0; j + len - 1 < size; j++)
        {
            bool flag = 1;
            int fooscore = 0;
            for (int k = 0; k < len/2 + 1; k++)
            {
                if (board[i][j + k] != board[i][j + len - k - 1] || !board[i][j + k] || !board[i][j + len - k - 1])
                {
                    flag = 0;
                    break;
                }
                fooscore += bonusEvaluations[board[i][j + k]][i][j + k];
                if (k != len - k - 1) fooscore += bonusEvaluations[board[i][j + k]][i][j + k];
            }
            if (flag)
            {
                eval[len]+= len + fooscore;

            }\
        }

        // Vertical palindromes
        for (int j = 0; j < size; j++)
        for (int i = 0; i + len - 1 < size; i++)
        {
            bool flag = 1;
            int fooscore = 0;
            for (int k = 0; k < len/2 + 1; k++)
            {
                if (board[i + k][j] != board[i + len - k - 1][j] || !board[i + k][j] || !board[i + len - k - 1][j])
                {
                    flag = 0;
                    break;
                }
                fooscore += bonusEvaluations[board[i + k][j]][i + 1][j];
                if (k != len - k - 1) fooscore += bonusEvaluations[board[i + k][j]][i + 1][j];
            }
            if (flag)
            {
                eval[len]+= len + fooscore;
            }
        }
    }
    return eval[2] + eval[3]*2 + eval[4]*3 + eval[5]*3 + eval[6]*4 + eval[7]*4;
}

/// CHAOS' MOVE
string BestMoveForChaos()
{
    int bestScore = Infinity;
    int bestX = -1, bestY = -1;
    for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
    {
        if (board[i][j]) continue;
        if (bestX < 0 && bestY < 0)
        {
            bestX = i;
            bestY = j;
        }

        board[i][j] = ChaosInput;
        int score = minimaxAlg(board, 1, true, -Infinity, Infinity);
        board[i][j] = 0;
        if (score < bestScore)
        {
            bestScore = score;
            bestX = i;
            bestY = j;
        }
    }
    board[bestX][bestY] = ChaosInput;
    // Generate Bonuses
    for (int i = bestX%unitbonus; i < size; i+=unitbonus)
    {
        for (int j = bestY%unitbonus; j < size; j+=unitbonus)
        {
            bonusEvaluations[ChaosInput][i][j]++;
        }
    }
    string res;
    res.push_back(bestX + 'A');
    res.push_back(bestY + 'a');
    return res;
}

/// ORDER'S MOVE
string BestMoveForOrder()
{
    int bestScore = -Infinity;
    pair<int, int> srcMove;
    pair<int, int> bestMove;
    for (int thisX = 0; thisX < size; thisX++)
    for (int thisY = 0; thisY < size; thisY++)
    {
        if (!board[thisX][thisY]) continue;
        int thisChip = board[thisX][thisY];
        for (int i = thisX; i < size; i++)
        {
            if (board[i][thisY] && thisX != i) break;
            board[thisX][thisY] = 0;
            board[i][thisY] = thisChip;
            int score = minimaxAlg(board, 1, false, -Infinity, Infinity);
            if (bestScore <= score)
            {
                bestScore = score;
                srcMove = {thisX, thisY};
                bestMove = {i, thisY};
            }
            board[i][thisY] = 0;
            board[thisX][thisY] = thisChip;
        }
        for (int i = thisX - 1; i >= 0; i--)
        {
            if (board[i][thisY]) break;
            board[thisX][thisY] = 0;
            board[i][thisY] = thisChip;
            int score = minimaxAlg(board, 1, false, -Infinity, Infinity);
            if (bestScore <= score)
            {
                bestScore = score;
                srcMove = {thisX, thisY};
                bestMove = {i, thisY};
            }
            board[i][thisY] = 0;
            board[thisX][thisY] = thisChip;

        }
        for (int j = thisY + 1; j < size; j++)
        {
            if (board[thisX][j]) break;
            if (j == thisY) continue;
            board[thisX][thisY] = 0;
            board[thisX][j] = thisChip;
            int score = minimaxAlg(board, 1, false, -Infinity, Infinity);
            if (bestScore <= score)
            {
                bestScore = score;
                srcMove = {thisX, thisY};
                bestMove = {thisX, j};
            }
            board[thisX][j] = 0;
            board[thisX][thisY] = thisChip;

        }
        for (int j = thisY - 1; j >= 0; j--)
        {
            if (board[thisX][j]) break;
            if (j == thisY) continue;
            board[thisX][thisY] = 0;
            board[thisX][j] = thisChip;
            int score = minimaxAlg(board, 1, false, -Infinity, Infinity);
            if (bestScore <= score)
            {
                bestScore = score;
                srcMove = {thisX, thisY};
                bestMove = {thisX, j};
            }
            board[thisX][j] = 0;
            board[thisX][thisY] = thisChip;

        }
    }
    int foo = board[srcMove.first][srcMove.second];
    board[srcMove.first][srcMove.second] = 0;
    board[bestMove.first][bestMove.second] = foo;
    // Generate Bonuses
    for (int i = srcMove.first%unitbonus; i < size; i+=unitbonus)
    {
        for (int j = srcMove.second%unitbonus; j < size; j+=unitbonus)
        {
            bonusEvaluations[foo][i][j] = max(bonusEvaluations[foo][i][j] - 1, 0);
        }
    }
    for (int i = bestMove.first%unitbonus; i < size; i+=unitbonus)
    {
        for (int j = bestMove.second%unitbonus; j < size; j+=unitbonus)
        {
            bonusEvaluations[foo][i][j]++;
        }
    }
    string res;
    res.push_back(srcMove.first + 'A');
    res.push_back(srcMove.second + 'a');
    res.push_back(bestMove.first + 'A');
    res.push_back(bestMove.second + 'a');
    return res;
}


/// MAIN SEARCH ALGORITHM
// implemented:
//      minimax
//      alpha - beta pruning


int minimaxAlg(vector<vector<int>> board, int depth, bool isOrder, int alpha, int beta)
{
    if (depth >= maxdepth){
        return evaluate(board);
    }

    if (isOrder)
    {
        int bestScore = -Infinity;
        for (int thisX = 0; thisX < size; thisX++)
        for (int thisY = 0; thisY < size; thisY++)
        {
            if (!board[thisX][thisY]) continue;
            int thisChip = board[thisX][thisY];
            for (int i = thisX; i < size; i++)
            {
                if (board[i][thisY] && thisX != i) break;
                board[thisX][thisY] = 0;
                board[i][thisY] = thisChip;
                bestScore = max(bestScore, minimaxAlg(board, depth + 1, false, alpha, beta));
                alpha = max(alpha, bestScore);
                board[i][thisY] = 0;
                board[thisX][thisY] = thisChip;
                if (beta <= alpha) return bestScore;
            }
            for (int i = thisX - 1; i >= 0; i--)
            {
                if (board[i][thisY]) break;
                board[thisX][thisY] = 0;
                board[i][thisY] = thisChip;
                bestScore = max(bestScore, minimaxAlg(board, depth + 1, false, alpha, beta));
                alpha = max(alpha, bestScore);
                board[i][thisY] = 0;
                board[thisX][thisY] = thisChip;
                if (beta <= alpha) return bestScore;
            }
            for (int j = thisY + 1; j < size; j++)
            {
                if (board[thisX][j]) break;
                if (j == thisY) continue;
                board[thisX][thisY] = 0;
                board[thisX][j] = thisChip;
                bestScore = max(bestScore, minimaxAlg(board, depth + 1, false, alpha, beta));
                alpha = max(alpha, bestScore);
                board[thisX][j] = 0;
                board[thisX][thisY] = thisChip;
                if (beta <= alpha) return bestScore;
            }
            for (int j = thisY - 1; j >= 0; j--)
            {
                if (board[thisX][j]) break;
                if (j == thisY) continue;
                board[thisX][thisY] = 0;
                board[thisX][j] = thisChip;
                bestScore = max(bestScore, minimaxAlg(board, depth + 1, false, alpha, beta));
                alpha = max(alpha, bestScore);
                board[thisX][j] = 0;
                board[thisX][thisY] = thisChip;
                if (beta <= alpha) return bestScore;
            }
        }
        return bestScore;
    }
    else
    {
        int bestScore = Infinity;
        for (int k = 1; k <= size; k++)
        for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
        {
            if (board[i][j]) continue;
            board[i][j] = k;
            int score = minimaxAlg(board, depth + 1, !isOrder, alpha, beta);
            board[i][j] = 0;
            bestScore = min(score, bestScore);
            beta = min(beta, score);
            if (beta <= alpha) return bestScore;
        }
        return bestScore;
    }
}
