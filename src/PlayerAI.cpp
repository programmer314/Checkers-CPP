#include "PlayerAI.h"
#include <vector>
#include <unordered_map>
#include <cfloat>
#include <algorithm>

#define USE_ALPHABETA true
#define DEPTH 11

using namespace std;

PlayerAI::PlayerAI(uint32_t o_board, uint32_t x_board, uint32_t kings)
{
    update(o_board, x_board, kings, false, false);
}

PlayerAI::~PlayerAI()
{
}

void PlayerAI::minimax(int depth, bool max_player)
{
    uint32_t best_move = minimax(this->board, depth, max_player).move;
    simulate_move(this->board, this->board, best_move, !max_player, true);
}

ScoreMovePair PlayerAI::minimax(uint32_t board[], int depth, bool max_player)
{
    ScoreMovePair result;

    if (depth == 0)
    {
        result.score = evaluate(board);
        return result;
    }

    float evalLimit;
    uint32_t best_move;

    if (max_player)
    {
        evalLimit = -FLT_MAX;
        for (uint32_t move : get_all_moves(board, false))
        {
            uint32_t temp_board[3];
            simulate_move(board, temp_board, move, false, true);

            float evaluation = minimax(temp_board, depth - 1, false).score;
            if (evalLimit < evaluation)
                best_move = move;

            evalLimit = max(evalLimit, evaluation);
        }
    }
    else
    {
        evalLimit = FLT_MAX;
        for (uint32_t move : get_all_moves(board, true))
        {
            uint32_t temp_board[3];
            simulate_move(board, temp_board, move, true, true);

            float evaluation = minimax(temp_board, depth - 1, true).score;
            if (evalLimit > evaluation)
                best_move = move;

            evalLimit = min(evalLimit, evaluation);
        }
    }

    result.move = best_move;
    result.score = best_move ? evalLimit : evaluate(board);

    return result;
}

void PlayerAI::alphabeta(int depth, bool max_player)
{
    uint32_t best_move = alphabeta(this->board, depth, max_player, -FLT_MAX, FLT_MAX).move;
    simulate_move(this->board, this->board, best_move, !max_player, true);
}

ScoreMovePair PlayerAI::alphabeta(uint32_t board[], int depth, bool max_player, float alpha, float beta)
{
    ScoreMovePair result;

    if (depth == 0)
    {
        result.score = evaluate(board);
        return result;
    }

    float evalLimit;
    uint32_t best_move;

    if (max_player)
    {
        evalLimit = -FLT_MAX;
        for (uint32_t move : get_all_moves(board, false))
        {
            uint32_t temp_board[3];
            simulate_move(board, temp_board, move, false, true);

            float evaluation = alphabeta(temp_board, depth - 1, false, alpha, beta).score;
            if (evalLimit < evaluation)
                best_move = move;

            evalLimit = max(evalLimit, evaluation);

            alpha = max(alpha, evalLimit);
            if (beta <= alpha)
                break;
        }
    }
    else
    {
        evalLimit = FLT_MAX;
        for (uint32_t move : get_all_moves(board, true))
        {
            uint32_t temp_board[3];
            simulate_move(board, temp_board, move, true, true);

            float evaluation = alphabeta(temp_board, depth - 1, true, alpha, beta).score;
            if (evalLimit > evaluation)
                best_move = move;

            evalLimit = min(evalLimit, evaluation);

            beta = min(beta, evalLimit);
            if (beta <= alpha)
                break;
        }
    }

    result.move = best_move;
    result.score = best_move ? evalLimit : evaluate(board);

    return result;
}

void PlayerAI::update(uint32_t o_board, uint32_t x_board, uint32_t kings, bool o_wins, bool x_wins)
{
    this->board[0] = x_board;
    this->board[1] = o_board;
    this->board[2] = kings;

    this->o_wins = o_wins;
    this->x_wins = x_wins;
}

void PlayerAI::next_move(bool player, uint32_t o_board, uint32_t x_board, uint32_t kings, bool o_wins, bool x_wins)
{
    update(o_board, x_board, kings, o_wins, x_wins);

    if (USE_ALPHABETA)
        alphabeta(DEPTH, player);
    else
        minimax(DEPTH, player);
}

float PlayerAI::evaluate(uint32_t board[])
{
    int scores[4];

    scores[0] = bitsize(board[0] & (~board[2])); // X pieces
    scores[1] = bitsize(board[1] & (~board[2])); // O pieces
    scores[2] = bitsize(board[0] & board[2]);    // X kings
    scores[3] = bitsize(board[1] & board[2]);    // O kings

    return (scores[0] - scores[1]) * 3 + (scores[2] - scores[3]) * 5;
}

unordered_map<uint32_t, uint32_t> PlayerAI::get_piece_moves(uint32_t board[], uint32_t pos, uint32_t origin, uint32_t captures)
{
    unordered_map<uint32_t, uint32_t> moves;

    const uint32_t x_board = board[0], o_board = board[1], kings = board[2];
    bool o_turn = pos & o_board;
    const uint32_t player = board[o_turn], opponent = board[!o_turn];

    if (captures)
        moves[pos | origin] = captures;

    bool forward = true, left = true;
    do
    {
        uint32_t new_pos = move(pos, left, forward, o_turn);

        if (new_pos)
        {
            if (!captures && !(new_pos & (o_board | x_board)))
                moves[pos | new_pos] = 0;
            else
            {
                uint32_t capture_pos = new_pos;
                uint32_t new_pos = move(capture_pos, left, forward, o_turn);

                if (new_pos && !(captures & capture_pos) && (capture_pos & opponent) &&
                    ((new_pos & origin) || !(new_pos & (o_board | x_board))))
                {
                    uint32_t new_board[3];
                    simulate_move(board, new_board, pos | new_pos | capture_pos, o_turn, false);

                    for (const auto& cur_pos : get_piece_moves(new_board, new_pos, origin, captures | capture_pos))
                    {
                        auto prev_move = moves.find(cur_pos.first);
                        if (prev_move == moves.end())
                            moves.insert(cur_pos);
                        else if (bitsize(prev_move->second) < bitsize(cur_pos.second))
                            prev_move->second = cur_pos.second;
                    }
                }
            }
        }

        left = !left;
        if (left && (kings & pos))
            forward = !forward;

    } while (!left || !forward);

    return moves;
}

unordered_map<uint32_t, uint32_t> PlayerAI::get_piece_moves(uint32_t board[], uint32_t pos)
{
    return get_piece_moves(board, pos, pos, 0);
}

void PlayerAI::simulate_move(uint32_t board[], uint32_t new_board[], uint32_t move, bool is_o, bool promote)
{
    uint32_t captures = move & board[!is_o];
    move &= ~captures;

    new_board[is_o] = board[is_o];
    if (move & ~board[is_o])
        new_board[is_o] ^= move;

    new_board[!is_o] = board[!is_o] & ~captures;

    new_board[2] = board[2] & ~captures;
    if ((new_board[2] & move) && (move & ~new_board[2]))
        new_board[2] ^= move;

    if (promote)
        new_board[2] |= (new_board[0] & O_BACK_RANK) | (new_board[1] & X_BACK_RANK);
}

vector<uint32_t> PlayerAI::get_all_moves(uint32_t board[], bool is_o)
{
    vector<uint32_t> moves;

    for (uint32_t pos = 1; pos > 0; pos <<= 1)
        if (board[is_o] & pos)
            for (const auto& move_pair : get_piece_moves(board, pos))
                moves.push_back((move_pair.first) | (move_pair.second));

    return moves;
}

// return 0 if move is out of bounds
uint32_t PlayerAI::move(uint32_t pos, bool left, bool forward, bool o_turn)
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

int PlayerAI::bitsize(uint32_t num)
{
    int count = num & 1;
    while(num >>= 1)
        count += num & 1;

    return count;
}
