
import sys
import subprocess
import os
import random
import time
from datetime import datetime

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
        response = self.p.stdout.readline().strip()
        if response == "":
            return "crash"
        else:
            return response.split()[1]
        
    def state(self):
        self.p.stdin.write("gameinfo\n")
        self.p.stdin.flush()
        return self.p.stdout.readline().strip()

    def print_board(self):
        self.p.stdin.write("board\n")
        self.p.stdin.flush()
        board = self.p.stdout.readline().strip()
        print(board.replace("?", "\n"), end='')

e1 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\engines\\" + sys.argv[1], sys.argv[3])
e2 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\engines\\" + sys.argv[2], sys.argv[4])

draws = 0
e1wins = 0
e2wins = 0
e1crash = 0
e2crash = 0

def refresh(engine, fen):
    os.system('cls')
    print(fen, end='')
    engine.print_board()
    text = sys.argv[1] + ": " + str(e1wins)
    text = f"{text:<10}"
    print(text + "crashes: " + str(e1crash))
    text = sys.argv[2] + ": " + str(e2wins)
    text = f"{text:<10}"
    print(text + "crashes: " + str(e2crash))
    text = "draws: " + str(draws)
    text = f"{text:<10}" 
    print(text + "game: " + str(i+1) + "/" + str(len(fenlist)))
    sys.stdout.flush()
    
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
        
        refresh(e1, fen)
          
        state = e1.state()
        if state == "draw":
            draws += 1
            refresh(e1, fen)
            break
        elif state == "mate":
            e2wins += 1
            refresh(e1, fen)
            break

        move = e1.bestmove()
        if move == "crash":
            e1crash += 1
            draws += 1
            with open('debug.txt', 'a') as log:
                log.write("engine1(" + sys.argv[1] + ") crashed with " + fen + " at " + datetime.now().strftime('%H:%M:%S') + "\n")
            e2.send_message("quit")
            time.sleep(5)
            e1 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\engines\\" + sys.argv[1], sys.argv[3])
            e2 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\engines\\" + sys.argv[2], sys.argv[4])
            time.sleep(5)
            break
            
        e1.send_message("position current moves " + move)
        e2.send_message("position current moves " + move)

        refresh(e1, fen)
            
        state = e2.state()
        if state == "draw":
            draws += 1
            refresh(e1, fen)
            break
        elif state == "mate":
            e1wins += 1
            refresh(e1, fen)
            break

        move = e2.bestmove()
        if move == "crash":
            e2crash += 1
            draws += 1
            with open('debug.txt', 'a') as log:
                log.write("engine2(" + sys.argv[2] + ") crashed with " + fen + " at " + datetime.now().strftime('%H:%M:%S') + "\n")
            e1.send_message("quit")
            time.sleep(5)
            e1 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\engines\\" + sys.argv[1], sys.argv[3])
            e2 = Engine("c:\\users\\14244\\desktop\\chess\\mm\\engines\\" + sys.argv[2], sys.argv[4])
            time.sleep(5)
            break
        e1.send_message("position current moves " + move)
        e2.send_message("position current moves " + move)

e1.send_message("quit")
e2.send_message("quit")
