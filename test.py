from subprocess import Popen, PIPE
import subprocess
import argparse
import os
import pgrep


parser = argparse.ArgumentParser(description='Test the program')

parser.add_argument('-f', '--file', help='The file to test', default='flush')

args = parser.parse_args()
file = args.file


if not os.path.exists('testfolder'):
    subprocess.run(['mkdir', 'testfolder'])


process = Popen('./'+file, stdout=PIPE, stdin=PIPE,
                bufsize=1, universal_newlines=True)


def restart_process(p):
    p.kill()
    p = Popen('./'+file, stdout=PIPE, stdin=PIPE,
              bufsize=1, universal_newlines=True)
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
    assert current_dir != new_dir and new_dir.find(
        'testfolder') != -1, 'The directory is not changed'
    print('Test two passed')


def test_three():
    command = execute_command('echo heisann')
    assert command == 'heisann', 'The output is not correct, should been heisann, was: ' + command
    print('Test three passed')


def test_four():
    execute_command('echo heisann > testfile')
    local_file = open('testfile', 'r')
    assert local_file.readline().strip() == 'heisann', 'The file is not correct'
    cat_command = execute_command('cat testfile')
    assert cat_command == 'heisann', 'The output is not correct, should been heisann, was: ' + cat_command
    print('Test four passed')

def test_five():
    execute_command('head -1 < testfile > testfile2')
    local_file = open('testfile2', 'r')
    assert local_file.readline().strip() == 'heisann', 'The file is not correct'
    os.remove('testfile')
    os.remove('testfile2')
    print('Test five passed')

def test_six():
    process.stdin.write('sleep 2 & \n')
    sleep = pgrep.pgrep('sleep')
    for num in sleep:
        print('sleep: ' + str(num))
    assert len(sleep) >= 1, 'The sleep process is not found'
    jobs = execute_command('jobs')
    print('jobs: '+jobs)
    assert jobs.find('sleep') != -1, 'The jobs command is not correct'


test_one()
process = restart_process(process)
test_two()
process = restart_process(process)
test_three()
process = restart_process(process)
test_four()
process = restart_process(process)
test_five()
process = restart_process(process)
test_six()

os.removedirs('testfolder')

print('All test passed')