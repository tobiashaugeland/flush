from asyncio import subprocess
from subprocess import Popen, PIPE
import subprocess
import argparse
import os

from click import command

parser = argparse.ArgumentParser(description='Test the program')

if not os.path.exists('testfolder'):
    subprocess.run(['mkdir', 'testfolder'])


process = Popen('./flush', stdout=PIPE, stdin=PIPE, bufsize=1, universal_newlines=True)


def restart_process(p):
    p.kill()
    p = Popen('./flush', stdout=PIPE, stdin=PIPE, bufsize=1, universal_newlines=True)
    return p

def execute_command(command):
    process.stdin.write(command + '\n')
    output = process.stdout.readline().strip()
    return output


def test_one():
    command = execute_command('/bin/echo test')
    assert command == 'test', 'The output is not correct'
    print('Test one passed')


def test_two():
    current_dir = execute_command('pwd')
    execute_command('cd testfolder')
    new_dir = execute_command('pwd')
    assert current_dir != new_dir and new_dir.find('testfolder') != -1, 'The directory is not changed'
    print('Test two passed')

def test_three():
    command = execute_command('echo heisann')
    assert command == 'heisann', 'The output is not correct, should been heisann, was: ' + command
    print('Test three passed')

test_one()
process = restart_process(process)
test_two()
process = restart_process(process)
test_three()


# process.stdin.write('cd ..\n')
# print(process.stdout.readline())
# for line in io.TextIOWrapper(process.stdout, encoding='utf-8'):
#     print(line)
# process.stdin.write(b'exit\n')
# process.stdin.flush()
print('Tests passed')
exit(0)