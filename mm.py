
import sys
import subprocess
import os
import random
import time

class Engine:
    def __init__(self, path, movetime, debug):
        self.path = path
        self.p = subprocess.Popen(path, stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        self.movetime = movetime
        self.debug = debug

    def send_message(self, message):
        self.p.stdin.write(message + "\n")
        self.p.stdin.flush()

    def bestmove(self):
        self.p.stdin.write("go movetime " + self.movetime + "\n")
        self.p.stdin.flush()
        response = self.p.stdout.readline()
        response = response.strip()
        with open(self.debug, 'a') as log:
            log.write("got " + response + " from " + self.path + "\n")
        response = response.split()
        return response[1]

        #return self.p.stdout.readline().strip().split()[1]

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

e1 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\engines\\" + sys.argv[1], sys.argv[3], "e1debug.txt")
e2 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\engines\\" + sys.argv[2], sys.argv[4], "e2debug.txt")

draws = 0
e1wins = 0
e2wins = 0

with open("C:\\Users\\14244\\Desktop\\chess\\mm\\fen\\lc01k.txt", 'r') as fens:
    fenlist = fens.readlines()

random.shuffle(fenlist)

for i in range(len(fenlist)):
    fen = fenlist[i]
    e1.send_message("ucinewgame")
    e2.send_message("ucinewgame")
    e1.send_message("position fen " + fen)
    e2.send_message("position fen " + fen)
    while True:
        os.system('cls')
        print(fen, end='')
        e1.print_board()
        print(sys.argv[1] + ": " + str(e1wins))
        print(sys.argv[2] + ": " + str(e2wins))
        print("draws: " + str(draws) + "\ngame: " + str(i+1) + "/" + str(len(fenlist)))
        sys.stdout.flush()
          
        state = e1.state()
        if state == "draw":
            draws += 1
            os.system('cls')
            print(fen, end='')
            e1.print_board()
            print(sys.argv[1] + ": " + str(e1wins))
            print(sys.argv[2] + ": " + str(e2wins))
            print("draws: " + str(draws) + "\ngame: " + str(i+1) + "/" + str(len(fenlist)))
            sys.stdout.flush()
            break
        elif state == "mate":
            e2wins += 1
            os.system('cls')
            print(fen, end='')
            e1.print_board()
            print(sys.argv[1] + ": " + str(e1wins))
            print(sys.argv[2] + ": " + str(e2wins))
            print("draws: " + str(draws) + "\ngame: " + str(i+1) + "/" + str(len(fenlist)))
            sys.stdout.flush()
            break

        move = e1.bestmove()
        e1.send_message("position current moves " + move)
        e2.send_message("position current moves " + move)

        os.system('cls')
        print(fen, end='')
        e1.print_board()
        print(sys.argv[1] + ": " + str(e1wins))
        print(sys.argv[2] + ": " + str(e2wins))
        print("draws: " + str(draws) + "\ngame: " + str(i+1) + "/" + str(len(fenlist)))
        sys.stdout.flush()
            
        state = e2.state()
        if state == "draw":
            draws += 1
            os.system('cls')
            print(fen, end='')
            e1.print_board()
            print(sys.argv[1] + ": " + str(e1wins))
            print(sys.argv[2] + ": " + str(e2wins))
            print("draws: " + str(draws) + "\ngame: " + str(i+1) + "/" + str(len(fenlist)))
            sys.stdout.flush()
            break
        elif state == "mate":
            e1wins += 1
            os.system('cls')
            print(fen, end='')
            e1.print_board()
            print(sys.argv[1] + ": " + str(e1wins))
            print(sys.argv[2] + ": " + str(e2wins))
            print("draws: " + str(draws) + "\ngame: " + str(i+1) + "/" + str(len(fenlist)))
            sys.stdout.flush()
            break

        move = e2.bestmove()
        e1.send_message("position current moves " + move)
        e2.send_message("position current moves " + move)

e1.send_message("quit")
e2.send_message("quit")

