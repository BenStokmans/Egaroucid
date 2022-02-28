import subprocess
import sys

phase = str(sys.argv[1])
player = str(sys.argv[2])
hour = str(sys.argv[3])
minute = str(sys.argv[4])
second = str(sys.argv[5])
'''
with open(phase + '_' + player + '.txt', 'r') as f:
    data = f.read().replace('\n\n', '\n')
with open(phase + '_' + player + '.txt', 'w') as f:
    f.write(data)
exit()
'''
cmd = 'adjust_param.out ' + phase + ' ' + player + ' ' + hour + ' ' + minute + ' ' + second + ' learned_data/' + phase + '_' + player + '.txt'
print(cmd)
p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
param = p.stdout.read().decode().replace('\r\n', '\n')
with open(phase + '_' + player + '.txt', 'w') as f:
    f.write(param)
