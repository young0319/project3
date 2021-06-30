#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <string>
#include <sstream>
#include <cassert>
using namespace std;
#define INF 100000000

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}

	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;

    //construct
    OthelloBoard(array<array<int, SIZE>, SIZE> b, int player)
    {
        disc_count[0] = disc_count[1] = disc_count[2] = 0;
        board = b;
        for(int i=0;i<8;i++)
        {
            for(int j=0;j<8;j++)
            {
                if(board[i][j] == 1) disc_count[1]++;
                else if(board[i][j] == 2) disc_count[2]++;
            }
        }

        disc_count[0] = 64 - disc_count[1] - disc_count[2];
        cur_player = player;
        done = false;
        winner = -1;
        next_valid_spots = get_valid_spots();
    }

    //copy construct
    OthelloBoard(const OthelloBoard& b)
    {
        board = b.board;
        next_valid_spots = b.next_valid_spots;
        disc_count = b.disc_count;
        cur_player = b.cur_player;
        done = b.done;
        winner = b.winner;

    }

private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    //回傳board[p.x][p.y]的值
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    //下棋
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard() {
        reset();
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }
    
    //output winner
    std::string encode_player(int state) {
        if (state == BLACK) return "O";
        if (state == WHITE) return "X";
        return "Draw";
    }
    //output 
    std::string encode_spot(int x, int y) {
        if (is_spot_valid(Point(x, y))) return ".";
        if (board[x][y] == BLACK) return "O";
        if (board[x][y] == WHITE) return "X";
        return " ";
    }
    //output
    std::string encode_output(bool fail=false) {
        int i, j;
        std::stringstream ss;
        ss << "Timestep #" << (8*8-4-disc_count[EMPTY]+1) << "\n";
        ss << "O: " << disc_count[BLACK] << "; X: " << disc_count[WHITE] << "\n";
        if (fail) {
            ss << "Winner is " << encode_player(winner) << " (Opponent performed invalid move)\n";
        } else if (next_valid_spots.size() > 0) {
            ss << encode_player(cur_player) << "'s turn\n";
        } else {
            ss << "Winner is " << encode_player(winner) << "\n";
        }
        ss << "+---------------+\n";
        for (i = 0; i < SIZE; i++) {
            ss << "|";
            for (j = 0; j < SIZE-1; j++) {
                ss << encode_spot(i, j) << " ";
            }
            ss << encode_spot(i, j) << "|\n";
        }
        ss << "+---------------+\n";
        ss << next_valid_spots.size() << " valid moves: {";
        if (next_valid_spots.size() > 0) {
            Point p = next_valid_spots[0];
            ss << "(" << p.x << "," << p.y << ")";
        }
        for (size_t i = 1; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << ", (" << p.x << "," << p.y << ")";
        }
        ss << "}\n";
        ss << "=================\n";
        return ss.str();
    }
    //output
    std::string encode_state() {
        int i, j;
        std::stringstream ss;
        ss << cur_player << "\n";
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE-1; j++) {
                ss << board[i][j] << " ";
            }
            ss << board[i][j] << "\n";
        }
        ss << next_valid_spots.size() << "\n";
        for (size_t i = 0; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << p.x << " " << p.y << "\n";
        }
        return ss.str();
    }
};

//------------------------------------------

//can be deleted
/*
struct Point_orignal {
    int x, y;
};
*/

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({(float)x, (float)y});
    }
}

//calculate heuristic value
int heuristic(OthelloBoard now)
{
    int board_value[8][8] = {
        100,-30,  6,  2,  2,  6,-30,100,
        -30,-50,  0,  0,  0,  0,-50,-30,
          6,  0, 10,  0,  0, 10,  0,  6,
          2,  0,  0,  3,  3,  0,  0,  2,
          2,  0,  0,  3,  3,  0,  0,  2,
          6,  0, 10,  0,  0, 10,  0,  6,
        -30,-50,  0,  0,  0,  0,-50,-30,
        100,-30,  6,  2,  2,  6,-30,100,
    };
    
    if(now.board[0][0] == player) board_value[0][1] = board_value[1][0] = board_value[1][1] = 50;
    else if(now.board[0][0] == 3-player) board_value[0][1] = board_value[1][0] = board_value[1][1] = -50;
    
    if(now.board[7][0] == player) board_value[7][1] = board_value[6][0] = board_value[6][1] = 50;
    else if(now.board[7][0] == 3-player) board_value[7][1] = board_value[6][0] = board_value[6][1] = -50;
    
    if(now.board[0][7] == player) board_value[0][6] = board_value[1][6] = board_value[1][7] = 50;
    else if(now.board[0][7] == 3-player) board_value[0][6] = board_value[1][6] = board_value[1][7] = -50;
    
    if(now.board[7][7] == player) board_value[6][6] = board_value[6][7] = board_value[7][6] = 50;
    else if(now.board[7][7] == 3-player) board_value[6][6] = board_value[6][7] = board_value[7][6] = -50;
    
    int heu = 0;
    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            //board_value
            if(now.board[i][j] == player) heu += (board_value[i][j]);
            else if(now.board[i][j] == 3-player) heu -= (board_value[i][j]);
            
            //中間
            if((now.board[i+1][j]!=0 && now.board[i-1][j]!=0) || (now.board[i+1][j+1]!=0 && now.board[i-1][j-1]!=0) || 
            (now.board[i][j-1]!=0 && now.board[i][j+1]!=0) || (now.board[i-1][j-1]!=0 && now.board[i-1][j-1]!=0)) 
                heu += 10;
            //
            
            
        }    
    }
    return heu;
}

//player - opponent
int find_value(OthelloBoard board)
{   
    
    if(((int)board.disc_count[1]+(int)board.disc_count[2])>50)
        return (((int)board.disc_count[player] - (int)board.disc_count[3-player])*5 + heuristic(board));
    else
        return heuristic(board);
}
//minmax
int minmax(OthelloBoard board_cur, int depth, int maxminizingPlayer, int A, int B)
{   
    /*
    if(depth == 1 && maxminizingPlayer == player)// (board_cur.next_valid_spots.empty() && maxminizingPlayer==player))
    {
        return find_value(board_cur);
    }*/
    if(depth == 0 || board_cur.done)// (board_cur.next_valid_spots.empty() && maxminizingPlayer==player))
    {
        return find_value(board_cur);
    }
    if(maxminizingPlayer == player)
    {
        int value = -1 * INF;
        for(int d=0;d<(int)board_cur.next_valid_spots.size();d++)
        {
            OthelloBoard board_next = board_cur;
            board_next.put_disc(board_cur.next_valid_spots[d]);

            value = max(value, minmax(board_next, depth-1, 3-maxminizingPlayer, A, B));
            A = max(A, value);
           if(A>=B) break;
       }
       return value;
    }
    else if(maxminizingPlayer == 3-player)
    {
       int value = INF;
       for(int d=0;d<(int)board_cur.next_valid_spots.size();d++)
        {
          OthelloBoard board_next = board_cur;
          board_next.put_disc(board_cur.next_valid_spots[d]);

           value = min(value, minmax(board_next, depth-1, 3-maxminizingPlayer, A, B));
            B = min(B, value);
            if(B<=A) break;
        }
        return value;
    }
    return 0;
}

void write_valid_spot(std::ofstream& fout) {
    int n_valid_spots = (int)next_valid_spots.size();
    srand(time(NULL));
    
    int index = -1, A = -1*INF, B = INF, value_cur;
    for(int d=0;d<n_valid_spots;d++)
    {
        OthelloBoard board_new(board, player);
        board_new.put_disc(next_valid_spots[d]);
        if(board_new.disc_count[0] <= 11)
            value_cur = minmax(board_new, 11, 3-player, A, B);  
        else
            value_cur = minmax(board_new, 5, 3-player, A, B);
        
        if(value_cur>A)
        {
            A = value_cur;
            index = d;
       }
    }
    
    // Choose random spot. (Not random uniform here)
    //int index = (rand() % n_valid_spots);
    Point p = next_valid_spots[index];
    
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}

/* 
    先 後
1   w  w
2   w  w
3   w  w
4   l  w
5   w  w
*/