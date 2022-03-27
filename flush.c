#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>

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

char **parse_input(char *input)
{
    char **parsed_array = NULL;
    char *token = strtok(input, " \t");

    int length = 0;
    while (token)
    {
        parsed_array = realloc(parsed_array, sizeof(char *) * (++length));
        token[strcspn(token, "\r\n")] = 0;
        parsed_array[length - 1] = token;
        token = strtok(NULL, " ");
    }
    parsed_array = realloc(parsed_array, sizeof(char *) * (length + 1));
    parsed_array[length] = NULL;

    return parsed_array;
}

int execute_task(char **input)
{
    char **arg_list = NULL;
    int index = 0;
    while (*input)
    {
        if (strcmp(*input, "<") == 0)
        {
            FILE *fp = fopen(*(++input), "r");
            dup2(fileno(fp), STDIN_FILENO);
        }

        else if (strcmp(*input, ">") == 0)
        {
            FILE *fp = fopen(*(++input), "w");
            dup2(fileno(fp), 1);
        }
        else
        {
            arg_list = realloc(arg_list, strlen(*input) + 1);
            arg_list[index] = *input;
            index++;
        }
        *input++;
    }
    execvp(arg_list[0], arg_list);
    return 0;
}

int main()
{
    while (1)
    {
        // print current directory
        printf("%s: ", getcwd(NULL, 0));
        char buf[PATH_MAX];
        fgets(buf, sizeof(buf), stdin);

        if (strcmp(buf, "exit\n") == 0)
        {
            exit(0);
        }
        char **parsed_array = parse_input(buf);
        execute_task(parsed_array);
        // pid_t kek = fork();
        // if (kek == 0)
        // {
        //     exit(0);
        // }
        // else
        // {
        //     int status;
        //     waitpid(kek, &status, 0);
        //     if (WIFEXITED(status))
        //     {
        //         printf("Exit status = %d\n", WEXITSTATUS(status));
        //     }
        // }
        fflush(NULL);
    }
}
