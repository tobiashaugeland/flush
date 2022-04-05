#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BACKGROUND_PROCESSES 10
#define MAX_PATH 4096
typedef struct process_data
{
    pid_t pid;
    char command[128];
} process_data;

typedef struct command_list
{
    char **argv;
} command_list;

process_data pids[MAX_BACKGROUND_PROCESSES];

/**
 * @brief Changes current working directory to the given path
 *
 * @return 0 on success, -1 on failure
 */
int change_directory(char *pathname)
{
    int ret = chdir(pathname);
    if (ret == -1)
    {
        perror("could not change directory");
        return -1;
    }
    return 0;
}

/**
 * @brief Splits char* into char**
 *
 * @param input String to be split
 * @return char**
 */
char **smaller_parsing(char *input)
{
    char **argv = NULL;
    int i = 0;
    char *token = strtok(input, " \t");
    while (token)
    {
        argv = realloc(argv, sizeof(input) * (++i));
        token[strcspn(token, " \r\n")] = 0;
        argv[i - 1] = token;
        token = strtok(NULL, " ");
    }
    argv = realloc(argv, sizeof(char *) * (i + 1));
    argv[i] = NULL;
    return argv;
}

/**
 * @brief Parses the input string and puts the commands into a command_list struct
 * seperated by pipes.
 *
 * @param input The input string
 * @return A command_list struct where the commands are to be stored
 */
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

        char **argv = NULL;
        int index = 0;

        while (*command_list->argv)
        {
            if (strcmp(*command_list->argv, "<") == 0)
            {
                FILE *fp = fopen(*(++command_list->argv), "r");
                dup2(fileno(fp), STDIN_FILENO);
            }
            else if (strcmp(*command_list->argv, ">") == 0)
            {
                FILE *fp = fopen(*(++command_list->argv), "w");
                dup2(fileno(fp), STDOUT_FILENO);
            }
            else
            {
                argv = realloc(argv, sizeof(**command_list->argv) * (++index));
                argv[index - 1] = *command_list->argv;
            }
            command_list->argv++;
        }

        return execvp(argv[0], argv);
    }
    return pid;
}

void execute_task(int n, command_list *input_list)
{
    int i, in, fd[2], index;
    pid_t pid;
    in = 0;

    for (i = 0; i < n - 1; i++)
    {
        pipe(fd);
        pid = pipe_task(in, fd[1], input_list + i);
        if (pid == -1)
        {
            perror("error in executing task");
            exit(1);
        }
        close(fd[1]);
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) != 0)
            {
                exit(WEXITSTATUS(status));
            }
        }
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
            dup2(fileno(fp), STDOUT_FILENO);
        }
        else
        {
            arg_list = realloc(arg_list, sizeof(*input) * (++index));
            arg_list[index - 1] = *input;
        }
        *input++;
    }
    if (in != 0)
    {
        dup2(in, 0);
    }
    execvp(arg_list[0], arg_list);

    perror("execvp");
    exit(1);
}

void kill_all_inactive_processes()
{
    int i;
    for (i = 0; i < MAX_BACKGROUND_PROCESSES; i++)
    {
        if (pids[i].pid != 0 && !(waitpid(pids[i].pid, NULL, WNOHANG) == 0))
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
        char buf[MAX_PATH];
        // print current directory
        printf("%s: ", getcwd(NULL, 0));

        // ctrl + d makes fgets return NULL
        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            printf("\n");
            exit(0);
        }
        buf[strcspn(buf, "\n")] = 0;

        kill_all_inactive_processes();

        if (strlen(buf) == 0)
        {
            continue;
        }

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
            fflush(NULL);
            continue;
        }

        else if (strncmp(internal_command, "cd", 2) == 0)
        {
            change_directory(buf + 3);
            continue;
        }

        // Please don't pipe more than 16 commands, okay? Thank you, bye.
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
        for (int i = 0; i < 16; i++)
        {
            if (parsed_array[i].argv != NULL)
            {
                free(parsed_array[i].argv);
            }
        }
        fflush(NULL);
    }
}
