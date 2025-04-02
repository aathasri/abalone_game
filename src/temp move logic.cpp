
// void Board::moveE(std::vector<std::string> &pieces)
// {
//     if (pieces.size() == 1) {
//         moveOnePiece(pieces);
//     }
//     if (isHorizintal(pieces)) {
//         movePiecesInline(pieces, move);
//     } else {
//         movePieceSideStep(pieces, move);
//     }
// }

// void Board::moveNE(std::vector<std::string>& pieces)
// {
//     std::string move;

//     if (pieces.size() == 1) {
//         move = "i - " + pieces[0].substr(0,2) + " - NE";
//         moveOnePiece(move);
//     }

//     sortMovePieces(pieces);
//     if (isForwardSloping(pieces)) {
//         move = "i - " + pieces[0].substr(0,2) + " - NE";
//         movePiecesInline(pieces, move);
//     } else {
//         move = "s - " + pieces[0].substr(0,2) + " - " + pieces[pieces.size() - 1].substr(0,2) + " - NE";
//         movePieceSideStep(pieces, move);
//     }
// }

// void Board::moveNW(std::vector<std::string>& pieces)
// {
//     std::string move;

//     if (pieces.size() == 1) {
//         move = "i - " + pieces[0].substr(0,2) + " - NW";
//         moveOnePiece(move);
//     }

//     sortMovePieces(pieces);
//     if (isBackwardSloping(pieces)) {
//         move = "i - " + pieces[0].substr(0,2) + " - NW";
//         movePiecesInline(pieces, move);
//     } else {
//         move = "s - " + pieces[0].substr(0,2) + " - " + pieces[pieces.size() - 1].substr(0,2) + " - NW";
//         movePieceSideStep(pieces, move);
//     }
// }

// void Board::moveW(std::vector<std::string>& pieces)
// {
//     std::string move;

//     if (pieces.size() == 1) {
//         move = "i - " + pieces[0].substr(0,2) + " - W";
//         moveOnePiece(move);
//     }

//     sortMovePieces(pieces);
//     if (isHorizintal(pieces)) {
//         move = "i - " + pieces[0].substr(0,2) + " - W";
//         movePiecesInline(pieces, move);
//     } else {
//         move = "s - " + pieces[0].substr(0,2) + " - " + pieces[pieces.size() - 1].substr(0,2) + " - W";
//         movePieceSideStep(pieces, move);
//     }
// }

// void Board::moveSW(std::vector<std::string>& pieces)
// {
//     std::string move;

//     if (pieces.size() == 1) {
//         move = "i - " + pieces[0].substr(0,2) + " - SW";
//         moveOnePiece(move);
//     }

//     sortMovePieces(pieces);
//     if (isForwardSloping(pieces)) {
//         move = "i - " + pieces[0].substr(0,2) + " - SW";
//         movePiecesInline(pieces, move);
//     } else {
//         move = "s - " + pieces[0].substr(0,2) + " - " + pieces[pieces.size() - 1].substr(0,2) + " - SW";
//         movePieceSideStep(pieces, move);
//     }
// }

// void Board::moveSE(std::vector<std::string>& pieces)
// {
//     std::string move;

//     if (pieces.size() == 1) {
//         move = "i - " + pieces[0].substr(0,2) + " - SE";
//         moveOnePiece(move);
//     }

//     sortMovePieces(pieces);
//     if (isBackwardSloping(pieces)) {
//         move = "i - " + pieces[0].substr(0,2) + " - SE";
//         movePiecesInline(pieces, move);
//     } else {
//         move = "s - " + pieces[0].substr(0,2) + " - " + pieces[pieces.size() - 1].substr(0,2) + " - SE";
//         movePieceSideStep(pieces, move);
//     }
// }

// /**
//  * Ensures pieces are horizontal and continuous
//  * Letter index is consistent, number index increments
//  */
// bool Board::isHorizintal(const std::vector<std::string>& pieces)
// {
//     if (pieces.size() == 2) {
//         return pieces[0][0] == pieces[1][0] && pieces[0][1] + 1 == pieces[1][1];
//     } 
//     if (pieces.size() == 3) {
//         return (pieces[0][0] == pieces[1][0] && pieces[0][0] == pieces[2][0]) 
//         && (pieces[0][1] == pieces[1][1] + 1 && pieces[0][1] == pieces[2][1] + 2);
//     }
// }

// /**
//  * Ensures pieces are backward sloping (\) and continuous
//  * Letter index increments, number index is consistent
//  */
// bool Board::isBackwardSloping(const std::vector<std::string>& pieces)
// {
//     if (pieces.size() == 2) {
//         return pieces[0][0] + 1 == pieces[1][0] && pieces[0][1] == pieces[1][1];
//     } 
//     if (pieces.size() == 3) {
//         return (pieces[0][0] + 1 == pieces[1][0] && pieces[0][0] + 2 == pieces[2][0]) 
//         && (pieces[0][1] == pieces[1][1] && pieces[0][1] == pieces[2][1]);
//     }
// }

// /**
//  * Ensures pieces are forward sloping (/) and continuous
//  * Letter index increments, number index increments
//  */
// bool Board::isForwardSloping(const std::vector<std::string>& pieces)
// {
//     if (pieces.size() == 2) {
//         return pieces[0][0] + 1 == pieces[1][0] && pieces[0][1] + 1 == pieces[1][1];
//     } 
//     if (pieces.size() == 3) {
//         return (pieces[0][0] + 1 == pieces[1][0] && pieces[0][0] + 2 == pieces[2][0]) 
//         && (pieces[0][1] + 1 == pieces[1][1] && pieces[0][1] + 2 == pieces[2][1]);
//     }
// }

// void Board::moveOnePiece(const std::string &move)
// {
//     std::pair<int, int> moveMapping = moveMap.at(move.substr(9));
//     int pieceLetterIndex = 'I' - move[4];
//     int pieceNumberIndex = move[5] - '1';
//     int newPositionLetterIndex = pieceLetterIndex + moveMapping.first;
//     int newPositionNumberIndex = pieceNumberIndex + moveMapping.second;
//     if (validPosition(newPositionLetterIndex, newPositionNumberIndex) && mat[newPositionLetterIndex][newPositionNumberIndex] == '0') {
//         mat[newPositionLetterIndex][newPositionNumberIndex] = mat[pieceLetterIndex][pieceNumberIndex];
//         mat[pieceLetterIndex][pieceNumberIndex] = '0';
//     }
// }

// void Board::movePiecesInline(const std::vector<std::string>& pieces, const std::string& move)
// {
//     //generate pieces from range
//     //if there are 2 pieces
//         //check if spot in direction of movement.
//         // if its empty you can move no problem
//         // if its the piece of the color moving, invalid move
//         // if its the piece of the opposite colour, check a piece 1 more to the right
//             // if that is empty you can move, if not it is an invalid move

//     // if there are 3 pieces
//         // do everything you did for 2 piece but at the end if it's 
    
//     int size = pieces.size();

//     char moveColour = pieces[0][2];
//     std::string direction = move.substr(9);
//     std::pair<int, int> moveMapping = moveMap.at(direction);

//     std::string mainPiece;
//     // if moving E, NE, SE i want the piece at the end of the list
//     if (direction == "E" || direction == "NE" || direction == "SE") {
//         mainPiece = pieces[size - 1];
//     }
//     // if im move W, NW, or SW i want the piece at the start of the list
//     if (direction == "W" || direction == "NW" || direction == "SW") {
//         mainPiece = pieces[0];
//     }

//     int pieceLetterIndex = 'I' - mainPiece[0];
//     int pieceNumberIndex = mainPiece[1] - '1';

//     // if the space in the direction of movement is empty, move the line regardless of size then return

//     if (mat[pieceLetterIndex + moveMapping.first][pieceNumberIndex + moveMapping.second] == '0') {
        

//     }

//     // if size is 2 or more and the color to the right is opponent check 1 more to the right, if empty or outofbounds, sent it

//     if (size == 2) {
        
        
//     }

//     // if size is 3 and the colour to the right is op and to the right of that is op, if the one after is empty of out of bounds, send it

//     if (size == 3) {

//     }

//     // TODO: all other cases return error because cant move
// }

// void Board::movePieceSideStep(const std::vector<std::string>& pieces, const std::string& move)
// {
//     std::pair<int, int> moveMapping = moveMap.at(move.substr(14));

//     bool moveValidated = true;

//     for (const std::string& piece : pieces) {
//         int pieceLetterIndex = 'I' - piece[0];
//         int pieceNumberIndex = piece[1] - '1';
//         int newPositionLetterIndex = pieceLetterIndex + moveMapping.first;
//         int newPositionNumberIndex = pieceNumberIndex + moveMapping.second;
//         if (!validPosition(newPositionLetterIndex, newPositionNumberIndex) || mat[newPositionLetterIndex][newPositionNumberIndex] != '0') {
//             moveValidated = false;
//             break;
//         }
//     }

//     if (moveValidated) {
//         for (const std::string& piece : pieces) {
//             int pieceLetterIndex = 'I' - piece[0];
//             int pieceNumberIndex = piece[1] - '1';
//             int newPositionLetterIndex = pieceLetterIndex + moveMapping.first;
//             int newPositionNumberIndex = pieceNumberIndex + moveMapping.second;
//             mat[newPositionLetterIndex][newPositionNumberIndex] = mat[pieceLetterIndex][pieceNumberIndex];
//             mat[pieceLetterIndex][pieceNumberIndex] = '0';
//         }
//     }
// }