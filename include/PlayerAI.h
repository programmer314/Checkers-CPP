#ifndef PLAYERAI_H
#define PLAYERAI_H

#include <vector>
#include <unordered_map>

using namespace std;

struct ScoreMovePair {
    uint32_t move;
    float score;
};

// board is {uint32_t x_board, uint32_t o_board, uint32_t kings}

class PlayerAI
{
    public:
        PlayerAI(uint32_t o_board, uint32_t x_board, uint32_t kings);
        virtual ~PlayerAI();

        void minimax(int depth, bool max_player);
        void alphabeta(int depth, bool max_player);

        void next_move(bool player, uint32_t o_board, uint32_t x_board, uint32_t kings, bool o_wins, bool x_wins);

        uint32_t o_board() { return this->board[1]; }
        uint32_t x_board() { return this->board[0]; }
        uint32_t kings() { return this->board[2]; }

    private:
        static const uint32_t EVEN_MASK = 252645135;
        static const uint32_t ROW_END_MASK = 4026531855;
        static const uint32_t O_BACK_RANK = 15;
        static const uint32_t X_BACK_RANK = 4026531840;
        static const uint32_t L_COL_MASK = 16843009;
        static const uint32_t R_COL_MASK = 2155905152;

        uint32_t board[3];
        bool o_wins = false, x_wins = false;

        ScoreMovePair minimax(uint32_t board[], int depth, bool max_player);
        ScoreMovePair alphabeta(uint32_t board[], int depth, bool max_player, float alpha, float beta);

        void update(uint32_t o_board, uint32_t x_board, uint32_t kings, bool o_wins, bool x_wins);
        float evaluate(uint32_t board[]);

        unordered_map<uint32_t, uint32_t> get_piece_moves(uint32_t board[], uint32_t pos);
        unordered_map<uint32_t, uint32_t> get_piece_moves(uint32_t board[], uint32_t pos, uint32_t origin, uint32_t captures);
        void simulate_move(uint32_t board[], uint32_t new_board[], uint32_t move, bool is_o, bool promote);
        vector<uint32_t> get_all_moves(uint32_t board[], bool is_o);

        uint32_t move(uint32_t pos, bool left, bool forward, bool o_turn);
        int bitsize(uint32_t num);
};

#endif // PLAYERAI_H
