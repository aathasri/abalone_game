import sys
import math
import time
from PyQt5.QtWidgets import (
    QApplication, QGraphicsView, QGraphicsScene, QGraphicsEllipseItem, QGraphicsPolygonItem,
    QGraphicsItem, QMessageBox, QLabel, QTextEdit, QFrame, QPushButton
)
from PyQt5.QtGui import QBrush, QColor, QPolygonF, QFont, QPainter
from PyQt5.QtCore import Qt, QPointF, QTimer
import abalone_cpp  # Import the C++ module

# Window + board layout constants
SCENE_WIDTH = 1080
SCENE_HEIGHT = 870
BOARD_CLUSTER_WIDTH = 680
BOARD_CLUSTER_HEIGHT = 680
TILE_SIZE = 50
H_GAP = 7.5
V_GAP = 2.5

# Game state tracking
player_timers = {"black": 0.0, "white": 0.0}
player_moves = {"black": 0, "white": 0}
player_points = {"black": 0, "white": 0}
current_turn_start = time.time()

# Board layout
BOARD_COORDS = [
    (5, 9), (6, 9), (7, 9), (8, 9), (9, 9),
    (4, 8), (5, 8), (6, 8), (7, 8), (8, 8), (9, 8),
    (3, 7), (4, 7), (5, 7), (6, 7), (7, 7), (8, 7), (9, 7),
    (2, 6), (3, 6), (4, 6), (5, 6), (6, 6), (7, 6), (8, 6), (9, 6),
    (1, 5), (2, 5), (3, 5), (4, 5), (5, 5), (6, 5), (7, 5), (8, 5), (9, 5),
    (1, 4), (2, 4), (3, 4), (4, 4), (5, 4), (6, 4), (7, 4), (8, 4),
    (1, 3), (2, 3), (3, 3), (4, 3), (5, 3), (6, 3), (7, 3),
    (1, 2), (2, 2), (3, 2), (4, 2), (5, 2), (6, 2),
    (1, 1), (2, 1), (3, 1), (4, 1), (5, 1)
]

# Initial marble positions (will be updated by backend)
WHITE_MARBLES = [
    (5, 9), (6, 9), (7, 9), (8, 9), (9, 9),
    (4, 8), (5, 8), (6, 8), (7, 8), (8, 8), (9, 8),
    (5, 7), (6, 7), (7, 7)
]

BLACK_MARBLES = [
    (1, 2), (2, 2), (3, 2), (4, 2), (5, 2), (6, 2),
    (1, 1), (2, 1), (3, 1), (4, 1), (5, 1),
    (3, 3), (4, 3), (5, 3)
]

DIRECTIONS = {
    (-1, 1): "SW",
    (0, 1): "SE",
    (1, 0): "E",
    (1, -1): "NE",
    (0, -1): "NW",
    (-1, 0): "W"
}

class SelectableTile(QGraphicsEllipseItem):
    current_player = "black"  # Start with Player 1 (black)
    selected_tiles = []
    tile_map = {}
    update_callback = None
    log_callback = None
    backend_game = abalone_cpp.AbaloneGame()  # Initialize C++ backend

    def __init__(self, col, row, x, y, color=None):
        super().__init__(0, 0, TILE_SIZE, TILE_SIZE)
        self.base_color = QColor("#6e574b")
        self.selected_color = QColor("#ffd700")
        self.setPos(x, y)
        self.setFlag(QGraphicsItem.ItemIsSelectable)
        self.setAcceptedMouseButtons(Qt.LeftButton)
        self.coord = (col, row)
        self.selected = False
        self.marble_color = color
        SelectableTile.tile_map[self.coord] = self
        self.update_color()

    def update_color(self):
        if self.selected:
            self.setBrush(QBrush(self.selected_color))
        elif self.marble_color == "black":
            self.setBrush(QBrush(QColor("#1a1a1a")))
        elif self.marble_color == "white":
            self.setBrush(QBrush(QColor("#f0f0f0")))
        else:
            self.setBrush(QBrush(self.base_color))

    def mousePressEvent(self, event):
        shift = event.modifiers() & Qt.ShiftModifier

        if not shift and self.marble_color == SelectableTile.current_player:
            for tile in SelectableTile.selected_tiles:
                tile.selected = False
                tile.update_color()
            SelectableTile.selected_tiles = [self]
            self.selected = True

        elif shift and self.marble_color == SelectableTile.current_player:
            if self.selected:
                SelectableTile.selected_tiles.remove(self)
                self.selected = False
            else:
                if len(SelectableTile.selected_tiles) < 3:
                    temp_selection = SelectableTile.selected_tiles + [self]
                    if self._is_valid_group(temp_selection):
                        SelectableTile.selected_tiles.append(self)
                        self.selected = True

        for tile in SelectableTile.tile_map.values():
            tile.update_color()
        super().mousePressEvent(event)

    def _send_to_backend(self, start_coords, direction):
        global current_turn_start
        try:
            # Send move to C++ backend
            status, black_pos, white_pos, ai_time = SelectableTile.backend_game.make_player_and_ai_move(
                start_coords, direction
            )
            if status == "INVALID":
                self._register_move(start_coords, direction_str=direction, valid=False)
                return

            # Update board state
            self._update_board(black_pos, white_pos)

            # Log player move
            move_str = f"{self._format_coords(start_coords)} > {direction}"
            duration = time.time() - current_turn_start
            player_timers[self.current_player] += duration
            player_moves[self.current_player] += 1
            if SelectableTile.log_callback:
                SelectableTile.log_callback(
                    f"[MOVE] {self.current_player.upper()}: {move_str} | Time: {duration:.2f}s"
                )

            # Log AI move (simplified, assumes AI is white)
            self.current_player = "white"
            if SelectableTile.log_callback:
                SelectableTile.log_callback(
                    f"[MOVE] WHITE (AI): Processed | Time: {ai_time:.2f}s"
                )
            player_timers["white"] += ai_time
            player_moves["white"] += 1

            # Switch back to player
            self.current_player = "black"
            current_turn_start = time.time()

            if SelectableTile.update_callback:
                SelectableTile.update_callback()
            self._clear_selection()

        except RuntimeError as e:
            QMessageBox.warning(None, "Error", f"Backend error: {str(e)}")

    def _update_board(self, black_pos, white_pos):
        # Reset all tiles
        for tile in SelectableTile.tile_map.values():
            tile.marble_color = None
            tile.update_color()

        # Update black marbles
        for col, row in black_pos:
            if (col, row) in SelectableTile.tile_map:
                SelectableTile.tile_map[(col, row)].marble_color = "black"
                SelectableTile.tile_map[(col, row)].update_color()

        # Update white marbles
        for col, row in white_pos:
            if (col, row) in SelectableTile.tile_map:
                SelectableTile.tile_map[(col, row)].marble_color = "white"
                SelectableTile.tile_map[(col, row)].update_color()

        # Update points (14 marbles initially, subtract remaining)
        player_points["black"] = 14 - len(white_pos)  # Black scores by pushing white
        player_points["white"] = 14 - len(black_pos)  # White scores by pushing black

    def _register_move(self, start_coords, direction_str=None, valid=True):
        move_str = f"{self._format_coords(start_coords)} > {direction_str}"
        if not valid:
            if SelectableTile.log_callback:
                SelectableTile.log_callback(f"[INVALID] {self.current_player.upper()} tried: {move_str}")
            QMessageBox.warning(None, "Invalid Move", "That move is invalid according to game rules.")
        if SelectableTile.update_callback:
            SelectableTile.update_callback()

    def _format_coords(self, coords):
        return ",".join([f"{chr(ord('I') - col + 1)}{row}" for col, row in coords])

    def _clear_selection(self):
        for tile in SelectableTile.selected_tiles:
            tile.selected = False
            tile.update_color()
        SelectableTile.selected_tiles.clear()

    def _is_valid_group(self, tiles):
        if len(tiles) <= 1:
            return True
        color = tiles[0].marble_color
        coords = [t.coord for t in tiles]
        if any(t.marble_color != color for t in tiles):
            return False
        coords = sorted(coords)
        if len(coords) == 2:
            dx = coords[1][0] - coords[0][0]
            dy = coords[1][1] - coords[0][1]
            return abs(dx) <= 1 and abs(dy) <= 1 and (coords[0][0] + dx, coords[0][1] + dy) == coords[1]
        elif len(coords) == 3:
            dx1 = coords[1][0] - coords[0][0]
            dy1 = coords[1][1] - coords[0][1]
            dx2 = coords[2][0] - coords[1][0]
            dy2 = coords[2][1] - coords[1][1]
            if (dx1, dy1) != (dx2, dy2):
                return False
            expected = [coords[0],
                        (coords[0][0] + dx1, coords[0][1] + dy1),
                        (coords[0][0] + 2 * dx1, coords[0][1] + 2 * dy1)]
            return coords == expected
        return False


class AbaloneBoardView(QGraphicsView):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Abalone")
        self.setFixedSize(SCENE_WIDTH, SCENE_HEIGHT)
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

        self.scene = QGraphicsScene()
        self.setSceneRect(0, 0, SCENE_WIDTH, SCENE_HEIGHT)
        self.setScene(self.scene)

        self.rows = self._group_rows(BOARD_COORDS)
        self.sorted_rows = sorted(self.rows.keys(), reverse=True)

        self._draw_board_background()
        self._draw_tiles()
        self._add_ui_elements()
        self._start_timers()

        SelectableTile.update_callback = self._update_labels
        SelectableTile.log_callback = self._append_log

    def _group_rows(self, coords):
        rows = {}
        for col, row in coords:
            rows.setdefault(row, []).append(col)
        for row in rows:
            rows[row].sort()
        return rows

    def _draw_board_background(self):
        radius = 320
        center_x = SCENE_WIDTH / 2
        center_y = SCENE_HEIGHT / 2
        hex_points = [
            QPointF(center_x + radius * math.cos(math.radians(60 * i)),
                    center_y + radius * math.sin(math.radians(60 * i)))
            for i in range(6)
        ]
        hexagon = QGraphicsPolygonItem(QPolygonF(hex_points))
        hexagon.setBrush(QBrush(QColor("#4a2c18")))
        hexagon.setZValue(-1)
        self.scene.addItem(hexagon)

    def _draw_tiles(self):
        for row in self.sorted_rows:
            cols = self.rows[row]
            count = len(cols)
            row_width = count * TILE_SIZE + (count - 1) * H_GAP
            row_x_offset = (BOARD_CLUSTER_WIDTH - row_width) / 2
            total_height = len(self.sorted_rows) * TILE_SIZE + (len(self.sorted_rows) - 1) * V_GAP
            row_y_offset = (BOARD_CLUSTER_HEIGHT - total_height) / 2
            row_index = self.sorted_rows.index(row)
            y = row_y_offset + row_index * (TILE_SIZE + V_GAP)
            offset_x = (SCENE_WIDTH - BOARD_CLUSTER_WIDTH) / 2
            offset_y = (SCENE_HEIGHT - BOARD_CLUSTER_HEIGHT) / 2
            for i, col in enumerate(cols):
                x = row_x_offset + i * (TILE_SIZE + H_GAP)
                coord = (col, row)
                color = "black" if coord in BLACK_MARBLES else "white" if coord in WHITE_MARBLES else None
                tile = SelectableTile(col, row, x + offset_x, y + offset_y, color)
                self.scene.addItem(tile)

    def _add_ui_elements(self):
        font = QFont("Arial", 11)
        self.arrows = {}

        # Turn Timer
        self.timer_label = QLabel("00.00", self)
        self.timer_label.setFont(QFont("Arial", 20, QFont.Bold))
        self.timer_label.setGeometry(470, 20, 140, 40)
        self.timer_label.setAlignment(Qt.AlignCenter)

        # Player 1 (Black)
        self.p1_box = QLabel(self)
        self.p1_box.setGeometry(20, 80, 160, 180)
        self.p1_box.setStyleSheet("background: #f0f0f0; border: 3px solid black; border-radius: 20px;")
        self.p1_box.setText("Moves: 0\nPoints: 0\n⚫ PLAYER 1\nYOUR TURN\n00.00")
        self.p1_box.setAlignment(Qt.AlignCenter)
        self.p1_box.setFont(font)

        # Player 2 (White - AI)
        self.p2_box = QLabel(self)
        self.p2_box.setGeometry(SCENE_WIDTH - 180, 80, 160, 180)
        self.p2_box.setStyleSheet("background: #f0f0f0; border: 3px solid black; border-radius: 20px;")
        self.p2_box.setText("Moves: 0\nPoints: 0\n⚪ COMP\n\n00.00")
        self.p2_box.setAlignment(Qt.AlignCenter)
        self.p2_box.setFont(font)

        # Move log
        self.log_box = QTextEdit(self)
        self.log_box.setGeometry(60, SCENE_HEIGHT - 120, 960, 100)
        self.log_box.setStyleSheet("background-color: #6e574b; color: white; border-radius: 12px; border: 4px solid #4a2c18;")
        self.log_box.setReadOnly(True)
        self.log_box.setFont(QFont("Courier", 10))

        positions = {
            "NW": (300, 200),
            "NE": (720, 200),
            "W": (200, 400),
            "E": (820, 400),
            "SW": (300, 600),
            "SE": (720, 600),
        }

        for dir_str, (x, y) in positions.items():
            btn = QPushButton(dir_str, self)
            btn.setGeometry(x, y, 60, 40)
            btn.setStyleSheet("background: yellow; font-weight: bold; border: 2px solid black; border-radius: 6px;")
            btn.clicked.connect(lambda checked, d=dir_str: self._handle_direction_click(d))
            self.arrows[dir_str] = btn

    def _append_log(self, message):
        self.log_box.append(message)

    def _start_timers(self):
        self.timer = QTimer()
        self.timer.timeout.connect(self._update_labels)
        self.timer.start(100)

    def _update_labels(self):
        elapsed = time.time() - current_turn_start
        self.timer_label.setText(f"{elapsed:.2f}")

        self.p1_box.setText(
            f"Moves: {player_moves['black']}\nPoints: {player_points['black']}\n⚫ PLAYER 1\n{'YOUR TURN' if SelectableTile.current_player == 'black' else ''}\n{player_timers['black']:.2f}"
        )
        self.p2_box.setText(
            f"Moves: {player_moves['white']}\nPoints: {player_points['white']}\n⚪ COMP\n{'YOUR TURN' if SelectableTile.current_player == 'white' else ''}\n{player_timers['white']:.2f}"
        )

    def _handle_direction_click(self, direction_str):
        if len(SelectableTile.selected_tiles) == 0:
            QMessageBox.warning(self, "No Selection", "Select 1–3 marbles first.")
            return

        start_coords = [tile.coord for tile in SelectableTile.selected_tiles]
        SelectableTile.tile_map[start_coords[0]]._send_to_backend(start_coords, direction_str)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    view = AbaloneBoardView()
    view.show()
    sys.exit(app.exec_())