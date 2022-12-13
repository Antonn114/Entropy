#pragma GCC optimize("Ofast,unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <array>
#include <time.h>

using namespace std;

///             === VARIABLES ===


/*
? BOARD POSITION REF

	COL 1	2	3	4	5	6	7
ROW	
1	    1	2	3	4	5	6	7
2	    8	9	10	11	12	13	14
3	    15	16	17	18	19	20	21
4	    22	23	24	25	26	27	28
5	    29	30	31	32	33	34	35
6	    36	37	38	39	40	41	42
7	    43	44	45	46	47	48	49

? MOVING IN LOCAL POSITION
	-8	-7	-6
	-1	0	+1
	+6	+7	+8

*/

const int n = 7;
const int boardSize = n*n + 1;
int maxdepth = 3;
int unitbonus = 2;
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
double coeff[] = {0, 0, 1.21249, 1.8809, 2.45611, 2.64828, 3.47708, 3.77076};

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
        {
            continue;
        }
        else if (RawInput == "Quit")
            break;
        else
        {
            if (RawInput.length() == 1)
            {
                colourInput = RawInput[0] - '0';
                string move = BestMoveForSide(-1);
                cout << move << endl;
            }
            else if (RawInput.length() == 4)
            {
                for (int i = 0; i < (int)PlacedPieces.size(); i++)
                    if (PlacedPieces[i].pos == (RawInput[0] - 'A')*7 + RawInput[1] - 'a' + 1)
                    {
                        doMove(1, Move(RawInput, -1, i));
                        break;
                    }
            }
            else if (RawInput.length() == 3)
            {
                doMove(-1, Move(RawInput.substr(1, 2), RawInput[0] - '0', -1));
                string move = BestMoveForSide(1);
                cout << move << endl;
            }
        }
    }

    return 0;
}

double evaluate(bool GetHeuristic)
{
    double eval = 0;
	
	for (int len = 2; len <= n; len++)
	{
		for (int i = 1; i < boardSize; i++)
		{
			if (i + len >= n)
				continue;
			bool flag = 1;
            bool isfullcolor = 1;
			int l = i, u = i + len - 1;
			int inflatedAreas = 0;
			while(l < u)
			{
				if (!board[l] || !board[u] || board[l] != board[u])
				{
					flag = 0;
					break;
				}
                if (board[l] != board[i]) isfullcolor = 0;
                if (board[u] != board[i]) isfullcolor = 0;
				l++;
				u--;
			}
			if (flag)
				eval += len * (GetHeuristic ? coeff[len] : 1) + (GetHeuristic && isfullcolor ? len * 0.27 : 0);
		}
		for (int i = 1; i < boardSize - (len - 1)*n; i++)
		{
			bool flag = 1;
            bool isfullcolor = 1;
            int l = i, u = i + (len - 1)*n;
            while(l < u)
			{
				if (!board[l] || !board[u] || board[l] != board[u])
				{
					flag = 0;
					break;
				}
                if (board[l] != board[i]) isfullcolor = 0;
                if (board[u] != board[i]) isfullcolor = 0;

				l+=7;
				u-=7;
			}
			if (flag)
				eval += len * (GetHeuristic ? coeff[len] : 1) + (GetHeuristic && isfullcolor ? len  * 0.27 : 0);
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
				for (int j = 1; thisPos + dpos[i]*j < boardSize && thisPos + dpos[i]*j >= 1; j++)
				{
                    if (board[thisPos + dpos[i]*j] || (abs(dpos[i]) == 1 && row(thisPos + dpos[i]*j) != row(thisPos + dpos[i]*(j - 1)))) break;
					LegalMoves.push_back(Move(oldPos + string{char(row(thisPos + dpos[i]*j) + 'A'), char(col(thisPos + dpos[i]*j) + 'a')}, -1, k));
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
            if (flag.unknownColour)
            {
                for (int k = 1; k <= n; k++)
                    LegalMoves.push_back(Move(string{char(row(i) + 'A'), char(col(i) + 'a')}, k, -1));
            }else
			    LegalMoves.push_back(Move(string{char(row(i) + 'A'), char(col(i) + 'a')}, colourInput, -1));
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
        TranspositionTable[board] = TTEntry(evaluate(), depth);
        return color * TranspositionTable[board].value;
    }

    double score = 0;
    vector<Move> LegalMoves;
    registerLegalMoves(LegalMoves, color, flag);

    flag.unknownColour = true;

    for (const auto &move : LegalMoves)
    {
        doMove(color, move);
        score = -Search(depth + 1, -color, -beta, -alpha, flag);
        undoMove(color, move);

        // fail hard beta cutoff
        if (score >= beta)
            return beta;
        
        if (score == alpha && depth == 0)
            returnMoves.push_back(move);
        
        if (score > alpha)
        {
            if (depth == 0)
            {
                returnMoves = {move};
            }
            alpha = score;
        }
    }
    return alpha;
}
