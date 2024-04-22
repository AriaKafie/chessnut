
import sys
import subprocess
import os
import random

class Engine:
    def __init__(self, path, movetime):
        self.p = subprocess.Popen(path, stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        self.movetime = movetime

    def send_message(self, message):
        self.p.stdin.write(message + "\n")
        self.p.stdin.flush()

    def bestmove(self):
        self.p.stdin.write("go movetime " + self.movetime + "\n")
        self.p.stdin.flush()
        return self.p.stdout.readline().strip().split()[1]

    def state(self):
        self.p.stdin.write("gameinfo\n")
        self.p.stdin.flush()
        return self.p.stdout.readline().strip()

    def print_board(self):
        self.p.stdin.write("board\n")
        self.p.stdin.flush()
        board = self.p.stdout.readline().strip()
        print(board.replace("?", "\n"), end='')
        sys.stdout.flush()

e1 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\bin\\" + sys.argv[1], sys.argv[3])
e2 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\bin\\" + sys.argv[2], sys.argv[4])

draws = 0
e1wins = 0
e2wins = 0

with open('c:\\users\\14244\\desktop\\chess\\mm\\fen\\equal_fens.txt', 'r') as fens:
    fenlist = fens.readlines()

random.shuffle(fenlist)

for fen in fenlist:
    e1.send_message("ucinewgame")
    e2.send_message("ucinewgame")
    startfen = fen
    e1.send_message("position fen " + fen)
    e2.send_message("position fen " + fen)
    while True:
        os.system('cls')
        print(startfen, end='')
        e1.print_board()
        print(sys.argv[1] + ": " + str(e1wins))
        print(sys.argv[2] + ": " + str(e2wins))
        print("draws: " + str(draws))
        sys.stdout.flush()
          
        state = e1.state()
        if state == "draw":
            draws += 1
            break
        elif state == "mate":
            e2wins += 1
            break

        move = e1.bestmove()
        e1.send_message("position current moves " + move)
        e2.send_message("position current moves " + move)

        os.system('cls')
        print(startfen, end='')
        e1.print_board()
        print(sys.argv[1] + ": " + str(e1wins))
        print(sys.argv[2] + ": " + str(e2wins))
        print("draws: " + str(draws))
        sys.stdout.flush()
            
        state = e1.state()
        if state == "draw":
            draws += 1
            break
        elif state == "mate":
            e1wins += 1
            break

        move = e2.bestmove()
        e1.send_message("position current moves " + move)
        e2.send_message("position current moves " + move)

e1.send_message("quit")
e2.send_message("quit")

