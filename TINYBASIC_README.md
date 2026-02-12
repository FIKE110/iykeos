# TINYBASIC Interpreter v2.0

A complete TinyBASIC interpreter for IYKE OS, featuring structured programming,
file management, graphics, and full Turing completeness.

## Features

### Programming Constructs
- **Variables**: Integer (A-Z), String (A$-Z$)
- **Arrays**: 1D and 2D arrays with DIM statement
- **Control Flow**: FOR/NEXT, WHILE/WEND, IF/ELSE/ELSEIF/ENDIF
- **Subroutines**: GOSUB/RETURN
- **Functions**: Built-in math and string functions
- **Line Numbers**: Traditional BASIC line numbering

### Data Types
- **Integers**: 32-bit signed integers
- **Strings**: Up to 256 characters
- **Arrays**: Multi-dimensional arrays up to 1000 elements

### Built-in Functions

#### Math Functions
- `RND(n)` - Random number (0 to n-1)
- `ABS(x)` - Absolute value
- `SQR(x)` - Square root
- `PEEK(addr)` - Read memory address

#### String Functions
- `LEN(s$)` - String length
- `ASC(s$)` - ASCII value of first character
- `VAL(s$)` - Convert string to number
- `STR$(n)` - Convert number to string
- `CHR$(n)` - Character from ASCII code
- `LEFT$(s$, n)` - Left n characters
- `RIGHT$(s$, n)` - Right n characters
- `MID$(s$, start, len)` - Substring
- `INSTR(haystack$, needle$)` - Find substring position

### Graphics Commands
- `SCREEN mode` - Set VGA mode
- `PSET x, y, color` - Draw pixel
- `LINE x1, y1, x2, y2, color` - Draw line
- `CIRCLE xc, yc, r, color` - Draw circle
- `CLS` - Clear screen
- `COLOR c` - Set text color
- `LOCATE row, col` - Position cursor

### File Management
- `LOAD "filename"` - Load program
- `SAVE "filename"` - Save program
- `FILES` - List files
- `KILL "filename"` - Delete file

### Other Commands
- `PRINT` - Output text
- `INPUT "prompt"; var` - Get user input
- `LET var = expr` - Assignment
- `GOTO line` - Jump to line
- `GOSUB line` - Call subroutine
- `RETURN` - Return from subroutine
- `FOR var = start TO end [STEP n]` - Loop
- `NEXT var` - End loop
- `WHILE expr` - Conditional loop
- `WEND` - End while
- `IF expr THEN ... [ELSEIF ...] [ELSE ...] ENDIF` - Conditionals
- `DIM name(size)` - Declare array
- `DIM name(size1, size2)` - Declare 2D array
- `READ var` - Read from DATA
- `DATA values...` - Data statements
- `RESTORE` - Reset DATA pointer
- `REM` or `'` - Comment
- `BEEP freq, duration` - Sound
- `DELAY ms` - Pause
- `POKE addr, value` - Write memory
- `RUN` - Start program
- `END` or `STOP` - End program
- `LIST` - List program
- `NEW` - Clear program

## Usage Examples

### Hello World
```basic
10 PRINT "Hello, World!"
20 END
```

### Variables and Math
```basic
10 A = 10
20 B = 20
30 C = A + B * 2
40 PRINT "Result: "; C
50 END
```

### For Loop
```basic
10 FOR I = 1 TO 10
20 PRINT "Count: "; I
30 NEXT I
40 END
```

### While Loop
```basic
10 X = 10
20 WHILE X > 0
30 PRINT X
40 X = X - 1
50 WEND
60 END
```

### Arrays
```basic
10 DIM ARR(10)
20 FOR I = 0 TO 10
30 ARR(I) = I * I
40 NEXT I
50 FOR I = 0 TO 10
60 PRINT "ARR("; I; ") = "; ARR(I)
70 NEXT I
80 END
```

### Graphics
```basic
10 SCREEN 19
20 FOR I = 0 TO 100
30 PSET I, I, 15
40 NEXT I
50 CIRCLE 50, 50, 20, 12
60 END
```

### String Manipulation
```basic
10 A$ = "Hello"
20 B$ = "World"
30 C$ = A$ + " " + B$
40 PRINT C$
50 PRINT "Length: "; LEN(C$)
60 PRINT "First 5 chars: "; LEFT$(C$, 5)
70 END
```

### File Operations
```basic
10 PRINT "Saving program..."
20 SAVE "MYPROG.BAS"
30 PRINT "Loading program..."
40 LOAD "MYPROG.BAS"
50 LIST
60 END
```

### Subroutines
```basic
10 PRINT "Main program"
20 GOSUB 100
30 PRINT "Back in main"
40 END
100 PRINT "In subroutine"
110 RETURN
```

### Conditional Statements
```basic
10 X = 15
20 IF X > 10 THEN
30   PRINT "X is greater than 10"
40 ELSEIF X = 10 THEN
50   PRINT "X equals 10"
60 ELSE
70   PRINT "X is less than 10"
80 ENDIF
90 END
```

## Running Programs

1. Create a BASIC program using the EDITOR program or copy DEMO.BAS
2. Save the file with a .BAS extension
3. Run the TINYBASIC program
4. The interpreter will automatically load and execute the program from the text buffer

## Notes

- Line numbers are required and should be in ascending order
- Variables are case-insensitive
- String concatenation uses the + operator
- Comments start with REM or '
- Multiple statements per line are not supported
- Maximum 500 lines per program
- Maximum 120 characters per line
