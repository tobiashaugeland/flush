#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>


int make_dir(const char *pathname, mode_t mode)
{
    int ret;
    ret = mkdir(pathname, mode);
    if (ret == -1)
    {
        perror("could not make directory");
        return -1;
    }
    return 0;
}

int main()
{
    // print current directory
    printf("Current directory: %s\n", getcwd(NULL, 0));
    char buf[1024];
    fgets(buf, sizeof(buf), stdin);
    make_dir(buf,0777);
}