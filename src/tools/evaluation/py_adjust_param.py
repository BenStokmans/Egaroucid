import subprocess
import sys

phase = str(sys.argv[1])
if len(sys.argv) > 3:
    hour = str(sys.argv[2])
    minute = str(sys.argv[3])
    second = str(sys.argv[4])
    beta = str(sys.argv[5])
else:
    hour = '0'
    minute = '10'
    second = '0'
    beta = '2.0'

train_data = [
    'data5_01.dat',
    'data5_02.dat',
    'data5_04.dat',
    'data5_06.dat',
    'data5_07.dat',
    'data5_08.dat',
    'data5_09.dat',
    'data5_10.dat',
    'data5_11.dat',
    'data5_15.dat',
    'data5_16.dat',
    'data5_99.dat'
]

train_dir = './../../../train_data/bin_data/20230201/'
model_dir = './../../../model/'

additional_params = ''
for tfile in train_data:
    additional_params += ' ' + train_dir + tfile

#additional_params = ' big_data_new_3.dat big_data_new_14.dat'
#additional_params = ' big_data.dat'

#executable = 'Egaroucid5_evaluation_optimizer_cuda.exe'
executable = 'adjust_param.out'

cmd = executable + ' ' + phase + ' ' + hour + ' ' + minute + ' ' + second + ' ' + beta + ' ' + model_dir + phase + '.txt' + additional_params
print(cmd, file=sys.stderr)
p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
result = p.stdout.readline().decode().replace('\r\n', '\n').replace('\n', '')
print(result)
param = p.stdout.read().decode().replace('\r\n', '\n')
with open(phase + '.txt', 'w') as f:
    f.write(param)
