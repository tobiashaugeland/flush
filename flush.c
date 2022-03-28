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

typedef struct
{
    char **argv;
} command_list;

process_data pids[MAX_BACKGROUND_PROCESSES];

int change_directory(char *pathname)
{
    pathname[strcspn(pathname, "\n")] = 0;
    int ret = chdir(pathname);
    if (ret == -1)
    {
        perror("could not change directory");
        return -1;
    }
    return 0;
}

char **smaller_parsing(char *input)
{
    char **argv = NULL;
    int i = 0;
    char *token = strtok(input, " \t");
    while (token)
    {
        argv = realloc(argv, sizeof(char *) * (++i));
        token[strcspn(token, " \r\n")] = 0;
        argv[i - 1] = token;
        token = strtok(NULL, " ");
    }
    argv = realloc(argv, sizeof(char *) * (i + 1));
    argv[i] = NULL;
    return argv;
}

void parse_input(char *input, command_list *command_list)
{
    char *input_copy = strdup(input);
    int index = 0;
    char *p;

    while ((p = strsep(&input_copy, "|")))
    {
        command_list[index++].argv = smaller_parsing(p);
    }
    free(input_copy);
}

int pipe_task(int in, int out, command_list *command_list)
{
    pid_t pid;

    if ((pid = fork()) == 0)
    {
        if (in != 0)
        {
            dup2(in, 0);
            close(in);
        }

        if (out != 1)
        {
            dup2(out, 1);
            close(out);
        }

        return execvp(command_list->argv[0], command_list->argv);
    }

    return pid;
}

void execute_task(int n, command_list *input_list)
{
    int i, in, fd[2], index;
    in = 0;

    for (i = 0; i < n - 1; i++)
    {
        pipe(fd);
        pipe_task(in, fd[1], input_list + i);
        close(fd[1]);
        in = fd[0];
    }

    char **arg_list = NULL;
    char **input = (input_list + i)->argv;
    index = 0;
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
    if (in != 0)
    {
        dup2(in, 0);
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

        else if (strstr(internal_command, "cd") != NULL)
        {
            change_directory(buf + 3);
            continue;
        }

        command_list parsed_array[16] = {0};
        parse_input(buf, parsed_array);

        int n_pipe = 0;

        for (int i = 0; i < 16; i++)
        {
            if (parsed_array[i].argv != NULL)
            {
                n_pipe++;
            }
        }

        pid_t child_pid = fork();
        if (child_pid == 0)
        {
            execute_task(n_pipe, parsed_array);
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
        free(parsed_array->argv);
        fflush(NULL);
    }
}
