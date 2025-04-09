#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <chrono>
#include "game.h"
#include "settings.h"
#include "move_generator.h"
#include "minimax.h"
#include "move.h"
#include "board.h"

namespace py = pybind11;

class AbaloneGameWrapper {
public:
    AbaloneGameWrapper() {
        GameSettings settings;
        settings.setBoardLayout(BoardLayout::STANDARD);
        settings.setPlayer1Color(PlayerColour::BLACK); // Player 1 = Black, Player 2 = White (AI)
        settings.setGameMode(GameMode::PLAYER_VS_COMPUTER);
        game = std::make_unique<Game>(settings);
    }

    std::tuple<std::string, std::vector<std::pair<int, int>>, std::vector<std::pair<int, int>>, double>
    make_player_and_ai_move(const std::vector<std::pair<int, int>>& coords, const std::string& direction_str) {
        // Convert Python coords (col, row) to C++ (row, col) and adjust to 0-based indexing
        std::vector<std::pair<int, int>> adjusted_coords;
        for (const auto& [col, row] : coords) {
            int row_cpp = 9 - row; // Convert row from 1-9 (bottom-up) to 0-8 (top-down)
            int col_cpp = col - 1; // Convert col from 1-9 to 0-8
            adjusted_coords.emplace_back(row_cpp, col_cpp);
        }

        // Get direction and infer move type
        MoveDirection move_dir = string_to_direction(direction_str);
        MoveType move_type = infer_move_type(adjusted_coords, move_dir);

        // Create Move object
        Move player_move(move_type, move_dir);
        for (const auto& [row, col] : adjusted_coords) {
            player_move.addPosition(row, col);
        }

        // Validate and apply player move
        MoveGenerator move_gen;
        move_gen.generateMoves(game->getCurrentPlayer(), game->getBoard());
        const auto& valid_moves = move_gen.getGeneratedMoves();

        if (valid_moves.find(player_move) == valid_moves.end()) {
            return {"INVALID", {}, {}, 0.0};
        }

        game->applyMove(player_move);
        if (game->getCurrentPlayer() == 1) game->incrementMoveCountP1();
        game->switchPlayer();

        // AI move
        auto start_time = std::chrono::high_resolution_clock::now();
        Move ai_move = game->getAI().findBestMove(game->getBoard(), game->getCurrentPlayer());
        game->applyMove(ai_move);
        auto end_time = std::chrono::high_resolution_clock::now();
        double ai_time = std::chrono::duration<double>(end_time - start_time).count();
        if (game->getCurrentPlayer() == 2) game->incrementMoveCountP2();
        game->switchPlayer();

        // Get updated board positions
        auto [black_pos, white_pos] = get_board_positions();

        return {"VALID", black_pos, white_pos, ai_time};
    }

private:
    std::unique_ptr<Game> game;

    MoveType infer_move_type(const std::vector<std::pair<int, int>>& coords, MoveDirection dir) {
        if (coords.size() == 1) return MoveType::INLINE;

        // Get direction delta
        auto [dx_dir, dy_dir] = DirectionHelper::getDelta(dir);

        // Check if marbles are in a line
        bool is_inline = true;
        if (coords.size() == 2) {
            int dx = coords[1].first - coords[0].first;
            int dy = coords[1].second - coords[0].second;
            // Inline if direction aligns with marble line
            is_inline = (dx == dx_dir && dy == dy_dir) || (dx == -dx_dir && dy == -dy_dir);
        } else if (coords.size() == 3) {
            int dx1 = coords[1].first - coords[0].first;
            int dy1 = coords[1].second - coords[0].second;
            int dx2 = coords[2].first - coords[1].first;
            int dy2 = coords[2].second - coords[1].second;
            if (dx1 == dx2 && dy1 == dy2) {
                is_inline = (dx1 == dx_dir && dy1 == dy_dir) || (dx1 == -dx_dir && dy1 == -dy_dir);
            } else {
                is_inline = false; // Not in a straight line, must be sidestep
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
        throw std::runtime_error("Unknown direction: " + dir);
    }

    std::pair<std::vector<std::pair<int, int>>, std::vector<std::pair<int, int>>>
    get_board_positions() {
        std::vector<std::pair<int, int>> black_pos, white_pos;
        const auto& board = game->getBoard().getBoard();
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (board[i][j] == 1 && game->getSettings().getPlayerColourMap().at(PlayerColour::BLACK) == 1) {
                    black_pos.emplace_back(j + 1, 9 - i); // Convert back to Python (col, row)
                } else if (board[i][j] == 2 && game->getSettings().getPlayerColourMap().at(PlayerColour::WHITE) == 2) {
                    white_pos.emplace_back(j + 1, 9 - i);
                }
            }
        }
        return {black_pos, white_pos};
    }
};

PYBIND11_MODULE(abalone_cpp, m) {
    py::class_<AbaloneGameWrapper>(m, "AbaloneGame")
        .def(py::init<>())
        .def("make_player_and_ai_move", &AbaloneGameWrapper::make_player_and_ai_move);
}