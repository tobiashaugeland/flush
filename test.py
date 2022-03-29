import argparse
import os
from time import sleep
import pgrep
import multiprocessing
import pexpect


parser = argparse.ArgumentParser(description='Test the shell')

parser.add_argument('-f', '--file', help='The file to test', default='flush')
parser.add_argument('-e', '--extended',
                    help='Run extended tests', action='store_true')
parser.add_argument(
    '-d', '--dynamic', help='Run dynamic tests. Use this if you have dynamic arrays', action='store_true')
parser.add_argument('-a', '--all', help='Run all tests', action='store_true')

args = parser.parse_args()
file = args.file
extended = args.extended or args.all
dynamic = args.dynamic or args.all

if not os.path.exists('testfolder'):
    os.mkdir('testfolder')


def test_one():
    runner = pexpect.spawn('./'+file)
    runner.sendline('/bin/echo test')
    runner.expect('test')
    assert runner.after == b'test', 'The output is not correct, was: ' + \
        str(runner.after)
    runner.kill(0)
    print('Test one passed')


def test_two():
    runner = pexpect.spawn('./'+file)
    current_dir = os.getcwd()
    runner.write('cd testfolder\n')
    runner.write('pwd\n')
    runner.expect(current_dir+'/testfolder')
    new_dir = runner.after.decode('utf-8').strip()
    assert current_dir != new_dir and new_dir.find(
        'testfolder') != -1, 'The directory is not changed'
    runner.kill(0)
    print('Test two passed')


def test_three():
    runner = pexpect.spawn('./'+file)
    runner.write('echo test\n')
    runner.expect('test')
    res = runner.after.decode('utf-8').strip()
    assert res == 'test', 'The output is not correct, should been heisann, was: ' + res
    runner.kill(0)
    print('Test three passed')


def test_four():
    runner = pexpect.spawn('./'+file)
    runner.write('echo heisann > testfile\n')
    sleep(0.05)
    local_file = open('testfile', 'r')
    assert local_file.readline().strip() == 'heisann', 'The file is not correct'
    runner.write('cat testfile\n')
    runner.expect('heisann')
    res = runner.after.decode('utf-8').strip()
    assert res == 'heisann', 'The output is not correct, should been heisann, was: ' + res
    runner.kill(0)
    print('Test four passed')


def test_five():
    runner = pexpect.spawn('./'+file)
    runner.write('head -1 < testfile > testfile2\n')
    sleep(0.05)
    local_file = open('testfile2', 'r')
    assert local_file.readline().strip() == 'heisann', 'The file is not correct'
    os.remove('testfile')
    os.remove('testfile2')
    runner.kill(0)
    print('Test five passed')


def test_six():
    runner = pexpect.spawn('./'+file)
    runner.write('sleep 5 &\n')
    sleep = pgrep.pgrep('sleep')
    assert len(sleep) >= 1, 'The sleep process is not found'
    runner.write('jobs\n')
    runner.expect('sleep')
    res = runner.after.decode('utf-8').strip()
    assert res.find('sleep') != -1, 'The sleep process is not found'
    runner.kill(0)
    print('Test six passed')


def test_seven():
    local_file = open('testfile', 'w')
    local_file.write('line one\n')
    local_file.write('line two\n')
    local_file.close()
    runner = pexpect.spawn('./'+file)
    runner.write('cat testfile | head -1 | cat > testfile2\n')
    sleep(0.05)
    local_file = open('testfile2', 'r')
    assert local_file.readline().strip() == 'line one', 'The file is not correct'
    os.remove('testfile')
    os.remove('testfile2')
    runner.kill(0)
    print('Test seven passed')


def test_eight():
    test_string = 'echo '
    append_string = 'a '*100
    append_string += '\n'
    runner = pexpect.spawn('./'+file)
    runner.write(test_string+append_string)
    runner.expect('a '*100)
    res = runner.after.decode('utf-8').strip()
    assert sum(c.isalpha() for c in res) == 100, 'The output is not correct'
    runner.kill(0)
    print('Test eight passed')


def main():
    test_one()
    test_two()
    test_three()
    test_four()
    test_five()
    test_six()
    if extended:
        test_seven()
    if dynamic:
        test_eight()
    os.removedirs('testfolder')


p = multiprocessing.Pool(1)

deferred = p.apply_async(main)
try:
    res = deferred.get(timeout=5)
    print('All tests passed')
except multiprocessing.TimeoutError:
    print('TimeoutError, did you remember to fflush stdout in your shell?')

p.terminate()
