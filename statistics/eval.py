import subprocess
import glob
from othello_py import *
from copy import deepcopy
from tqdm import tqdm

drs = [
    'data/records15/'
]

out_drs = [
    'data/records15_1_50_with_eval/'
]

def score_to_string(score):
    s = (score + 64) // 2
    return chr(ord('!') + s)

MAX_DEPTH = 15

egaroucid = subprocess.Popen(('egaroucid6_depth_eval.exe').split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)

for dr, out_dr in zip(drs, out_drs):
    files = glob.glob(dr + '*.txt')
    for file in files:
        with open(file, 'r') as f:
            data = f.read().splitlines()
        out_file = out_dr + file.split('\\')[-1]
        print(out_file)
        with open(out_file, 'w') as f:
            for datum in tqdm(data):
                output_data = ''
                o = othello()
                o.player = black
                idx = 0
                for i in range(hw):
                    for j in range(hw):
                        o.grid[i][j] = black if datum[idx] == 'p' else white if datum[idx] == 'o' else vacant
                        idx += 1
                o.check_legal()
                '''
                for i in range(hw):
                    for j in range(hw):
                        if o.grid[i][j] == legal:
                            oo = deepcopy(o)
                            oo.move(i, j)
                            input_datum = '1\n'
                            for k in range(hw):
                                for l in range(hw):
                                    input_datum += '0' if oo.grid[k][l] == black else '1' if oo.grid[k][l] == white else '.'
                                input_datum += '\n'
                            output_data += chr(ord('!') + i * hw + j)
                            for depth in range(MAX_DEPTH):
                                egaroucid.stdin.write((input_datum + str(depth) + '\n').encode('utf-8'))
                                egaroucid.stdin.flush()
                                line = egaroucid.stdout.readline().decode()
                                val = int(line)
                                output_data += score_to_string(val)
                            output_data += ' '
                '''
                input_datum = '0\n'
                for k in range(hw):
                    for l in range(hw):
                        input_datum += '0' if o.grid[k][l] == black else '1' if o.grid[k][l] == white else '.'
                    input_datum += '\n'
                for depth in range(MAX_DEPTH):
                    egaroucid.stdin.write((input_datum + str(depth) + '\n').encode('utf-8'))
                    egaroucid.stdin.flush()
                    line = egaroucid.stdout.readline().decode()
                    val = int(line)
                    output_data += score_to_string(val)
                f.write(datum + ' ' + output_data + '\n')

egaroucid.kill()