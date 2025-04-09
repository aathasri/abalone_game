import sys
import math
# Import necessary PyQt5 widgets, GUI elements, and core classes
from PyQt5.QtWidgets import QApplication, QGraphicsView, QGraphicsScene, QGraphicsEllipseItem, QGraphicsPolygonItem, QGraphicsItem
from PyQt5.QtGui import QBrush, QColor, QPolygonF
from PyQt5.QtCore import Qt, QPointF

# Define constants for tile size, horizontal and vertical gaps between tiles, and overall scene/board sizes
TILE_SIZE = 50          # Diameter of each tile (marble circle)
H_GAP = 7.5             # Horizontal gap between tiles
V_GAP = 2.5             # Vertical gap between tiles
SCENE_SIZE = 680        # Total size of the QGraphicsScene (width and height)
BOARD_CLUSTER_SIZE = 550  # Size of the area where the board tiles are drawn

# Define the board coordinates that represent valid positions for the game board tiles
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

# Define starting positions for white marbles
WHITE_MARBLES = [
    (5, 9), (6, 9), (7, 9), (8, 9), (9, 9),
    (4, 8), (5, 8), (6, 8), (7, 8), (8, 8), (9, 8),
    (5, 7), (6, 7), (7, 7)
]

# Define starting positions for black marbles
BLACK_MARBLES = [
    (1, 2), (2, 2), (3, 2), (4, 2), (5, 2), (6, 2),
    (1, 1), (2, 1), (3, 1), (4, 1), (5, 1),
    (3, 3), (4, 3), (5, 3)
]

# Define a custom QGraphicsEllipseItem that represents a selectable tile (marble)
class SelectableTile(QGraphicsEllipseItem):
    # Class variables for keeping track of current player's turn and selected tiles
    current_player = "black"   # Indicates which player's turn it is
    selected_tiles = []        # List to store currently selected tiles
    tile_map = {}              # Dictionary mapping board coordinates to tile instances

    def __init__(self, col, row, x, y, color=None):
        # Initialize the ellipse with a fixed size (TILE_SIZE x TILE_SIZE)
        super().__init__(0, 0, TILE_SIZE, TILE_SIZE)
        # Base color for an unselected tile without a marble
        self.base_color = QColor("#6e574b")
        # Color to indicate a selected tile (highlight)
        self.selected_color = QColor("#ffd700")
        # Position the tile in the scene
        self.setPos(x, y)
        # Enable item selection functionality
        self.setFlag(QGraphicsItem.ItemIsSelectable)
        # Accept only left mouse button clicks for selection
        self.setAcceptedMouseButtons(Qt.LeftButton)
        # Store the board coordinate (col, row)
        self.coord = (col, row)
        # State to track whether this tile is selected
        self.selected = False
        # The marble color on this tile (if any), can be "black", "white", or None
        self.marble_color = color
        # Add this tile instance to the global tile_map for easy lookup
        SelectableTile.tile_map[self.coord] = self
        # Update the display color based on the current state
        self.update_color()

    def update_color(self):
        """
        Update the brush color of the tile based on its selection and marble state.
        """
        if self.selected:
            # If selected, use the highlight color
            self.setBrush(QBrush(self.selected_color))
        elif self.marble_color == "black":
            # If a black marble is present, use black color
            self.setBrush(QBrush(QColor("#1a1a1a")))
        elif self.marble_color == "white":
            # If a white marble is present, use white color
            self.setBrush(QBrush(QColor("#f0f0f0")))
        else:
            # Otherwise, use the base tile color
            self.setBrush(QBrush(self.base_color))

    def mousePressEvent(self, event):
        """
        Handle mouse press events to select/deselect tiles.
        """
        # Only allow selection if the tile has a marble of the current player's color
        if self.marble_color != SelectableTile.current_player:
            return

        # Check if the Shift key is pressed to allow multiple selection
        shift = event.modifiers() & Qt.ShiftModifier

        if not shift:
            # Without Shift, clear all previous selections and select only this tile
            for tile in SelectableTile.selected_tiles:
                tile.selected = False
                tile.update_color()
            SelectableTile.selected_tiles = [self]
            self.selected = True
        else:
            # With Shift, toggle the selection of this tile if already selected
            if self.selected:
                SelectableTile.selected_tiles.remove(self)
                self.selected = False
            else:
                # Allow selection of up to 3 tiles at once
                if len(SelectableTile.selected_tiles) < 3:
                    # Check if adding this tile to current selection forms a valid group
                    temp_selection = SelectableTile.selected_tiles + [self]
                    if self._is_valid_group(temp_selection):
                        SelectableTile.selected_tiles.append(self)
                        self.selected = True

        # Update the colors of all tiles based on the new selection state
        for tile in SelectableTile.tile_map.values():
            tile.update_color()
        # Call the base class mousePressEvent to ensure normal processing
        super().mousePressEvent(event)

    def _is_valid_group(self, tiles):
        """
        Check if a given group of selected tiles forms a valid group (collinear and adjacent)
        according to game rules.
        """
        # A group of one tile is always valid
        if len(tiles) <= 1:
            return True

        # All tiles in the group must have the same marble color
        color = tiles[0].marble_color
        coords = [t.coord for t in tiles]

        if any(t.marble_color != color for t in tiles):
            return False

        # Sort coordinates for easier comparison
        coords = sorted(coords)

        # For two selected tiles, check that they are adjacent (difference is at most 1 and consecutive)
        if len(coords) == 2:
            dx = coords[1][0] - coords[0][0]
            dy = coords[1][1] - coords[0][1]
            if abs(dx) > 1 or abs(dy) > 1:
                return False
            return (coords[0][0] + dx, coords[0][1] + dy) == coords[1]

        # For three selected tiles, check collinearity and that they are in direct sequence without gaps
        elif len(coords) == 3:
            dx1 = coords[1][0] - coords[0][0]
            dy1 = coords[1][1] - coords[0][1]
            dx2 = coords[2][0] - coords[1][0]
            dy2 = coords[2][1] - coords[1][1]

            # They must all lie in the same direction
            if (dx1, dy1) != (dx2, dy2):
                return False

            # Build the expected sequence of coordinates starting from the first tile
            expected = [coords[0],
                        (coords[0][0] + dx1, coords[0][1] + dy1),
                        (coords[0][0] + 2 * dx1, coords[0][1] + 2 * dy1)]
            return coords == expected

        # Return False for any group sizes that are not explicitly handled (should not happen)
        return False

# Define the main game board view that displays the board and manages tile layout
class AbaloneBoardView(QGraphicsView):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Abalone")  # Set window title
        self.setFixedSize(SCENE_SIZE, SCENE_SIZE)  # Set fixed window size

        # Create a QGraphicsScene to hold all board items and set its dimensions
        self.scene = QGraphicsScene()
        self.scene.setSceneRect(0, 0, SCENE_SIZE, SCENE_SIZE)
        self.setScene(self.scene)

        # Group the board coordinates by row to help with drawing
        self.rows = self._group_rows(BOARD_COORDS)
        # Sort the rows in reverse order so the highest row is drawn first
        self.sorted_rows = sorted(self.rows.keys(), reverse=True)

        # Draw the board background (hexagon shape) and the game tiles
        self._draw_board_background()
        self._draw_tiles()

    def _group_rows(self, coords):
        """
        Group the board coordinates by their row value.
        Returns a dictionary mapping each row to a list of columns.
        """
        rows = {}
        for col, row in coords:
            # Add the column to the list for this row
            rows.setdefault(row, []).append(col)
        for row in rows:
            # Sort the columns for each row
            rows[row].sort()
        return rows

    def _draw_board_background(self):
        """
        Draw a hexagon background for the board.
        """
        # Set a large radius for the hexagon shape
        radius = 320
        # Calculate the center of the scene
        center_x = SCENE_SIZE / 2
        center_y = SCENE_SIZE / 2
        # Create 6 points for the hexagon based on polar coordinates
        hex_points = [
            QPointF(center_x + radius * math.cos(math.radians(60 * i)),
                    center_y + radius * math.sin(math.radians(60 * i)))
            for i in range(6)
        ]
        # Create a QGraphicsPolygonItem from the hexagon points
        hexagon = QGraphicsPolygonItem(QPolygonF(hex_points))
        # Set the brush color for the hexagon background
        hexagon.setBrush(QBrush(QColor("#4a2c18")))
        # Set the hexagon behind other items by giving it a lower Z-value
        hexagon.setZValue(-1)
        # Add the hexagon to the scene
        self.scene.addItem(hexagon)

    def _draw_tiles(self):
        """
        Draw each tile (marble) on the board according to their coordinates.
        """
        # Iterate over each row in the sorted order
        for row in self.sorted_rows:
            # Retrieve the columns in the current row
            cols = self.rows[row]
            count = len(cols)
            # Calculate the width of the row including gaps between tiles
            row_width = count * TILE_SIZE + (count - 1) * H_GAP
            # Center the row horizontally within the board cluster area
            row_x_offset = (BOARD_CLUSTER_SIZE - row_width) / 2

            # Calculate total number of rows and overall height of the board cluster
            number_of_rows = len(self.sorted_rows)
            total_height = number_of_rows * TILE_SIZE + (number_of_rows - 1) * V_GAP
            # Center the row vertically within the board cluster area
            row_y_offset = (BOARD_CLUSTER_SIZE - total_height) / 2
            # Determine the vertical position of the current row based on its index
            row_index = self.sorted_rows.index(row)
            y = row_y_offset + row_index * (TILE_SIZE + V_GAP)

            # Calculate an overall offset to center the board cluster within the scene
            overall_offset = (SCENE_SIZE - BOARD_CLUSTER_SIZE) / 2
            # Iterate over each column in the current row
            for i, col in enumerate(cols):
                # Calculate the horizontal position for this tile
                x = row_x_offset + i * (TILE_SIZE + H_GAP)
                # Build the coordinate tuple for the tile
                coord = (col, row)
                color = None
                # Determine if this coordinate should have a black marble
                if coord in BLACK_MARBLES:
                    color = "black"
                # Determine if this coordinate should have a white marble
                elif coord in WHITE_MARBLES:
                    color = "white"
                # Create the tile with the computed position and color, applying the overall offset
                tile = SelectableTile(col, row, x + overall_offset, y + overall_offset, color)
                # Add the tile to the scene so it appears on the board
                self.scene.addItem(tile)

# Standard Python idiom to run the application
if __name__ == '__main__':
    # Create the QApplication instance (handles the event loop)
    app = QApplication(sys.argv)
    # Create an instance of the board view
    view = AbaloneBoardView()
    # Show the view window
    view.show()
    # Execute the application event loop and exit cleanly on close
    sys.exit(app.exec_())
