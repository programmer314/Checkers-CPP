#include <iostream>
#include <sstream>
#include "PlayerAI.h"

#define O_PIECE 'O'
#define X_PIECE 'X'
#define O_KING '@'
#define X_KING '#'
#define SPACE '*'

using namespace std;

const uint32_t EVEN_MASK = 252645135;
const uint32_t ROW_END_MASK = 4026531855;
const uint32_t O_BACK_RANK = 15;
const uint32_t X_BACK_RANK = 4026531840;
const uint32_t L_COL_MASK = 16843009;
const uint32_t R_COL_MASK = 2155905152;

uint32_t x_board = 4293918720;
uint32_t o_board = 4095;
uint32_t kings = 0;

bool o_turn = true;
bool o_wins = false, x_wins = false;
uint32_t piece_pos;

PlayerAI playerAI(o_board, x_board, kings);

bool is_space(uint32_t pos)
{
    return pos & ~(o_board + x_board);
}

char getKingSide(char king)
{
    if (king == O_KING)
        return O_PIECE;
    else if (king == X_KING)
        return X_PIECE;
    else
        return king;

}

bool same_side(uint32_t pos, char piece)
{
    piece = getKingSide(piece);
    return
        piece == O_PIECE && (o_board & pos) ||
        piece == X_PIECE && (x_board & pos);
}

void add_piece(bool is_o, uint32_t pos, bool is_king)
{
    if (is_o)
        o_board |= pos;
    else
        x_board |= pos;

    if (is_king)
        kings |= pos;
}

void remove_piece(bool is_o, uint32_t pos)
{
    if (is_o)
        o_board &= ~pos;
    else
        x_board &= ~pos;

    kings &= ~pos;
}

// return 0 if move is out of bounds
uint32_t move(uint32_t pos, bool left, bool forward, bool o_turn)
{
    bool shift_left = o_turn == forward;
    int shift_value = 5;

    if (pos & EVEN_MASK)
        shift_value--;
    if (left)
        shift_value--;
    if (!shift_left)
        shift_value = 8 - shift_value;

    if ((left && (pos & L_COL_MASK)) || (!left && (pos & R_COL_MASK)))
        return 0;

    if (shift_left)
        pos <<= shift_value;
    else
        pos >>= shift_value;

    return pos;
}

uint32_t move(uint32_t pos, bool left, bool forward)
{
    return move(pos, left, forward, same_side(pos, O_PIECE));
}

char getPiece(uint32_t pos)
{
    char piece = (pos & o_board) ? O_KING : ((pos & x_board) ? X_KING : SPACE);
    return (pos & kings) ? piece : getKingSide(piece);
}

// left and forward always first [lf, rf, lb, rb]
void available_captures(bool* captures, uint32_t pos)
{
    bool is_o = same_side(pos, O_PIECE);
    uint32_t opponent = is_o ? x_board : o_board;

    captures[0] = false;
    const int direction_count = (pos & kings) ? 4 : 2;
    bool left = true, forward = true;
    uint32_t capture_pos, new_pos;
    for (int i = 0; i < direction_count; i++)
    {
        captures[i + 1] = false;
        capture_pos = move(pos, left, forward);
        if (capture_pos && (opponent & capture_pos))
        {
            new_pos = move(capture_pos, left, forward, is_o);
            if (new_pos && is_space(new_pos))
            {
                captures[i + 1] = true;
                captures[0] = true;
            }

        }

        left = !left;
        if (left)
            forward = !forward;
    }
}

bool has_move(uint32_t pos)
{
    bool o_turn = same_side(pos, O_PIECE);
    uint32_t opponent = o_turn ? x_board : o_board;

    const char direction_count = (pos & kings) ? 4 : 2;
    bool left = true, forward = true;
    uint32_t new_pos;
    for (char i = 0; i < direction_count; i++)
    {
        new_pos = move(pos, left, forward);
        if (is_space(new_pos))
            return true;
        else if (new_pos & opponent)
        {
            new_pos = move(new_pos, left, forward, o_turn);
            if (is_space(new_pos))
                return true;

        }

        left = !left;
        if (left)
            forward = !forward;
    }

    return false;
}

bool is_winner(bool o_turn)
{
    uint32_t opponent = o_turn ? x_board : o_board;
    for (uint32_t pos = 1; pos != 0; pos <<= 1)
        if ((pos & opponent) && has_move(pos))
            return false;

    return true;
}

void change_turn()
{
    uint32_t king_rank = o_turn ? X_BACK_RANK : O_BACK_RANK;
    uint32_t player_board = o_turn ? o_board : x_board;
    kings |= player_board & king_rank;

    if (o_turn)
        o_wins = is_winner(o_turn);
    else
        x_wins = is_winner(o_turn);

    o_turn = !o_turn;
}

char getTurnChar()
{
    return o_turn ? O_PIECE : X_PIECE;
}

int main()
{
    string message = "";
    bool next_capture[] = { false, false, false, false, false };

    ostringstream str_stream;

    while (true)
    {
        //system("pause");
        system("@cls||clear");
        if (o_wins)
            cout << "Winner: O\n";
        else if (x_wins)
            cout << "Winner: X\n";
        else
            cout << "Player: " << getTurnChar() << endl;

        str_stream << "\n     1   2   3   4   5   6   7   8\n";
        str_stream << "    --------------------------------\n";

        bool even = false;
        int row_num = 1;
        int col_cycle = 0;
        char piece;
        for (uint32_t pos = 1 << 28; pos; pos >>= 4)
        {
            str_stream << " " << row_num << " ";
            if (!even)
                str_stream << "| ";

            for (col_cycle = 0; col_cycle < 4; col_cycle++)
            {
                piece = getPiece(pos << col_cycle);
                if (even)
                    str_stream << "| " << piece << " | " << SPACE << " ";
                else
                    str_stream << SPACE << " | " << piece << " | ";
            }

            if (even)
                str_stream << "|";

            str_stream << "\n    --------------------------------\n";
            even = !even;
            row_num++;
        }

        cout << str_stream.str();
        str_stream.str(string());
        str_stream.clear();

        if (o_wins || x_wins)
        {
            cout << "\nPlayer " << (o_wins ? O_PIECE : X_PIECE) << " has won the game!\n\n";
            break;
        }

        if (!o_turn)
        {
            playerAI.next_move(!o_turn, o_board, x_board, kings, o_wins, x_wins);
            o_board = playerAI.o_board();
            x_board = playerAI.x_board();
            kings = playerAI.kings();

            change_turn();
            continue;
        }

        cout << endl;
        if (!message.empty())
        {
            cout << message << "\n\n";
            message = "";
        }

        if (next_capture[0])
        {
            cout << "N to finish";
            if (next_capture[1] || next_capture[3])
                cout << ", L for left";
            if (next_capture[2] || next_capture[4])
                cout << ", R for right";
            cout << ": ";
            string direction;
            cin >> direction;

            if (direction == "n")
            {
                o_turn = !o_turn;
                next_capture[0] = false;
            }
            else if (direction == "l" && (next_capture[1] || next_capture[3]) ||
                     direction == "r" && (next_capture[2] || next_capture[4]))
            {
                char dir_index = (direction == "l") ? 1 : 2;
                bool forward = next_capture[dir_index];
                if (forward && next_capture[dir_index + 2])
                {
                    cout << "F for forwards, B for backwards: ";
                    string forward_string;
                    cin >> forward_string;

                    if (forward_string != "f" && forward_string != "b")
                    {
                        message = "Movement should be F for forwards or B for backwards";
                        continue;
                    }

                    forward = forward_string == "f";
                }

                uint32_t capture_pos = move(piece_pos, direction == "l", forward);
                uint32_t new_pos = move(capture_pos, direction == "l", forward, o_turn);

                add_piece(o_turn, new_pos, piece_pos & kings);
                remove_piece(!o_turn, capture_pos);
                remove_piece(o_turn, piece_pos);

                piece_pos = new_pos;
                available_captures(next_capture, piece_pos);

                if (next_capture[0])
                    message = "You can continue capturing";
                else
                    change_turn();
            }
            else
            {
                message = "Movement should be N to finish";
                if (next_capture[1] || next_capture[3])
                    message += " or L for left";
                if (next_capture[2] || next_capture[4])
                    message += " or R for right";
            }
            continue;
        }

        // get x and y coordinates and convert them to bits...
        int piece_x, piece_y;
        string direction;
        cout << "Row of piece position (1,8): ";
        cin >> piece_y;
        cout << "Column of piece position (1,8): ";
        cin >> piece_x;
        cout << "L for left, R for right: ";
        cin >> direction;

        if (piece_x < 1 || piece_x > 8 || piece_y < 1 || piece_y > 8)
        {
            message = "Expected valid column and row numbers";
            continue;
        }

        if (direction != "l" && direction != "r")
        {
            message = "Movement should be L (left) or R (right)";
            continue;
        }

        piece_x = piece_x - 1 - (piece_y % 2);
        if (piece_x % 2 == 1)
        {
            message = "Empty square chosen";
            continue;
        }

        piece_x = piece_x / 2;
        piece_y = 8 - piece_y;
        piece_pos = 1 << (piece_y * 4 + piece_x);

        if (is_space(piece_pos))
        {
            message = "Empty square chosen";
            continue;
        }
        if (!same_side(piece_pos, getTurnChar()))
        {
            message = "Enemy piece chosen";
            continue;
        }

        bool forward = true;
        if (piece_pos & kings)
        {
            cout << "F for forwards, B for backwards: ";
            string forward_input;
            cin >> forward_input;

            if (forward_input != "f" && forward_input != "b")
            {
                message = "Movement should be F for forwards or B for backwards";
                continue;
            }

            forward = forward_input == "f";
        }

        uint32_t new_pos = move(piece_pos, direction == "l", forward);
        if (!new_pos)
        {
            message = "Cannot leave board";
            continue;
        }

        if (same_side(new_pos, getTurnChar()))
        {
            message = "Cannot capture own piece";
            continue;
        }
        else if (is_space(new_pos))
        {
            add_piece(o_turn, new_pos, piece_pos & kings);
            remove_piece(o_turn, piece_pos);
        }
        else
        {
            uint32_t capture_pos = new_pos;
            new_pos = move(capture_pos, direction == "l", forward, o_turn);

            if (!new_pos)
            {
                message = "Cannot leave board";
                continue;
            }

            if (is_space(new_pos))
            {
                add_piece(o_turn, new_pos, piece_pos & kings);
                remove_piece(!o_turn, capture_pos);
                remove_piece(o_turn, piece_pos);

                piece_pos = new_pos;
                available_captures(next_capture, piece_pos);
                if (next_capture[0])
                {
                    message = "You can continue capturing";
                    continue;
                }
            }
            else
            {
                message = "Capture blocked";
                continue;
            }
        }

        change_turn();
    }
}
