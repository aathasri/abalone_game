#include "move_generator.h"

void MoveGenerator::generateMoves(int currentPlayerColour, const Board& currentBoard)
{
    const std::array<std::array<int, COLS>, ROWS>& board = currentBoard.getBoard();

    int opponentPlayerColour = 3 - currentPlayerColour;

    int countCurrentPlayer = 0;

    // iterate through pieces on the board
    for (int i = ROWS - 1; i > 0; i--) {
        for (int j = 0; j < COLS; j++) {

            bool validCurrPos = currentBoard.validPosition(i,j);

            if (validCurrPos && board[i][j] == currentPlayerColour) {
                
                // iterate through directions
                for (int k = 0; k < static_cast<int>(MoveDirection::COUNT); k++) {

                    // get movement indexes
                    int movementLetIndex = DirectionHelper::getDelta(k).first;
                    int movementNumIndex = DirectionHelper::getDelta(k).second;

                    bool potentialPositionValid = currentBoard.validPosition(i + movementLetIndex, j + movementNumIndex);
                                        
                    //if the position in the direction of movment is empty, a single can occur; a double or triple might be possible
                    if (potentialPositionValid && board[i + movementLetIndex][j+ movementNumIndex] == 0) {
                        Move singleEmptyInline = Move(MoveType::INLINE, static_cast<MoveDirection>(k));
                        singleEmptyInline.addPosition(i, j);
                        generated_moves.insert(singleEmptyInline);

                        // CHECK FOR SIDESTEP
                        MoveDirection currMoveDirection = static_cast<MoveDirection>(k);
                        std::vector<std::pair<MoveDirection, MoveDirection>> perp_directions = DirectionHelper::getPerpendiculars(currMoveDirection);

                        for (std::pair<MoveDirection, MoveDirection> perp_pair : perp_directions) {

                            int perpFirstLetIndex = DirectionHelper::getDelta(perp_pair.first).first;
                            int perpFirstNumIndex = DirectionHelper::getDelta(perp_pair.first).second;

                            int perpSecondLetIndex = DirectionHelper::getDelta(perp_pair.second).first;
                            int perpSecondNumIndex = DirectionHelper::getDelta(perp_pair.second).second;

                            bool perpFirstValid = currentBoard.validPosition(i + perpFirstLetIndex, j + perpFirstNumIndex);
                            bool perpSecondValid = currentBoard.validPosition(i + perpSecondLetIndex, j + perpSecondNumIndex);

                            int potentialPerpFirstLetIndex = perpFirstLetIndex + movementLetIndex;
                            int potentialPerpFirstNumIndex = perpFirstNumIndex + movementNumIndex;

                            int potentialPerpSecondLetIndex = perpSecondLetIndex + movementLetIndex;
                            int potentialPerpSecondNumIndex = perpSecondNumIndex + movementNumIndex;

                            bool potentialPerpFirstValid = currentBoard.validPosition(i + potentialPerpFirstLetIndex, j + potentialPerpFirstNumIndex);
                            bool potentialPerpSecondValid = currentBoard.validPosition(i + potentialPerpSecondLetIndex, j + potentialPerpSecondNumIndex);
                            
                            // Check double side step with one perpendicular
                            if (perpFirstValid && board[i + perpFirstLetIndex][j + perpFirstNumIndex] == currentPlayerColour) {
                                if (potentialPerpFirstValid && board[i + potentialPerpFirstLetIndex][j + potentialPerpFirstNumIndex] == 0) {
                                    Move doubleFirstSide = Move(MoveType::SIDESTEP, static_cast<MoveDirection>(k));
                                    doubleFirstSide.addPosition(i, j);
                                    doubleFirstSide.addPosition(i + perpFirstLetIndex, j + perpFirstNumIndex);
                                    generated_moves.insert(doubleFirstSide);
                                }
                            }

                            // check double side step with the other perpendicular
                            if (perpSecondValid && board[i + perpSecondLetIndex][j + perpSecondNumIndex] == currentPlayerColour) {
                                if (potentialPerpSecondValid && board[i + potentialPerpSecondLetIndex][j + potentialPerpSecondNumIndex] == 0) {
                                    Move doubleSecondSide = Move(MoveType::SIDESTEP, static_cast<MoveDirection>(k));
                                    doubleSecondSide.addPosition(i, j);
                                    doubleSecondSide.addPosition(i + perpSecondLetIndex, j + perpSecondNumIndex);
                                    generated_moves.insert(doubleSecondSide);

                                    // check tripple side step
                                    if (perpFirstValid && board[i + perpFirstLetIndex][j + perpFirstNumIndex] == currentPlayerColour) {
                                        if (potentialPerpFirstValid && board[i + potentialPerpFirstLetIndex][j + potentialPerpFirstNumIndex] == 0) {
                                            Move trippleSide = Move(MoveType::SIDESTEP, static_cast<MoveDirection>(k));
                                            trippleSide.copyMovePositions(doubleSecondSide);
                                            trippleSide.addPosition(i + perpFirstLetIndex, j + perpFirstNumIndex);
                                            generated_moves.insert(trippleSide);
                                        }
                                    }
                                }
                            }
                        }

                        // if the trailing piece in a double inline move is the current player's piece, add a 2 piece move to results
                        bool doubleTrailingPositionValid = currentBoard.validPosition(i - movementLetIndex, j - movementNumIndex);
                        if (doubleTrailingPositionValid && board[i - movementLetIndex][j - movementNumIndex] == currentPlayerColour) {
                            Move doubleEmptyInline = Move(MoveType::INLINE, static_cast<MoveDirection>(k));
                            doubleEmptyInline.copyMovePositions(singleEmptyInline);
                            doubleEmptyInline.addPosition(i - movementLetIndex, j - movementNumIndex);
                            generated_moves.insert(doubleEmptyInline);

                            // if the trailing piece in a tripple inline move is the current player's piece, add a 3 piece move to results
                            bool tripleTrailingPositionValid = currentBoard.validPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));
                            if (tripleTrailingPositionValid && board[i - (2 * movementLetIndex)][j - (2 * movementNumIndex)] == currentPlayerColour) {
                                Move tripleEmptyInline = Move(MoveType::INLINE, static_cast<MoveDirection>(k));
                                tripleEmptyInline.copyMovePositions(doubleEmptyInline);
                                tripleEmptyInline.addPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));
                                generated_moves.insert(tripleEmptyInline);
                            }
                        }
                    }

                    //if the position in the direction of movment is an opponent's piece; a double or triple inline push can occur
                    if (potentialPositionValid && board[i + movementLetIndex][j + movementNumIndex] == opponentPlayerColour) {


                        bool potentialDoublePushPositionValid = potentialPushPositionValid(i + (2 * movementLetIndex), j + (2 * movementNumIndex), currentBoard);

                        // bool potentialDoublePushPositionValid = currentBoard.validPosition(i + (2 * movementLetIndex), j + (2 * movementNumIndex));
                        bool doubleTrailingPositionValid = currentBoard.validPosition(i - movementLetIndex, j - movementNumIndex);

                        // if the second position in the direction of movement is empty; a double and tripple push can occur
                        if (potentialDoublePushPositionValid 
                            && doubleTrailingPositionValid && board[i - movementLetIndex][j - movementNumIndex] == currentPlayerColour) {
                            Move doubleOppInline = Move(MoveType::INLINE, static_cast<MoveDirection>(k));
                            doubleOppInline.addPosition(i, j);
                            doubleOppInline.addPosition(i - movementLetIndex, j - movementNumIndex);
                            generated_moves.insert(doubleOppInline);
                            
                            bool tripleTrailingPositionValid = currentBoard.validPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));

                            if (tripleTrailingPositionValid && board[i - (2 * movementLetIndex)][j - (2 * movementNumIndex)] == currentPlayerColour) {
                                Move trippleOppInline = Move(MoveType::INLINE, static_cast<MoveDirection>(k));
                                trippleOppInline.copyMovePositions(doubleOppInline);
                                trippleOppInline.addPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));
                                generated_moves.insert(trippleOppInline);
                            }
                        }

                        bool oppDoublePushPositionValid = currentBoard.validPosition(i + (2 * movementLetIndex), j + (2 * movementNumIndex));
                        bool potentialTriplePushPositionValid = potentialPushPositionValid(i + (3 * movementLetIndex), j + (3 * movementNumIndex), currentBoard);
                        bool tripleTrailingPositionValid = currentBoard.validPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));

                        // if the second position in the direction movement is a opponent's piece, only a tripple push can occur
                        if (oppDoublePushPositionValid && board[i + (2 * movementLetIndex)][j + (2 * movementNumIndex)] == opponentPlayerColour
                            && potentialTriplePushPositionValid
                            && doubleTrailingPositionValid && board[i - movementLetIndex][j - movementNumIndex] == currentPlayerColour
                            && tripleTrailingPositionValid && board[i - (2 * movementLetIndex)][j - (2 * movementNumIndex)] == currentPlayerColour) {
                                Move tripleOppInline = Move(MoveType::INLINE, static_cast<MoveDirection>(k));
                                tripleOppInline.addPosition(i, j);
                                tripleOppInline.addPosition(i - movementLetIndex, j - movementNumIndex);
                                tripleOppInline.addPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));
                                generated_moves.insert(tripleOppInline);
                            }
                    }
                }
                if (++countCurrentPlayer > 14) {
                    return;
                }
            }
        }
    }
}

const std::set<Move>& MoveGenerator::getGeneratedMoves() const
{
    return generated_moves;
}

void MoveGenerator::printMoves() const
{
    for (const auto& move : generated_moves) {
        move.printString();
    }
}

bool MoveGenerator::potentialPushPositionValid(int i, int j, const Board &currentBoard)
{
    // the index is off the board OR the index is on the board nad -1 or 0
    const std::array<std::array<int, COLS>, ROWS>& board = currentBoard.getBoard();

    bool onboard = currentBoard.validPosition(i, j);
    bool valid = false;
    if (onboard) {
        valid = board[i][j] == -1 || board[i][j] == 0;
    }

    return (valid || !onboard);
}