import subprocess
from time import time
import sys

strt_idx = int(sys.argv[1])
end_idx = int(sys.argv[2])

egaroucid = subprocess.Popen('a.exe'.split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE)

res_str = ''
tim = 0
stim = 0
nodes = 0
strt = time()
for i in range(strt_idx, end_idx):
    print('#', i)
    with open('./../../benchmark/ffotest/' + str(i) + '.txt', 'r') as f:
        s = f.read()
    egaroucid.stdin.write(s.encode('utf-8'))
    egaroucid.stdin.flush()
    result = egaroucid.stdout.readline().decode()
    res_str += '#' + str(i) + ' ' + result
    tim += int(result.split()[10])
    stim += int(result.split()[13])
    nodes += int(result.split()[7])
egaroucid.kill()

answer = '''#40 38  a2
#41 0   h4
#42 6   g2
#43 -12 c7  g3
#44 -14 d2  b8
#45 6   b2
#46 -8  b3
#47 4   g2
#48 28  f6
#49 16  e1
#50 10  d8
#51 6   e2  a3
#52 0   a3
#53 -2  d8
#54 -2  c7
#55 0   g6  g4  b7
#56 2   h5
#57 -10 a6
#58 4   g1
#59 64  g8  h4  e8'''

res_str_proc = ''
for line, ans_line in zip(res_str.splitlines(), answer.splitlines()):
    ans_score = ans_line.split()[1]
    ans_policies = ans_line.split()[2:]
    score = line.split()[4]
    policy = line.split()[6]
    res_str_proc += line
    if ans_score != score:
        res_str_proc += ' WRONG_SCORE'
    if not (policy in ans_policies):
        res_str_proc += ' WRONG_POLICY'
    res_str_proc += '\n'

print('done')
print(res_str_proc, end='')
print(tim / 1000, 'sec')
print(stim / 1000, 'sec search')
print(time() - strt, 'sec total')
print(nodes, 'nodes')
print(nodes / stim * 1000, 'nps')