import subprocess
from copy import deepcopy
from collections import deque
import heapq
from othello_py import *


n_parallel = 8

ai_exe = [subprocess.Popen('./../src/egaroucid5.out'.split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL) for _ in range(n_parallel)]

book = {}
try:
    with open('learned_data/book.txt') as f:
        data = f.read().splitlines()
    for datum in data:
        board, val = datum.split()
        val = float(val)
        book[board] = [val, False]
except:
    print('no book found')
val_threshold = 2
move_threshold = 0

que = []

def start_calc_value(o, ai_num):
    grid_str = ''
    for i in range(hw):
        for j in range(hw):
            grid_str += '0' if o.grid[i][j] == 0 else '1' if o.grid[i][j] == 1 else '.'
        grid_str += '\n'
    if grid_str.replace('\n', '') in book:
        return False
    ai_exe[ai_num].stdin.write((str(o.player) + '\n' + grid_str).encode('utf-8'))
    ai_exe[ai_num].stdin.flush()
    return True

def calc_value(o, ai_num, flag):
    if sum(o.n_stones) > 30:
        return
    grid_str = ''
    for i in range(hw):
        for j in range(hw):
            grid_str += '0' if o.grid[i][j] == 0 else '1' if o.grid[i][j] == 1 else '.'
        grid_str += '\n'
    if flag:
        o.print_info()
        line = ai_exe[ai_num].stdout.readline()
        print(line)
        _, _, val = [float(elem) for elem in line.decode().split()]
        if o.player == white:
            val = -val
        print(val)
        book[grid_str.replace('\n', '')] = [val, True]
        with open('learned_data/book.txt', 'a') as f:
            f.write(grid_str.replace('\n', '') + ' ' + str(o.player) + ' ' + str(round(val)) + '\n')
    else:
        val, flag = book[grid_str.replace('\n', '')]
        if flag:
            return
    if sum(o.n_stones) > move_threshold + 4 and abs(val) > abs(val_threshold):
        print('out of range')
        return
    for i in range(hw):
        for j in range(hw):
            if o.grid[i][j] == legal:
                next_o = deepcopy(o)
                next_o.move(i, j)
                if not next_o.check_legal():
                    next_o.player = 1 - next_o.player
                    if not next_o.check_legal():
                        continue
                heapq.heappush(que, (abs(val), next_o))

o = othello()
o.check_legal()
o.move(4, 5)
o.check_legal()
heapq.heappush(que, (0, o))
while que:
    print('                                      ', len(que))
    lst = []
    num = min(n_parallel, len(que))
    for i in range(num):
        val, tmp = heapq.heappop(que)
        print(val)
        lst.append([tmp, start_calc_value(tmp, i)])
    for i in range(num):
        calc_value(lst[i][0], i, lst[i][1])

for i in range(n_parallel):
    ai_exe[i].kill()