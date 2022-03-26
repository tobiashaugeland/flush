#include <stdio.h>
#include <unistd.h>
#include <errno.h>


int main()
{
    int i;
    for(i=0;i<10;i++)
    {
        printf("%d\n",i);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}