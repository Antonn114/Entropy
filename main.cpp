#include <bits/stdc++.h>
#define size 7
int maxdepth = 2;
using namespace std;
const int Infinity = numeric_limits<int>::max();

/*   COL - a b c d e f g
 * ROW
 *  A      x x x x x x x
 *  B      x x x x x x x
 *  C      x x x x x x x
 *  D      x x x x x x x
 *  E      x x x x x x x
 *  F      x x x x x x x
 *  G      x x x x x x x
*/

/*
vector<vector<int>> evalBoard{
    {2, 2, 2, 2, 2, 2, 2},
    {2, 1, 1, 2, 1, 1, 2},
    {2, 1, 2, 3, 2, 1, 2},
    {2, 2, 3, 3, 3, 2, 2},
    {2, 1, 2, 3, 2, 1, 2},
    {2, 1, 1, 2, 1, 1, 2},
    {2, 2, 2, 2, 2, 2, 2}
};
*/

vector<vector<int>> board(size, vector<int>(size, 0));

int ChaosInput; // chip
string InputStr; // Start, End, Order move

std::string BestMoveForChaos();
std::string BestMoveForOrder();

void showBoard(vector<vector<int>> board)
{
    for(int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
            cout << board[i][j] << " ";
        cout << endl;
    }
}

int evaluate(vector<vector<int>> board){
    int eval[size + 1] = {0};
    for (int len = 2; len <= size; len++)
    {
        // Horizontal palindromes
        for (int i = 0; i < size; i++)
        for (int j = 0; j + len - 1 < size; j++)
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
            if (flag)
                eval[len]+= len;
        }

        // Vertical palindromes
        for (int j = 0; j < size; j++)
        for (int i = 0; i + len - 1 < size; i++)
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
            if (flag)
                eval[len]+= len;
        }
    }
    return eval[2] + eval[3]*2 + eval[4]*2 + eval[5]*3 + eval[6]*3 + eval[7]*4;
}
int minimaxAlg(vector<vector<int>> board, int depth, bool isOrder, int alpha, int beta);

signed main()
{
    ios_base::sync_with_stdio(NULL);
    cin.tie(0);
    cout.tie(0);

    while(cin >> InputStr)
    {
        if (InputStr[0] == 'S'){
            //maxdepth = 3;
            for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                board[i][j] = 0;
            cin >> ChaosInput;
            cout << BestMoveForChaos() << endl;
        }else if (InputStr[0] == 'Q')
        {
            exit(0);
        }else if (InputStr.length() == 4){
            int foo = board[InputStr[0] - 'A'][InputStr[1] - 'a'];
            board[InputStr[0] - 'A'][InputStr[1] - 'a'] = 0;
            board[InputStr[2] - 'A'][InputStr[3] - 'a'] = foo;
            cin >> ChaosInput;
            cout << BestMoveForChaos() << endl;
        }else if (InputStr.length() == 3)
        {
            board[InputStr[1] - 'A'][InputStr[2] - 'a'] = InputStr[0] - '0';
            cout << BestMoveForOrder() << endl;
        }
    }

    return 0;
}

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
    return string{bestX + 'A', bestY + 'a'};
}

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
    return string{srcMove.first + 'A', srcMove.second + 'a', bestMove.first + 'A', bestMove.second + 'a'};
}

int minimaxAlg(vector<vector<int>> board, int depth, bool isOrder, int alpha, int beta)
{
    //cout << "BOARD || DEPTH = " << depth << " IS " << (isOrder ? "ORDER's" : "CHAOS'") << " TURN\n";
    //showBoard(board);
    if (depth >= maxdepth){
        return evaluate(board);
    }

    if (isOrder)
    {
        int bestScore = -Infinity;
        bool pruned = false;
        for (int thisX = 0; thisX < size && !pruned; thisX++)
        for (int thisY = 0; thisY < size && !pruned; thisY++)
        {
            if (!board[thisX][thisY]) continue;
            int thisChip = board[thisX][thisY];
            for (int i = thisX; i < size && !pruned; i++)
            {
                if (board[i][thisY] && thisX != i) break;
                board[thisX][thisY] = 0;
                board[i][thisY] = thisChip;
                int score = minimaxAlg(board, depth + 1, !isOrder, alpha, beta);
                bestScore = max(score, bestScore);
                board[i][thisY] = 0;
                board[thisX][thisY] = thisChip;

                alpha = max(alpha, score);
                if (beta <= alpha)
                {
                    pruned = true;
                    break;
                }

            }
            for (int i = thisX - 1; i >= 0 && !pruned; i--)
            {
                if (board[i][thisY]) break;
                board[thisX][thisY] = 0;
                board[i][thisY] = thisChip;
                int score = minimaxAlg(board, depth + 1, !isOrder, alpha, beta);
                bestScore = max(score, bestScore);
                board[i][thisY] = 0;
                board[thisX][thisY] = thisChip;

                alpha = max(alpha, score);
                if (beta <= alpha)
                {
                    pruned = true;
                    break;
                }
            }
            for (int j = thisY + 1; j < size && !pruned; j++)
            {
                if (board[thisX][j]) break;
                if (j == thisY) continue;
                board[thisX][thisY] = 0;
                board[thisX][j] = thisChip;
                int score = minimaxAlg(board, depth + 1, !isOrder, alpha, beta);
                bestScore = max(score, bestScore);
                board[thisX][j] = 0;
                board[thisX][thisY] = thisChip;

                alpha = max(alpha, score);
                if (beta <= alpha)
                {
                    pruned = true;
                    break;
                }
            }
            for (int j = thisY - 1; j >= 0 && !pruned; j--)
            {
                if (board[thisX][j]) break;
                if (j == thisY) continue;
                board[thisX][thisY] = 0;
                board[thisX][j] = thisChip;
                int score = minimaxAlg(board, depth + 1, !isOrder, alpha, beta);
                bestScore = max(score, bestScore);
                board[thisX][j] = 0;
                board[thisX][thisY] = thisChip;

                alpha = max(alpha, score);
                if (beta <= alpha)
                {
                    pruned = true;
                    break;
                }
            }
        }
        return bestScore;
    }
    else
    {
        bool pruned = false;
        int bestScore = Infinity;
        for (int k = 1; k <= size && !pruned; k++)
        for (int i = 0; i < size && !pruned; i++)
        for (int j = 0; j < size && !pruned; j++)
        {
            if (board[i][j]) continue;
            board[i][j] = k;
            int score = minimaxAlg(board, depth + 1, !isOrder, alpha, beta);
            board[i][j] = 0;
            bestScore = min(score, bestScore);

            beta = min(beta, score);
            if (beta <= alpha)
            {
                pruned = true;
                break;
            }
        }
        return bestScore;
    }
}
