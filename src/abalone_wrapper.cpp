#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "game.h"
#include "settings.h"

namespace py = pybind11;

// Wrapper class to interface with Python
class AbaloneGameWrapper {
public:
    AbaloneGameWrapper() {
        GameSettings settings;
        settings.setBoardLayout(BoardLayout::STANDARD);
        settings.setPlayer1Color(PlayerColour::BLACK); // Player 1 = Black, Player 2 = White (AI)
        settings.setGameMode(GameMode::PLAYER_VS_COMPUTER);
        game = std::make_unique<Game>(settings);
    }

    // Process a player move and AI response
    std::tuple<std::string, std::vector<std::pair<int, int>>, std::vector<std::pair<int, int>>, double>
    make_player_and_ai_move(const std::vector<std::pair<int, int>>& coords, const std::string& direction_str) {
        // Convert Python coords (col, row) to C++ (row, col) and adjust to 0-based indexing
        std::vector<std::pair<int, int>> adjusted_coords;
        for (const auto& [col, row] : coords) {
            int row_cpp = 9 - row; // Convert row from 1-9 (bottom-up) to 0-8 (top-down)
            int col_cpp = col - 1; // Convert col from 1-9 to 0-8
            adjusted_coords.emplace_back(row_cpp, col_cpp);
        }

        // Determine move type (INLINE or SIDESTEP) based on coords
        MoveType move_type = infer_move_type(adjusted_coords);
        MoveDirection move_dir = string_to_direction(direction_str);

        // Create Move object
        Move player_move(move_type, move_dir);
        for (const auto& [row, col] : adjusted_coords) {
            player_move.addPosition(row, col);
        }

        // Validate and apply player move
        MoveGenerator move_gen;
        move_gen.generateMoves(game->currentPlayer, game->board);
        const auto& valid_moves = move_gen.getGeneratedMoves();

        if (valid_moves.find(player_move) == valid_moves.end()) {
            return {"INVALID", {}, {}, 0.0};
        }

        game->board.applyMove(player_move);
        if (game->currentPlayer == 1) game->moveCountP1++;
        game->switchPlayer();

        // AI move
        auto start_time = std::chrono::high_resolution_clock::now();
        Move ai_move = game->ai.findBestMove(game->board, game->currentPlayer);
        game->board.applyMove(ai_move);
        auto end_time = std::chrono::high_resolution_clock::now();
        double ai_time = std::chrono::duration<double>(end_time - start_time).count();
        if (game->currentPlayer == 2) game->moveCountP2++;
        game->switchPlayer();

        // Get updated board positions
        auto [black_pos, white_pos] = get_board_positions();

        return {"VALID", black_pos, white_pos, ai_time};
    }

private:
    std::unique_ptr<Game> game;

    MoveType infer_move_type(const std::vector<std::pair<int, int>>& coords) {
        if (coords.size() == 1) return MoveType::INLINE;

        // Check if coords are in a straight line (INLINE)
        int dx1 = coords[1].first - coords[0].first;
        int dy1 = coords[1].second - coords[0].second;
        if (coords.size() == 2) {
            return (abs(dx1) <= 1 && abs(dy1) <= 1) ? MoveType::INLINE : MoveType::SIDESTEP;
        }
        int dx2 = coords[2].first - coords[1].first;
        int dy2 = coords[2].second - coords[1].second;
        return (dx1 == dx2 && dy1 == dy2) ? MoveType::INLINE : MoveType::SIDESTEP;
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
        const auto& board = game->board.getBoard();
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (board[i][j] == 1 && game->settings.getPlayerColourMap().at(PlayerColour::BLACK) == 1) {
                    black_pos.emplace_back(j + 1, 9 - i); // Convert back to Python (col, row)
                } else if (board[i][j] == 2 && game->settings.getPlayerColourMap().at(PlayerColour::WHITE) == 2) {
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
// namespace py = pybind11;
//
// // Converts "NE", "SW", etc. to MoveDirection enum
// MoveDirection stringToMoveDirection(const std::string& dir) {
//     if (dir == "NW") return MoveDirection::NW;
//     if (dir == "NE") return MoveDirection::NE;
//     if (dir == "E")  return MoveDirection::E;
//     if (dir == "SE") return MoveDirection::SE;
//     if (dir == "SW") return MoveDirection::SW;
//     if (dir == "W")  return MoveDirection::W;
//     throw std::invalid_argument("Invalid direction string: " + dir);
// }
//
// // Guess move type based on 1-3 coords
// Move inferMoveFromInput(const std::vector<std::pair<int, int>>& coords, const std::string& dirStr) {
//     MoveDirection dir = stringToMoveDirection(dirStr);
//     MoveType type = MoveType::INLINE;
//
//     if (coords.size() == 1) {
//         type = MoveType::INLINE;
//     } else if (coords.size() == 2 || coords.size() == 3) {
//         int dx1 = coords[1].first - coords[0].first;
//         int dy1 = coords[1].second - coords[0].second;
//         auto [ddx, ddy] = DirectionHelper::getDelta(dir);
//         if (dx1 == ddx && dy1 == ddy) {
//             type = MoveType::INLINE;
//         } else {
//             type = MoveType::SIDESTEP;
//         }
//     }
//
//     Move move(type, dir);
//     for (const auto& [c, r] : coords) {
//         move.addPosition(c, r);
//     }
//     return move;
// }
//
// py::tuple make_player_and_ai_move(std::vector<std::pair<int, int>> coords, std::string direction) {
//     // Set up game settings
//     GameSettings settings;
//     settings.setBoardLayout(BoardLayout::STANDARD);
//     settings.setPlayer1Color(PlayerColour::BLACK);
//     settings.setGameMode(GameMode::PLAYER_VS_COMPUTER);
//
//     // Init board
//     Board board = Game(settings).generateStandardBoard();
//     int currentPlayer = settings.getPlayerColourMap().at(PlayerColour::BLACK);
//
//     // Generate move
//     Move move = inferMoveFromInput(coords, direction);
//     MoveGenerator gen;
//     gen.generateMoves(currentPlayer, board);
//     const auto& validMoves = gen.getGeneratedMoves();
//
//     if (validMoves.find(move) == validMoves.end()) {
//         return py::make_tuple("INVALID", py::none(), py::none(), 0.0);
//     }
//
//     // Apply player move
//     board.applyMove(move);
//
//     // Run AI move
//     int aiPlayer = 2;
//     Minimax ai(3);
//
//     auto start = std::chrono::high_resolution_clock::now();
//     Move aiMove = ai.findBestMove(board, aiPlayer);
//     auto end = std::chrono::high_resolution_clock::now();
//     double elapsed = std::chrono::duration<double>(end - start).count();
//
//     board.applyMove(aiMove);
//
//     // Get new marble positions
//     std::vector<std::pair<int, int>> black;
//     std::vector<std::pair<int, int>> white;
//
//     auto boardArr = board.getBoard();
//     for (int r = 0; r < ROWS; ++r) {
//         for (int c = 0; c < COLS; ++c) {
//             if (boardArr[r][c] == 1) black.emplace_back(c, r);
//             else if (boardArr[r][c] == 2) white.emplace_back(c, r);
//         }
//     }
//
//     return py::make_tuple("OK", black, white, elapsed);
// }
//
// PYBIND11_MODULE(abalone_cpp, m) {
//     m.def("make_player_and_ai_move", &make_player_and_ai_move, "Apply a player move, return new board and AI response");
// }
