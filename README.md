ChessBot is an ongoing project that is capable of playing chess at a high level.
The goal of the program is to compete online against other bots, be used as an analyzer in user games, and play games against a user.

Currently, the program can generate and display a game board, generate a list of possible next moves, take user inputs for moves, and display moves made.

To run: 1. If your system is able to run makefiles, simply type "make" to compile and "a" to run.
           Alternatively, copy the line in Makefile and enter it in the command line.
        2. The program will expect you to enter moves in the form of square1, square2 in lowercase. Ex. a1b2 moves a piece from A1 to B2.
        3. For promotions, include the character of the promotion piece (b,n,r,q). Ex. a7a8r promotes to a rook.
        4. 'q' will quit the program, 't' will take back the last move, and 'p' will run a perftest of the current board. A perftest examines how many variations exist in a position. They are used for validating that a chess bot works correctly.
        5. Currently, the program will not play moves for either side and will expect you to enter moves for black and white.
