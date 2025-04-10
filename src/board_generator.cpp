// #include "board_generator.h"

// void BoardGenerator::generateBoards(const Board& currBoard, const std::set<Move>& moves)
// {
//     GeneratedBoards.clear();
//     GeneratedBoards.reserve(moves.size());

//     for (const Move& m : moves) {
//         auto b = std::make_unique<Board>(currBoard);
//         b->applyMove(m);
//         GeneratedBoards.push_back(std::move(b));
//     }
// }

// const std::vector<std::unique_ptr<Board>>& BoardGenerator::getGeneratedBoards() const
// {
//     return GeneratedBoards;
// }

// void BoardGenerator::printBoards() const
// {
//     for (const auto& b : GeneratedBoards) {
//         b->printPieces();
//     }
// }