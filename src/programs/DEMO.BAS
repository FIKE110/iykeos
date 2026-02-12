10 REM ========================================
20 REM TINYBASIC Demo Program
30 REM Demonstrates all major features
40 REM ========================================
50 CLS
60 PRINT "========================================"
70 PRINT "      TINYBASIC INTERPRETER v2.0"
80 PRINT "========================================"
90 PRINT ""
100 PRINT "Features: Variables, Arrays, Loops"
110 PRINT "          Functions, Graphics, Files"
120 PRINT "========================================"
130 PRINT ""
140 PRINT "Press any key to continue..."
150 REM Wait for key
160 FOR I = 1 TO 10000
170 NEXT I
180 CLS
190 
200 REM ========================================
210 REM Variable Demonstration
220 REM ========================================
230 PRINT "=== VARIABLE DEMONSTRATION ==="
240 PRINT ""
250 A = 10
260 B = 20
270 C = A + B
280 PRINT "A = "; A
290 PRINT "B = "; B
300 PRINT "A + B = "; C
310 PRINT ""
320 D$ = "Hello"
330 E$ = "World"
340 F$ = D$ + " " + E$
350 PRINT D$; " + "; E$; " = "; F$
360 PRINT ""
370 PRINT "Press any key to continue..."
380 FOR I = 1 TO 10000
390 NEXT I
400 CLS
410 
420 REM ========================================
430 REM Loop Demonstration
440 REM ========================================
450 PRINT "=== LOOP DEMONSTRATION ==="
460 PRINT ""
470 PRINT "FOR loop counting 1 to 5:"
480 FOR I = 1 TO 5
490 PRINT "  Count: "; I
500 NEXT I
510 PRINT ""
520 PRINT "WHILE loop:"
530 X = 5
540 WHILE X > 0
550 PRINT "  Countdown: "; X
560 X = X - 1
570 WEND
580 PRINT ""
590 PRINT "Nested loops (multiplication table):"
600 FOR I = 1 TO 3
610 FOR J = 1 TO 3
620 P = I * J
630 PRINT "  "; I; "x"; J; "="; P
640 NEXT J
650 NEXT I
660 PRINT ""
670 PRINT "Press any key to continue..."
680 FOR I = 1 TO 10000
690 NEXT I
700 CLS
710 
720 REM ========================================
730 REM Array Demonstration
740 REM ========================================
750 PRINT "=== ARRAY DEMONSTRATION ==="
760 PRINT ""
770 DIM ARR(10)
780 PRINT "Filling array with squares:"
790 FOR I = 0 TO 10
800 ARR(I) = I * I
810 NEXT I
820 PRINT "Array contents:"
830 FOR I = 0 TO 10
840 PRINT "  ARR("; I; ") = "; ARR(I)
850 NEXT I
860 PRINT ""
870 PRINT "Two-dimensional array (3x3):"
880 DIM GRID(3, 3)
890 FOR I = 0 TO 2
900 FOR J = 0 TO 2
910 GRID(I, J) = I * 3 + J
920 NEXT J
930 NEXT I
940 FOR I = 0 TO 2
950 PRINT "  Row "; I; ": ";
960 FOR J = 0 TO 2
970 PRINT GRID(I, J); " ";
980 NEXT J
990 PRINT ""
1000 NEXT I
1010 PRINT ""
1020 PRINT "Press any key to continue..."
1030 FOR I = 1 TO 10000
1040 NEXT I
1050 CLS
1060 
1070 REM ========================================
1080 REM Conditional Demonstration
1090 REM ========================================
1100 PRINT "=== CONDITIONAL DEMONSTRATION ==="
1110 PRINT ""
1120 PRINT "IF-ELSE-ENDIF examples:"
1130 X = 15
1140 PRINT "X = "; X
1150 IF X > 10 THEN
1160 PRINT "  X is greater than 10"
1170 ELSE
1180 PRINT "  X is 10 or less"
1190 ENDIF
1200 PRINT ""
1210 Y = 5
1220 PRINT "Y = "; Y
1230 IF Y < 0 THEN
1240 PRINT "  Y is negative"
1250 ELSEIF Y = 0 THEN
1260 PRINT "  Y is zero"
1270 ELSE
1280 PRINT "  Y is positive"
1290 ENDIF
1300 PRINT ""
1310 PRINT "Press any key to continue..."
1320 FOR I = 1 TO 10000
1330 NEXT I
1340 CLS
1350 
1360 REM ========================================
1370 REM Math Functions Demonstration
1380 REM ========================================
1390 PRINT "=== MATH FUNCTIONS ==="
1400 PRINT ""
1410 PRINT "Random numbers:"
1420 FOR I = 1 TO 5
1430 R = RND(100)
1440 PRINT "  Random (1-100): "; R
1450 NEXT I
1460 PRINT ""
1470 PRINT "Other functions:"
1480 PRINT "  ABS(-15) = "; ABS(-15)
1490 PRINT "  SQR(16) = "; SQR(16)
1500 PRINT "  2 + 3 * 4 = "; 2 + 3 * 4
1510 PRINT "  (2 + 3) * 4 = "; (2 + 3) * 4
1520 PRINT ""
1530 PRINT "Press any key to continue..."
1540 FOR I = 1 TO 10000
1550 NEXT I
1560 CLS
1570 
1580 REM ========================================
1590 REM String Functions Demonstration
1600 REM ========================================
1610 PRINT "=== STRING FUNCTIONS ==="
1620 PRINT ""
1630 T$ = "TINYBASIC"
1640 PRINT "Original string: "; T$
1650 L = LEN(T$)
1660 PRINT "  Length: "; L
1670 PRINT "  First char (ASC): "; ASC(T$)
1680 PRINT "  Left 5 chars: "; LEFT$(T$, 5)
1690 PRINT "  Right 4 chars: "; RIGHT$(T$, 4)
1700 PRINT "  Middle (3,4): "; MID$(T$, 3, 4)
1710 PRINT ""
1720 N = 42
1730 S$ = STR$(N)
1740 PRINT "Number "; N; " as string: "; S$
1750 PRINT ""
1760 PRINT "Press any key to continue..."
1770 FOR I = 1 TO 10000
1780 NEXT I
1790 CLS
1800 
1810 REM ========================================
1820 REM Graphics Demonstration
1830 REM ========================================
1840 PRINT "=== GRAPHICS DEMONSTRATION ==="
1850 PRINT ""
1860 PRINT "Switching to graphics mode..."
1870 SCREEN 19
1880 FOR I = 1 TO 5000
1890 NEXT I
1900 
1910 REM Draw some pixels
1920 FOR I = 0 TO 100
1930 C = I MOD 16
1940 PSET I, I, C
1950 PSET 100 - I, I, C
1960 NEXT I
1970 
1980 REM Draw lines
1990 LINE 10, 50, 90, 50, 15
2000 LINE 50, 10, 50, 90, 15
2010 
2020 REM Draw circles
2030 CIRCLE 50, 50, 20, 12
2040 CIRCLE 50, 50, 30, 11
2050 
2060 PRINT "Graphics drawn!"
2070 PRINT ""
2080 FOR I = 1 TO 30000
2090 NEXT I
2100 CLS
2110 
2120 REM ========================================
2130 REM GOSUB Demonstration
2140 REM ========================================
2150 PRINT "=== SUBROUTINE DEMONSTRATION ==="
2160 PRINT ""
2170 PRINT "Calling subroutines with GOSUB..."
2180 GOSUB 5000
2190 PRINT "Back from subroutine 1"
2200 PRINT ""
2210 X = 7
2220 GOSUB 5100
2230 PRINT "Factorial of "; X; " is "; F
2240 PRINT ""
2250 PRINT "Press any key to continue..."
2260 FOR I = 1 TO 10000
2270 NEXT I
2280 CLS
2290 GOTO 6000
2300 
2310 REM ========================================
2320 REM DATA/READ Demonstration
2330 REM ========================================
2340 PRINT "=== DATA/READ DEMONSTRATION ==="
2350 PRINT ""
2360 PRINT "Reading from DATA statements:"
2370 FOR I = 1 TO 5
2380 READ D
2390 PRINT "  Read value: "; D
2400 NEXT I
2410 PRINT ""
2420 PRINT "Restoring and reading again:"
2430 RESTORE
2440 FOR I = 1 TO 3
2450 READ D
2460 PRINT "  Read value: "; D
2470 NEXT I
2480 PRINT ""
2490 GOTO 7000
2500 
2510 REM ========================================
2520 REM DATA Section
2530 REM ========================================
2540 DATA 10, 20, 30, 40, 50
2550 
2560 REM ========================================
2570 REM Subroutines
2580 REM ========================================
5000 REM Subroutine 1 - Print message
5010 PRINT "  This is subroutine 1"
5020 RETURN
5100 REM Subroutine 2 - Calculate factorial
5110 F = 1
5120 FOR J = 2 TO X
5130 F = F * J
5140 NEXT J
5150 RETURN
6000 REM Continue after GOSUB demo
6010 PRINT "=== END OF DEMONSTRATION ==="
6020 PRINT ""
6030 PRINT "TINYBASIC v2.0 Demo Complete!"
6040 PRINT ""
6050 PRINT "This interpreter supports:"
6060 PRINT "  - Integer variables (A-Z)"
6070 PRINT "  - String variables (A$-Z$)"
6080 PRINT "  - Arrays with DIM"
6090 PRINT "  - FOR/NEXT loops"
6100 PRINT "  - WHILE/WEND loops"
6110 PRINT "  - IF/ELSE/ELSEIF/ENDIF"
6120 PRINT "  - GOSUB/RETURN"
6130 PRINT "  - Graphics commands"
6140 PRINT "  - File operations"
6150 PRINT "  - String functions"
6160 PRINT "  - Math functions"
6170 PRINT ""
6180 PRINT "Full Turing Complete!"
6190 END
7000 REM Final section
7010 PRINT "Press any key to exit..."
7020 FOR I = 1 TO 20000
7030 NEXT I
7040 GOTO 6000
