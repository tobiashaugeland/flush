from subprocess import Popen, PIPE
import argparse

parser = argparse.ArgumentParser(description='Test the program')


process = Popen('./flush', stdout=PIPE, stdin=PIPE)
process.stdin.write(b'ls\n')
process.stdin.flush()
while True:
    line = process.stdout.readline()
    if not line:
        break
    print(line)
process.stdin.write(b'exit\n')
process.stdin.flush()