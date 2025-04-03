#include "move_generator.h"

void MoveGenerator::generateMoves(int currentPlayerColour, const Board& currentBoard)
{
    const auto& board = currentBoard.getBoard();
    int opponentPlayerColour = 3 - currentPlayerColour;
    int countCurrentPlayer = 0;

    for (int i = ROWS - 1; i > 0; i--) {
        for (int j = 0; j < COLS; j++) {
            if (!currentBoard.validPosition(i, j) || board[i][j] != currentPlayerColour) continue;

            for (int k = 0; k < static_cast<int>(MoveDirection::COUNT); k++) {
                int dx = DirectionHelper::getDelta(k).first;
                int dy = DirectionHelper::getDelta(k).second;

                int nx = i + dx;
                int ny = j + dy;
                if (!currentBoard.validPosition(nx, ny)) continue;

                if (board[nx][ny] == 0) {
                    Move singleMove(MoveType::INLINE, static_cast<MoveDirection>(k));
                    singleMove.addPosition(i, j);
                    generated_moves.insert(singleMove);

                    for (const auto& [p1, p2] : DirectionHelper::getPerpendiculars(static_cast<MoveDirection>(k))) {
                        int dx1 = DirectionHelper::getDelta(p1).first;
                        int dy1 = DirectionHelper::getDelta(p1).second;
                        int dx2 = DirectionHelper::getDelta(p2).first;
                        int dy2 = DirectionHelper::getDelta(p2).second;

                        int x1 = i + dx1, y1 = j + dy1;
                        int x2 = i + dx2, y2 = j + dy2;

                        int nx1 = x1 + dx, ny1 = y1 + dy;
                        int nx2 = x2 + dx, ny2 = y2 + dy;

                        if (currentBoard.validPosition(x1, y1) && board[x1][y1] == currentPlayerColour &&
                            currentBoard.validPosition(nx1, ny1) && board[nx1][ny1] == 0) {
                            Move doubleSide(MoveType::SIDESTEP, static_cast<MoveDirection>(k));
                            doubleSide.addPosition(i, j);
                            doubleSide.addPosition(x1, y1);
                            generated_moves.insert(doubleSide);
                        }

                        if (currentBoard.validPosition(x2, y2) && board[x2][y2] == currentPlayerColour &&
                            currentBoard.validPosition(nx2, ny2) && board[nx2][ny2] == 0) {
                            Move doubleSide(MoveType::SIDESTEP, static_cast<MoveDirection>(k));
                            doubleSide.addPosition(i, j);
                            doubleSide.addPosition(x2, y2);
                            generated_moves.insert(doubleSide);

                            if (currentBoard.validPosition(x1, y1) && board[x1][y1] == currentPlayerColour &&
                                currentBoard.validPosition(nx1, ny1) && board[nx1][ny1] == 0) {
                                Move tripleSide(MoveType::SIDESTEP, static_cast<MoveDirection>(k));
                                tripleSide.addPosition(i, j);
                                tripleSide.addPosition(x1, y1);
                                tripleSide.addPosition(x2, y2);
                                generated_moves.insert(tripleSide);
                            }
                        }
                    }

                    int tx = i - dx;
                    int ty = j - dy;
                    if (currentBoard.validPosition(tx, ty) && board[tx][ty] == currentPlayerColour) {
                        Move doubleInline(MoveType::INLINE, static_cast<MoveDirection>(k));
                        doubleInline.addPosition(i, j);
                        doubleInline.addPosition(tx, ty);
                        generated_moves.insert(doubleInline);

                        int ttx = i - 2 * dx;
                        int tty = j - 2 * dy;
                        if (currentBoard.validPosition(ttx, tty) && board[ttx][tty] == currentPlayerColour) {
                            Move tripleInline(MoveType::INLINE, static_cast<MoveDirection>(k));
                            tripleInline.addPosition(i, j);
                            tripleInline.addPosition(tx, ty);
                            tripleInline.addPosition(ttx, tty);
                            generated_moves.insert(tripleInline);
                        }
                    }
                }

                if (board[nx][ny] == opponentPlayerColour) {
                    int opp2x = i + 2 * dx;
                    int opp2y = j + 2 * dy;
                    bool secondValid = currentBoard.validPosition(opp2x, opp2y);
                    bool secondEmpty = secondValid && board[opp2x][opp2y] == 0;

                    int tx = i - dx;
                    int ty = j - dy;
                    if (secondEmpty && currentBoard.validPosition(tx, ty) && board[tx][ty] == currentPlayerColour) {
                        Move doublePush(MoveType::INLINE, static_cast<MoveDirection>(k));
                        doublePush.addPosition(i, j);
                        doublePush.addPosition(tx, ty);
                        generated_moves.insert(doublePush);

                        int ttx = i - 2 * dx;
                        int tty = j - 2 * dy;
                        if (currentBoard.validPosition(ttx, tty) && board[ttx][tty] == currentPlayerColour) {
                            Move triplePush(MoveType::INLINE, static_cast<MoveDirection>(k));
                            triplePush.addPosition(i, j);
                            triplePush.addPosition(tx, ty);
                            triplePush.addPosition(ttx, tty);
                            generated_moves.insert(triplePush);
                        }
                    }

                    if (secondValid && board[opp2x][opp2y] == opponentPlayerColour) {
                        int opp3x = i + 3 * dx;
                        int opp3y = j + 3 * dy;
                        bool thirdValid = potentialPushPositionValid(opp3x, opp3y, currentBoard);
                        int tx2 = i - dx;
                        int ty2 = j - dy;
                        int tx3 = i - 2 * dx;
                        int ty3 = j - 2 * dy;

                        if (thirdValid && currentBoard.validPosition(tx2, ty2) && board[tx2][ty2] == currentPlayerColour &&
                            currentBoard.validPosition(tx3, ty3) && board[tx3][ty3] == currentPlayerColour) {
                            Move triplePush(MoveType::INLINE, static_cast<MoveDirection>(k));
                            triplePush.addPosition(i, j);
                            triplePush.addPosition(tx2, ty2);
                            triplePush.addPosition(tx3, ty3);
                            generated_moves.insert(triplePush);
                        }
                    }
                }
            }

            if (++countCurrentPlayer > 14) return;
        }
    }
}

const std::set<Move>& MoveGenerator::getGeneratedMoves() const {
    return generated_moves;
}

void MoveGenerator::printMoves() const {
    for (const auto& move : generated_moves) {
        move.printString();
    }
}

bool MoveGenerator::potentialPushPositionValid(int i, int j, const Board& currentBoard) {
    const auto& board = currentBoard.getBoard();
    bool onboard = currentBoard.validPosition(i, j);
    bool valid = false;
    if (onboard) {
        valid = board[i][j] == -1 || board[i][j] == 0;
    }
    return (valid || !onboard);
}
