from subprocess import Popen, PIPE

process = Popen('./flush', stdout=PIPE, stdin=PIPE)
process.stdin.write(b'echo test\n')
process.stdin.flush()
stdout, stdin = process.communicate()