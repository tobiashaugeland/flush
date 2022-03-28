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

#define MAX_BACKGROUND_PROCESSES 10
typedef struct
{
    pid_t pid;
    char command[128];
} process_data;

typedef struct {
    const char **argv;
} command_list;

process_data pids[MAX_BACKGROUND_PROCESSES];

int change_directory(char *pathname)
{
    pathname[strcspn(pathname, "\n")] = 0;
    int ret;
    ret = chdir(pathname);
    if (ret == -1)
    {
        perror("could not change directory");
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

void execute_task(char **input)
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
}

void kill_all_inactive_processes()
{
    int i;
    for (i = 0; i < MAX_BACKGROUND_PROCESSES; i++)
    {
        if (pids[i].pid != 0 && !waitpid(pids[i].pid, NULL, WNOHANG) == 0)
        {
            memset(&pids[i], 0, sizeof(pids[i]));
        }
    }
}

void print_active_processes()
{
    int i;
    for (i = 0; i < MAX_BACKGROUND_PROCESSES; i++)
    {
        if (pids[i].pid != 0)
        {
            printf("[%d] %s\n", pids[i].pid, pids[i].command);
        }
    }
}

int main()
{
    int pid_index = 0;
    while (1)
    {
        char buf[PATH_MAX];
        // print current directory
        printf("%s: ", getcwd(NULL, 0));

        // ctrl + d makes fgets return NULL
        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            printf("\n");
            exit(0);
        }

        kill_all_inactive_processes();

        // remove all data in input after &
        char *res = strstr(buf, "&");
        if (res)
        {
            memset(res, 0, strlen(res));
        }

        char internal_command[5] = {0};
        strncpy(internal_command, buf, 4);

        if (strcmp(internal_command, "exit") == 0)
        {
            exit(0);
        }

        else if (strcmp(internal_command, "jobs") == 0)
        {
            print_active_processes();
            continue;
        }

        else if(strstr(internal_command, "cd") != NULL)
        {
            change_directory(buf + 3);
            continue;
        }

        char **parsed_array = parse_input(buf);
        // execute_task(parsed_array);
        pid_t child_pid = fork();
        if (child_pid == 0)
        {
            execute_task(parsed_array);
        }
        else
        {
            if (res)
            {
                process_data data;
                data.pid = child_pid;
                strcpy(data.command, buf);
                pids[pid_index++ % 16] = data;
            }
            else
            {
                int status;
                waitpid(child_pid, &status, 0);
                if (WIFEXITED(status))
                {
                    printf("Exit status = %d\n", WEXITSTATUS(status));
                }
            }
        }
        fflush(NULL);
    }
}
