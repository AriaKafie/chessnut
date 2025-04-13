
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

    def debug(self):

        self.p.stdin.write("debug\n")
        self.p.stdin.flush()

        while True:
            line = self.p.stdout.readline().strip()
            if line:
                print(line)
            else:
                break
        
    def fen(self):

        self.p.stdin.write("fen\n")
        self.p.stdin.flush()

        fen = self.p.stdout.readline().strip()

        return fen
        
    def await_stop(self):
        
        cmd = input()
        self.p.stdin.write("stop\n")
        self.p.stdin.flush()
        
    def send_message(self, message):

        board = chess.Board(self.fen())

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

        color = self.fen().split()[1]

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
        
engine = Engine("c:\\users\\14244\\source\\repos\\search_root\\x64\\release\\search_root.exe")

while True:
    
    cmd = input()
    if cmd:
        if cmd == "quit":
            break
        elif cmd == "debug":
            engine.debug()
        elif cmd == "d":
            engine.print_board()
        elif cmd == "go":
            engine.go()
        else:
            engine.send_message(cmd)

engine.send_message("quit")
    
