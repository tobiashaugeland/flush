#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/limits.h>


int change_directory(const char *pathname)
{
    int ret;
    ret = chdir(pathname);
    if (ret == -1)
    {
        perror("could not change directory");
        return -1;
    }
    return 0;
}

int execute_command(const char *command)
{
    int ret;
    ret = system(command);
    if (ret == -1)
    {
        perror("could not execute command");
        return -1;
    }
    return 0;
}


int main()
{
    while(1) {
    // print current directory
    printf("%s: ", getcwd(NULL, 0));
    char buf[PATH_MAX];
    fgets(buf, sizeof(buf), stdin);
    // change directory
    execute_command(buf);
    }
}
