# Abalone

## Team members
- [Danton Soares](https://github.com/Danton1)
- [Aathavan Sriharan](https://github.com/aathasri)
- [Nick Thucydides](https://github.com/nick-thucydides)
- [Thomas Smith](https://github.com/Tromulus)


## Summary

<div align="center">
   <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/5/57/Abalone_standard.svg/1920px-Abalone_standard.svg.png" alt="Abalone Board" width="250"/>
</div>

Abalone is a two-player strategy board game that takes place on a hexagonal board. The objective is to push six of the opponent's marbles off the board. Players can move one, two, or three marbles in a straight line in any direction. The marbles can push the opponent's marbles if they have more marbles in the line of movement. The game ends when one player pushes six of the opponent's marbles off the board.

This project is a C++ implementation of the Abalone board game. The program reads an input file containing the board state and generates all possible moves for a given player. The output includes the moves and the board state after each move.


## File Structure
- `movegen.cpp`: Contains the implementation of the `MoveGen` class, which generates all possible moves for a given player.
- `boardgen.cpp`: Contains the implementation of the `BoardGen` class, which generates the board state after each move.
- `main.cpp`: Contains the main function that reads the input file, generates moves, and outputs the results.

```plaintext
.
├── src
│   ├── movegen.cpp
│   ├── boardgen.cpp
│   └── main.cpp
├── Test#.input
├── boardgen.exe
├── main.exe
├── movegen.exe
└── README.md
```

---

## Guide to Compile and Execute
### 1. Prerequisites
- C++ compiler (e.g., g++, clang++).
- Input files named Test#.input (where # represents a test case number) are in the same directory as the output file.

---

### 2. Compiling the Program
To compile the source files, open a terminal (Linux/macOS) or Command Prompt (Windows) and navigate to the directory containing the source files.

Make sure to compile all three source files: `movegen.cpp`, `boardgen.cpp`, and `main.cpp`.

   #### For Linux/macOS:
   ```bash
   g++ movegen.cpp -o movegen
   g++ boardgen.cpp -o boardgen
   g++ main.cpp -o main
   ```

   #### For Windows (using MinGW):
   ```bash
   g++ movegen.cpp -o movegen.exe
   g++ boardgen.cpp -o boardgen.exe
   g++ main.cpp -o main.exe
   ```

   In order to run the program in a computer without a C++ compiler, compile the source files with the flags `-static -static-libgcc -static-libstdc++` to statically link the required libraries and mitigate errors related to missing DLLs.

   #### For Linux/macOS:
   ```bash
   g++ movegen.cpp -o movegen -static -static-libgcc -static-libstdc++
   g++ boardgen.cpp -o boardgen -static -static-libgcc -static-libstdc++
   g++ main.cpp -o main -static -static-libgcc -static-libstdc++
   ```

   #### For Windows (using MinGW):
   ```bash
   g++ movegen.cpp -o movegen.exe -static -static-libgcc -static-libstdc++
   g++ boardgen.cpp -o boardgen.exe -static -static-libgcc -static-libstdc++
   g++ main.cpp -o main.exe -static -static-libgcc -static-libstdc++
   ```

### Explanation:
- `g++`: Invokes the GNU C++ compiler.
- `movegen.cpp`: Source file to compile.
- `-o movegen`: Outputs an executable named `movegen` (or `movegen.exe` on Windows).
- `-static -static-libgcc -static-libstdc++`: Flags to statically link the required libraries and mitigate errors related to missing DLLs.

If the requirements are met, all the executables (`movegen`, `boardgen`, and `main` or their `.exe` counterparts on Windows) will be generated.

---

### 3. Running the Program
If an argument is not passed to the program, it will take in Test1,input as the default input file.

Ensure the following:
   1. The required input file (e.g., `Test1.input`, `Test2.input`, etc.) is present in the same directory.
   2. All three files (`movegen`, `boardgen`, and `main`) are compiled and located in the same folder.

#### Example Execution:
To run the program with `Test1.input`:

**On Linux/macOS:**
```bash
./main Test1.input
```
**On Windows**
```bash
main.exe Test1.input
```

`Note`: The Windows executables are included in the submission folder and can be run without recompiling using the command:
```bash
main.exe Test<#>.input
```
Or simply by double-clicking the executable if you want to use the default input file (`Test1.input`).

---

### 4. Interpreting Output
The program processes the input file and outputs two files: 
1. `Test<#>.moves`: Contains all possible moves for the selected player.
2. `Test<#>.boards`: Contains the board state after each move.

---

### 5. Debugging and Logs
If any errors occur during compilation or execution:
1. Ensure the `Test<#>.input` file is properly formatted and present in the same directory as the executables.
2. Verify that all three executables (`movegen`, `boardgen`, and `main`) are compiled and located in the same folder.
3. Only run the `main` executable, passing the desired input file as a command-line argument.

If the program is not behaving as expected:
- Double-check the input file for formatting issues.
- Recompile the source files to ensure no compilation errors occurred.
- Use debugging tools (e.g., `gdb` for Linux/macOS or a debugger in your IDE) to trace any runtime issues.

---

If you encounter further issues, feel free to reach out to the team members listed at the beginning of this document.