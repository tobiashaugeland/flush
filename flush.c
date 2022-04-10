#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "LinkedList.h"

#define MAX_PATH 4096

typedef struct command_list
{
    char **argv;
} command_list;

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
        token = strtok(NULL, " \t");
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
 * @return number of tasks to be executed
 */
int parse_input(char *input, command_list *command_list)
{
    char *input_copy = strdup(input);
    int index = 0;
    char *p;
    while (p = strsep(&input_copy, "|"))
    {
        command_list = realloc(command_list, sizeof(command_list) * (++index));
        command_list[index - 1].argv = smaller_parsing(p);
    }
    free(input_copy);
    return index;
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

void kill_all_inactive_processes(node *head)
{
    node *current = head;
    while (current->next)
    {
        node *prev = current;
        current = next_node(current);
        int status;
        if (!(waitpid(getPid(current), &status, WNOHANG) == 0))
        {
            printf("Exit status [%s]: %d\n", getData(current), WEXITSTATUS(status));
            free(current->data);
            deleteNode(head, current);
            current = prev;
        }
    }
}

void print_active_processes(node *head)
{
    node *current = head;
    while (current->next)
    {
        current = next_node(current);
        pid_t pid = getPid(current);
        char *data = getData(current);
        printf("[%d] %s\n", pid, data);
    }
}
int main()
{
    node *head = init_list();
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

        kill_all_inactive_processes(head);

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
            print_active_processes(head);
            fflush(NULL);
            continue;
        }

        else if (strncmp(internal_command, "cd", 2) == 0)
        {
            change_directory(buf + 3);
            continue;
        }

        // Please don't pipe more than 16 commands, okay? Thank you, bye.
        command_list *parsed_array = malloc(sizeof(command_list));
        int n_pipe = parse_input(buf, parsed_array);

        pid_t child_pid = fork();
        if (child_pid == 0)
        {
            execute_task(n_pipe, parsed_array);
        }
        else
        {
            if (res)
            {
                process_data *data = malloc(sizeof(process_data));
                data->pid = child_pid;
                strcpy(data->command, buf);
                addNode(head, data);
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

        for (int i = 0; i < n_pipe; i++)
        {
            free(parsed_array[i].argv);
        }
        free(parsed_array);

        fflush(NULL);
    }
}
