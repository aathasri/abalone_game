import sys
import math
from PyQt5.QtWidgets import QApplication, QGraphicsView, QGraphicsScene, QGraphicsEllipseItem, QGraphicsPolygonItem, QGraphicsItem
from PyQt5.QtGui import QBrush, QColor, QPolygonF
from PyQt5.QtCore import Qt, QPointF

TILE_SIZE = 50
H_GAP = 7.5
V_GAP = 2.5
SCENE_SIZE = 680
BOARD_CLUSTER_SIZE = 550

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

class SelectableTile(QGraphicsEllipseItem):
    current_player = "black"
    selected_tiles = []
    tile_map = {}  # Maps coordinates to tile instances

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
        if self.marble_color != SelectableTile.current_player:
            return

        shift = event.modifiers() & Qt.ShiftModifier

        if not shift:
            for tile in SelectableTile.selected_tiles:
                tile.selected = False
                tile.update_color()
            SelectableTile.selected_tiles = [self]
            self.selected = True
        else:
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

    def _is_valid_group(self, tiles):
        if len(tiles) <= 1:
            return True

        color = tiles[0].marble_color
        coords = [t.coord for t in tiles]

        if any(t.marble_color != color for t in tiles):
            return False

        coords = sorted(coords)

        # Check collinearity and adjacency (same direction, no gaps)
        if len(coords) == 2:
            dx = coords[1][0] - coords[0][0]
            dy = coords[1][1] - coords[0][1]
            if abs(dx) > 1 or abs(dy) > 1:
                return False
            return (coords[0][0] + dx, coords[0][1] + dy) == coords[1]

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
        self.setFixedSize(SCENE_SIZE, SCENE_SIZE)

        self.scene = QGraphicsScene()
        self.scene.setSceneRect(0, 0, SCENE_SIZE, SCENE_SIZE)
        self.setScene(self.scene)

        self.rows = self._group_rows(BOARD_COORDS)
        self.sorted_rows = sorted(self.rows.keys(), reverse=True)

        self._draw_board_background()
        self._draw_tiles()

    def _group_rows(self, coords):
        rows = {}
        for col, row in coords:
            rows.setdefault(row, []).append(col)
        for row in rows:
            rows[row].sort()
        return rows

    def _draw_board_background(self):
        radius = 320
        center_x = SCENE_SIZE / 2
        center_y = SCENE_SIZE / 2
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
            row_x_offset = (BOARD_CLUSTER_SIZE - row_width) / 2

            number_of_rows = len(self.sorted_rows)
            total_height = number_of_rows * TILE_SIZE + (number_of_rows - 1) * V_GAP
            row_y_offset = (BOARD_CLUSTER_SIZE - total_height) / 2
            row_index = self.sorted_rows.index(row)
            y = row_y_offset + row_index * (TILE_SIZE + V_GAP)

            overall_offset = (SCENE_SIZE - BOARD_CLUSTER_SIZE) / 2
            for i, col in enumerate(cols):
                x = row_x_offset + i * (TILE_SIZE + H_GAP)
                coord = (col, row)
                color = None
                if coord in BLACK_MARBLES:
                    color = "black"
                elif coord in WHITE_MARBLES:
                    color = "white"
                tile = SelectableTile(col, row, x + overall_offset, y + overall_offset, color)
                self.scene.addItem(tile)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    view = AbaloneBoardView()
    view.show()
    sys.exit(app.exec_())
