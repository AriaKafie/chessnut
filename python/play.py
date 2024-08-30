
import chess
import chess.engine
import time
import sys
import subprocess
import threading
import pyautogui

pixels = []

x = 1950
y = 1900

pixels.append((x, y))

for i in range(1, 64):
    x -= 195
    if i % 8 == 0:
        x = 1950
        y -= 195
    pixels.append((x, y))
    
class Engine:
    
    def __init__(self, path):
        
        self.p = subprocess.Popen(path, stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

    def await_stop(self):
        
        cmd = input()
        self.p.stdin.write("stop\n")
        self.p.stdin.flush()
        
    def send_message(self, message):

        self.p.stdin.write("d\n")
        self.p.stdin.flush()

        line = ""

        while line.find("Fen") == -1:
            line = self.p.stdout.readline().strip()

        fen = line[5:]

        line = self.p.stdout.readline()
        line = self.p.stdout.readline()

        board = chess.Board(fen)

        legal_moves = [move.uci() for move in board.legal_moves]

        if message.strip() in legal_moves:

            self.send_message("moves " + message)
            self.go()

            return
        
        self.p.stdin.write(message + "\n")
        self.p.stdin.flush()

    def print_board(self):
        
        self.p.stdin.write("d\n")
        self.p.stdin.flush()

        line = ""

        while line.find("Key") == -1:
            line = self.p.stdout.readline().replace("\n", "")
            print(line)

        print(self.p.stdout.readline().replace("\n", ""))
        
    def go(self):

        self.p.stdin.write("d\n")
        self.p.stdin.flush()

        line = ""

        while "Fen" not in line:
            line = self.p.stdout.readline().strip()

        color = line.split()[2]
        
        line = self.p.stdout.readline()
        line = self.p.stdout.readline()

        input_thread = threading.Thread(target=self.await_stop)
        input_thread.start()
                
        self.p.stdin.write("go\n")
        self.p.stdin.flush()
        
        while True:
            response = self.p.stdout.readline().strip()
            print(response)
            sys.stdout.flush()
            if response.split()[0] == "bestmove":
                break

        bestmove = response.split()[1]

        self.send_message("moves " + bestmove)

        from_str = bestmove[:2]
        to_str   = bestmove[2:4]

        from_idx = 8 * (ord(from_str[1]) - ord('1')) + (ord('h') - ord(from_str[0]))
        to_idx   = 8 * (ord(to_str  [1]) - ord('1')) + (ord('h') - ord(to_str  [0]))

        if color == 'b':
            from_idx ^= 63
            to_idx   ^= 63
        
        xfrom, yfrom = pixels[from_idx]
        xto, yto = pixels[to_idx]

        pyautogui.click(xfrom, yfrom)
        pyautogui.click(xto, yto)

        if "q" in bestmove:
            pyautogui.click(xto, yto)
        
engine = Engine("c:\\users\\14244\\source\\repos\\chess\\x64\\release\\endl.exe")

cmd = ''

while cmd != "quit":
    
    cmd = input()

    if cmd == "d":
        engine.print_board()
    elif cmd == "go":
        engine.go()
    else:
        engine.send_message(cmd)

engine.send_message("quit")
    
