#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <chrono>
#include <random>
#include "game.h"
#include "settings.h"
#include "move_generator.h"
#include "minimax.h"
#include "move.h"
#include "board.h"

namespace py = pybind11;

class AbaloneGameWrapper {
public:
    AbaloneGameWrapper(int player1_color, int board_layout, int max_moves, int time_p1, int time_p2, int game_mode) {
        GameSettings settings;
        settings.setBoardLayout(static_cast<BoardLayout>(board_layout));
        settings.setPlayer1Color(static_cast<PlayerColour>(player1_color));
        settings.setGameMode(static_cast<GameMode>(game_mode));
        settings.setMoveLimit(max_moves);
        settings.setTimeLimits(true, time_p1, time_p2);
        game = std::make_unique<Game>(settings);
        this->max_moves = max_moves;
        this->time_p1 = time_p1;
        this->time_p2 = time_p2;
    }

    std::tuple<std::string, std::vector<std::pair<int, int>>, std::vector<std::pair<int, int>>, double>
    make_player_and_ai_move(const std::vector<std::pair<int, int>>& coords, const std::string& direction_str) {
        if (game->getMoveCountP1() + game->getMoveCountP2() >= max_moves) {
            auto [black_pos, white_pos] = get_board_positions();
            return {"MAX_MOVES", black_pos, white_pos, 0.0};
        }

        bool is_first_move = (game->getMoveCountP1() == 0 && game->getMoveCountP2() == 0);
        bool ai_is_black = (game->getSettings().getPlayerColourMap().at(PlayerColour::BLACK) == 2);
        bool has_player_move = !coords.empty();

        if (is_first_move && ai_is_black && !has_player_move) {
            // AI (Black) makes a random first move
            MoveGenerator move_gen;
            move_gen.generateMoves(2, game->getBoard()); // Player 2 (Black)
            const auto& valid_moves = move_gen.getGeneratedMoves();
            if (valid_moves.empty()) {
                auto [black_pos, white_pos] = get_board_positions();
                return {"INVALID", black_pos, white_pos, 0.0};
            }

            std::vector<Move> move_list(valid_moves.begin(), valid_moves.end());
            static std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<int> dist(0, move_list.size() - 1);
            Move ai_move = move_list[dist(rng)];

            auto start_time = std::chrono::high_resolution_clock::now();
            last_ai_undo = MoveUndo();
            game->applyMoveWithUndo(ai_move, last_ai_undo);
            auto end_time = std::chrono::high_resolution_clock::now();
            double ai_time = std::chrono::duration<double>(end_time - start_time).count();
            game->incrementMoveCountP2();
            game->switchPlayer();

            auto [black_pos, white_pos] = get_board_positions();
            return {"VALID", black_pos, white_pos, ai_time};
        }

        if (!has_player_move) {
            // No player move provided when expected
            auto [black_pos, white_pos] = get_board_positions();
            return {"INVALID", black_pos, white_pos, 0.0};
        }

        std::vector<std::pair<int, int>> adjusted_coords;
        for (const auto& [col, row] : coords) {
            int row_cpp = 9 - row; // Convert to 0-based, top-down
            int col_cpp = col - 1; // Convert to 0-based
            adjusted_coords.emplace_back(row_cpp, col_cpp);
        }

        MoveDirection move_dir = string_to_direction(direction_str);
        MoveType move_type = infer_move_type(adjusted_coords, move_dir);

        Move player_move(move_type, move_dir);
        for (const auto& [row, col] : adjusted_coords) {
            player_move.addPosition(row, col);
        }

        MoveGenerator move_gen;
        move_gen.generateMoves(game->getCurrentPlayer(), game->getBoard());
        const auto& valid_moves = move_gen.getGeneratedMoves();

        if (valid_moves.find(player_move) == valid_moves.end()) {
            auto [black_pos, white_pos] = get_board_positions();
            return {"INVALID", black_pos, white_pos, 0.0};
        }

        // Apply player move (Player 1)
        last_player_undo = MoveUndo();
        game->applyMoveWithUndo(player_move, last_player_undo);
        game->incrementMoveCountP1();
        game->switchPlayer();

        // AI move (Player 2)
        auto start_time = std::chrono::high_resolution_clock::now();
        Move ai_move = game->getAI().findBestMove(game->getBoard(), game->getCurrentPlayer());
        last_ai_undo = MoveUndo();
        game->applyMoveWithUndo(ai_move, last_ai_undo);
        auto end_time = std::chrono::high_resolution_clock::now();
        double ai_time = std::chrono::duration<double>(end_time - start_time).count();
        game->incrementMoveCountP2();
        game->switchPlayer();

        auto [black_pos, white_pos] = get_board_positions();
        return {"VALID", black_pos, white_pos, ai_time};
    }

    void undo_last_move() {
        if (last_ai_undo.changes.empty() && last_player_undo.changes.empty()) {
            return; // Nothing to undo
        }

        // Undo AI move first (if it exists)
        if (!last_ai_undo.changes.empty()) {
            game->getBoard().unmakeMove(last_ai_undo);
            game->decrementMoveCountP2();
            game->switchPlayer();
        }

        // Undo player move (if it exists)
        if (!last_player_undo.changes.empty()) {
            game->getBoard().unmakeMove(last_player_undo);
            game->decrementMoveCountP1();
            game->switchPlayer();
        }

        // Clear undo data
        last_player_undo = MoveUndo();
        last_ai_undo = MoveUndo();
    }

    std::pair<std::vector<std::pair<int, int>>, std::vector<std::pair<int, int>>>
    get_initial_state() {
        return get_board_positions();
    }

    std::pair<std::vector<std::pair<int, int>>, std::vector<std::pair<int, int>>>
    get_current_state() {
        return get_board_positions();
    }

private:
    std::unique_ptr<Game> game;
    int max_moves;
    int time_p1;
    int time_p2;
    MoveUndo last_player_undo;
    MoveUndo last_ai_undo;

    MoveType infer_move_type(const std::vector<std::pair<int, int>>& coords, MoveDirection dir) {
        if (coords.size() == 1) return MoveType::INLINE;
        auto [dx_dir, dy_dir] = DirectionHelper::getDelta(dir);
        bool is_inline = true;
        if (coords.size() == 2) {
            int dx = coords[1].first - coords[0].first;
            int dy = coords[1].second - coords[0].second;
            is_inline = (dx == dx_dir && dy == dy_dir) || (dx == -dx_dir && dy == -dy_dir);
        } else if (coords.size() == 3) {
            int dx1 = coords[1].first - coords[0].first;
            int dy1 = coords[1].second - coords[0].second;
            int dx2 = coords[2].first - coords[1].first;
            int dy2 = coords[2].second - coords[1].second;
            if (dx1 == dx2 && dy1 == dy2) {
                is_inline = (dx1 == dx_dir && dy1 == dy_dir) || (dx1 == -dx_dir && dy1 == -dy_dir);
            } else {
                is_inline = false;
            }
        }
        return is_inline ? MoveType::INLINE : MoveType::SIDESTEP;
    }

    MoveDirection string_to_direction(const std::string& dir) {
        if (dir == "W") return MoveDirection::W;
        if (dir == "NW") return MoveDirection::NW;
        if (dir == "NE") return MoveDirection::NE;
        if (dir == "E") return MoveDirection::E;
        if (dir == "SE") return MoveDirection::SE;
        if (dir == "SW") return MoveDirection::SW;
        if (dir == "RANDOM") return MoveDirection::E; // Dummy value for random move
        throw std::runtime_error("Unknown direction: " + dir);
    }

    std::pair<std::vector<std::pair<int, int>>, std::vector<std::pair<int, int>>>
    get_board_positions() {
        std::vector<std::pair<int, int>> black_pos, white_pos;
        const auto& board = game->getBoard().getBoard();
        auto color_map = game->getSettings().getPlayerColourMap();
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (board[i][j] == color_map.at(PlayerColour::BLACK)) {
                    black_pos.emplace_back(j + 1, 9 - i); // (col, row) for Python
                } else if (board[i][j] == color_map.at(PlayerColour::WHITE)) {
                    white_pos.emplace_back(j + 1, 9 - i);
                }
            }
        }
        return {black_pos, white_pos};
    }
};

PYBIND11_MODULE(abalone_cpp, m) {
    py::class_<AbaloneGameWrapper>(m, "AbaloneGame")
        .def(py::init<int, int, int, int, int, int>())
        .def("make_player_and_ai_move", &AbaloneGameWrapper::make_player_and_ai_move)
        .def("undo_last_move", &AbaloneGameWrapper::undo_last_move)
        .def("get_initial_state", &AbaloneGameWrapper::get_initial_state)
        .def("get_current_state", &AbaloneGameWrapper::get_current_state);
}