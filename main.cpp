/* Bot for Entropy (Codecup 2023)
 *
 * Author: D. Ngoc Anh Trung and N. Dang Dung
 *
 * November 2022
 *
*/

#include <bits/stdc++.h>
#define maxsize 7
int maxdepth = 3;
int iteratingdepth = 1;
int unitbonus = 2;

using namespace std;
const int Infinity = numeric_limits<int>::max();

vector<vector<int>> board(maxsize, vector<int>(maxsize, 0));
int ChaosInput;     // Input for chip colour
string InputStr;    // Start, End, Input for Chaos and Order

std::string BestMoveForChaos();
std::string BestMoveForOrder();
void showBoard();
int evaluate();
int minimaxAlg(int depth, bool isOrder, int alpha, int beta);

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
                // showBoard();
                cout << move << endl;
                cout.flush();
            }
            else if (InputStr.length() == 4)
            {
                // Input format: AbAc
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
                // Input format: 5Aa
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

void showBoard(){
    cout << "BOARD\n\n";
    for (int i = 0; i < maxsize; i++)
    {
        for (int j = 0; j < maxsize; j++)
        {
            cout << board[i][j] << " ";
        }
        cout << endl;
    }
    return;
}

int evaluate(){

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

/// SLIDE AND PLACE COMMANDS

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
            if (score <= bestScore)
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
    char orientation = 'x';
    int toMove = 0;
    int newPosition = 0;
    for (iteratingdepth = 1; iteratingdepth <= maxdepth; iteratingdepth++)
    {
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
    }
    string res;
    res.push_back(PlacedPieces[toMove].x + 'A');
    res.push_back(PlacedPieces[toMove].y + 'a');
    if (orientation == 'x')
    {
        res.push_back(newPosition + 'A');
        res.push_back(PlacedPieces[toMove].y + 'a');
    }else{
        res.push_back(PlacedPieces[toMove].x + 'A');
        res.push_back(newPosition + 'a');
    }
    Slide(orientation, toMove, newPosition);
    return res;
}


/// MAIN SEARCH ALGORITHM
// implemented:
//      minimax
//      alpha - beta pruning


int minimaxAlg(int depth, bool isOrder, int alpha, int beta)
{
    if (depth >= iteratingdepth || PlacedPieces.size() == 49){
        return evaluate();
    }

    if (isOrder)
    {
        int score;
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
