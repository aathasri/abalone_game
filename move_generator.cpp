#include "move_generator.h"

std::set<Move> MoveGenerator::generateMoves(int currentPlayerColour, Board currentBoard)
{
    std::set<Move> result; // define max size of result to make this process much quicker

    std::array<std::array<int, COLS>, ROWS>& board = currentBoard.getBoard();

    int opponentPlayerColour = 3 - currentPlayerColour;

    int countCurrentPlayer = 0;

    

    // iterate through pieces on the board
    for (int i = ROWS - 1; i > 0; i--) {
        for (int j = 0; j < COLS; j++) {

            

            if (board[i][j] == currentPlayerColour) {

                

                // iterate through directions
                for (int k = 0; k < static_cast<int>(MoveDirection::COUNT); k++) {

                    // get movement indexes
                    int movementLetIndex = directionArray[k].first;
                    int movementNumIndex = directionArray[k].second;

                    bool potentialPositionValid = currentBoard.validPosition(i + movementLetIndex,j + movementNumIndex);
                                        
                    //if the position in the direction of movment is empty, a single can occur; a double or triple might be possible
                    if (potentialPositionValid && board[i + movementLetIndex][j+ movementNumIndex] == 0) {
                        Move singleEmptyInline = Move(static_cast<MoveDirection>(k));
                        singleEmptyInline.addPosition(i, j);
                        result.insert(singleEmptyInline);



                        // CHECK FOR SIDESTEP
                        MoveDirection currMoveDirection = static_cast<MoveDirection>(k);
                        std::vector<std::pair<MoveDirection, MoveDirection>> perp_directions = MoveDirectionHelper::getPerpendiculars(currMoveDirection);
                        





                        // CHECK FOR INLINE

                        // if the trailing piece in a double inline move is the current player's piece, add a 2 piece move to results
                        bool doubleTrailingPositionValid = currentBoard.validPosition(i - movementLetIndex, j - movementNumIndex);
                        if (doubleTrailingPositionValid && board[i - movementLetIndex][j - movementNumIndex] == currentPlayerColour) {
                            Move doubleEmptyInline = Move(static_cast<MoveDirection>(k));
                            doubleEmptyInline.copyMovePositions(singleEmptyInline);
                            doubleEmptyInline.addPosition(i - movementLetIndex, j - movementNumIndex);
                            result.insert(doubleEmptyInline);

                            // if the trailing piece in a tripple inline move is the current player's piece, add a 3 piece move to results
                            bool tripleTrailingPositionValid = currentBoard.validPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));
                            if (tripleTrailingPositionValid && board[i - (2 * movementLetIndex)][j - (2 * movementNumIndex)] == currentPlayerColour) {
                                Move tripleEmptyInline = Move(static_cast<MoveDirection>(k));
                                tripleEmptyInline.copyMovePositions(doubleEmptyInline);
                                tripleEmptyInline.addPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));
                                result.insert(tripleEmptyInline);
                            }
                        }

                    }

                    //if the position in the direction of movment is an opponent's piece; a double or triple inline push can occur
                    if (potentialPositionValid && board[movementLetIndex][movementNumIndex] == opponentPlayerColour) {

                        bool potentialDoublePushPositionValid = board[i + (2 * movementLetIndex)][j + (2 * movementNumIndex)];
                        bool doubleTrailingPositionValid = currentBoard.validPosition(i - movementLetIndex, j - movementNumIndex);

                        // if the second position in the direction of movement is empty; a double and tripple push can occur
                        if (potentialDoublePushPositionValid && board[i + (2 * movementLetIndex)][j + (2 * movementNumIndex)] == 0 
                            && doubleTrailingPositionValid && board[i - movementLetIndex][j - movementNumIndex] == currentPlayerColour) {
                            Move doubleOppInline = Move(static_cast<MoveDirection>(k));
                            doubleOppInline.addPosition(i, j);
                            doubleOppInline.addPosition(i - movementLetIndex, j - movementNumIndex);
                            result.insert(doubleOppInline);
                            
                            bool tripleTrailingPositionValid = currentBoard.validPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));

                            if (tripleTrailingPositionValid && board[i - (2 * movementLetIndex)][j - (2 * movementNumIndex)] == currentPlayerColour) {
                                Move trippleOppInline = Move(static_cast<MoveDirection>(k));
                                trippleOppInline.copyMovePositions(doubleOppInline);
                                trippleOppInline.addPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));
                                result.insert(trippleOppInline);
                            }
                        }

                        bool potentialTriplePushPositionValid = currentBoard.validPosition(i + (3 * movementLetIndex), j + (3 * movementNumIndex));
                        bool tripleTrailingPositionValid = currentBoard.validPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));

                        // if the second position in the direction movement is a opponent's piece, only a tripple push can occur
                        if (potentialDoublePushPositionValid && board[i + (2 * movementLetIndex)][j + (2 * movementNumIndex)] == opponentPlayerColour
                            && potentialTriplePushPositionValid && board[i + (3 * movementLetIndex)][j + (3 * movementNumIndex)] == 0
                            && doubleTrailingPositionValid && board[i - movementLetIndex][j - movementNumIndex] == currentPlayerColour
                            && tripleTrailingPositionValid && board[i - (2 * movementLetIndex)][j - (2 * movementNumIndex)] == currentPlayerColour) {
                                Move tripleOppInline = Move(static_cast<MoveDirection>(k));
                                tripleOppInline.addPosition(i, j);
                                tripleOppInline.addPosition(i - movementLetIndex, j - movementNumIndex);
                                tripleOppInline.addPosition(i - (2 * movementLetIndex), j - (2 * movementNumIndex));
                                result.insert(tripleOppInline);
                            }
                    }
                }
                if (++countCurrentPlayer > 14) {
                    goto foundAllCurrrentPlayerPieces;
                }
            }
        }
    }
    foundAllCurrrentPlayerPieces:
    return result;
}

void MoveGenerator::printMoves(const std::set<Move>& m)
{
    for (const auto& move : m) {
        move.printString();
    }
}