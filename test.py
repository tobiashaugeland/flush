from subprocess import Popen, PIPE

process = Popen(['./flush'], stdout=PIPE, stdin=PIPE)
# process.stdin.write('echo test\n')
stdout, stdin = process.communicate()