10 PRINT "=================================="
20 PRINT "       TINY CALCULATOR v1.0"
30 PRINT "=================================="
40 PRINT ""
50 PRINT "Select operation:"
60 PRINT "1. Add (+)"
70 PRINT "2. Subtract (-)"
80 PRINT "3. Multiply (*)"
90 PRINT "4. Divide (/)"
100 PRINT "5. Exit"
110 PRINT ""
120 INPUT "Choice? "; C
130 IF C = 1 THEN GOTO 200
140 IF C = 2 THEN GOTO 300
150 IF C = 3 THEN GOTO 400
160 IF C = 4 THEN GOTO 500
170 IF C = 5 THEN GOTO 1000
180 PRINT "Invalid choice!"
190 GOTO 40
200 PRINT "=== ADDITION ==="
210 INPUT "First number? "; A
220 INPUT "Second number? "; B
230 R = A + B
240 PRINT ""
250 PRINT A; " + "; B; " = "; R
260 PRINT ""
270 GOTO 40
300 PRINT "=== SUBTRACTION ==="
310 INPUT "First number? "; A
320 INPUT "Second number? "; B
330 R = A - B
340 PRINT ""
350 PRINT A; " - "; B; " = "; R
360 PRINT ""
370 GOTO 40
400 PRINT "=== MULTIPLICATION ==="
410 INPUT "First number? "; A
420 INPUT "Second number? "; B
430 R = A * B
440 PRINT ""
450 PRINT A; " * "; B; " = "; R
460 PRINT ""
470 GOTO 40
500 PRINT "=== DIVISION ==="
510 INPUT "First number? "; A
520 INPUT "Second number? "; B
530 IF B = 0 THEN GOTO 550
540 R = A / B
545 PRINT ""
546 PRINT A; " / "; B; " = "; R
547 PRINT ""
548 GOTO 40
550 PRINT "Error: Cannot divide by zero!"
560 PRINT ""
570 GOTO 40
1000 PRINT ""
1010 PRINT "Thanks for using Calculator!"
1020 END
