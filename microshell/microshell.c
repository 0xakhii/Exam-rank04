/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ojamal <ojamal@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/15 18:42:32 by ojamal            #+#    #+#             */
/*   Updated: 2023/08/15 19:35:04 by ojamal           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int	puterr(char *str, char *arg)
{
	while (str && *str)
		write(2, str++, 1);
	while (arg && *arg)
			write(2, arg++, 1);
	write(2, "\n", 1);
	return (1);
}

int execute(char **av, int i, int save_fd, char **env)
{
	av[i] = NULL;
	dup2(save_fd, STDIN_FILENO);
	close(save_fd);
	execve(av[0], av, env);
	return (puterr("Command not found: ", av[0]));
}

int	main(int ac, char **av, char **env)
{
	int	i;
	int fd[2];
	int save_fd;
	(void)ac;

	i = 0;
	save_fd = dup(STDIN_FILENO);
	while (av[i] && av[i + 1])
	{
		av = &av[i + 1];
		i = 0;
		while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
			i++;
		if (i == 0)
			continue;
		if (strcmp(av[0], "cd") == 0)
		{
			if (i != 2)
				puterr("error: cd: bad arguments", NULL);
			else if (chdir(av[1]) != 0)
				puterr("error: cd: No such file or directory ", av[1]);
		}
		else if ((av[i] == NULL || strcmp(av[i], ";") == 0))
		{
			if (fork() == 0)
			{	
				if (execute(av, i, save_fd, env))
					return (1);
			}
			else
			{
				close(save_fd);
				while(waitpid(-1, NULL, WUNTRACED) != -1);
				save_fd = dup(STDIN_FILENO);
			}
		}
		else if(strcmp(av[i], "|") == 0)
		{
			pipe(fd);
			if ( fork() == 0)
			{
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				if (execute(av, i, save_fd, env))
					return (1);
			}
			else
			{
				close(fd[1]);
				close(save_fd);
				save_fd = fd[0];
			}
		}
	}
	close(save_fd);
	return (0);
}