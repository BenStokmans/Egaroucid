import subprocess

with open('compile_cmd.txt', 'r') as f:
    cmd = f.read()
o = subprocess.run(cmd, shell=True, encoding='utf-8', stderr=subprocess.STDOUT, timeout=None)
print('------------------compiled------------------')
