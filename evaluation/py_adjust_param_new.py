import subprocess
import sys

phase = str(sys.argv[1])
if len(sys.argv) > 3:
    hour = str(sys.argv[2])
    minute = str(sys.argv[3])
    second = str(sys.argv[4])
    beta = str(sys.argv[5])
else:
    if int(phase) >= 20:
        hour = '0'
        minute = '20'
        second = '0'
        beta = '0.005'
    else:
        hour = '0'
        minute = '20'
        second = '0'
        beta = '0.005'
if int(phase) >= 20:
    additional_params = ' data_0000004.dat data_0000006.dat data_0000007.dat data_0000008.dat data_0000009.dat data_0000010.dat'
else:
    additional_params = ' data_0000001.dat data_0000002.dat data_0000003.dat'
#additional_params = ' big_data_new_3.dat big_data_new_14.dat'
#additional_params = ' big_data.dat'

cmd = 'adjust_param_new.out ' + phase + ' ' + hour + ' ' + minute + ' ' + second + ' ' + beta + ' learned_data/' + phase + '.txt' + additional_params
print(cmd, file=sys.stderr)
p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
result = p.stdout.readline().decode().replace('\r\n', '\n').replace('\n', '')
print(result)
param = p.stdout.read().decode().replace('\r\n', '\n')
with open(phase + '.txt', 'w') as f:
    f.write(param)
