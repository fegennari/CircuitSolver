# Circuit - mult_ic.nl

INPUT(1)
INPUT(2)
INPUT(3)
INPUT(4)
INPUT(5)
INPUT(6)
OUTPUT(25)
OUTPUT(25)
OUTPUT(26)
OUTPUT(27)
OUTPUT(22)
OUTPUT(13)
15 = XOR(6 5)
24 = XOR(5 4)
IC1 n0 n7 n8 9 6 6 1 0  M
IC2 9 10 11 12 6 6 2 0  M
IC3 12 13 14 0 6 6 3 0  M
IC4 n1 16 17 18 5 15 8 7  M
IC5 18 19 20 21 5 15 11 7  M
IC6 21 22 23 0 5 15 14 10  M
IC7 n2 25 n3 28 4 24 17 16  M
IC8 28 26 n4 29 4 24 20 16  M
IC9 29 27 n5 0 4 24 23 19  M

