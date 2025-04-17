# include "image_processing.h"

int	get_num_cores(void)
{
	int	num_cores;

	#if defined(_WIN32) || defined(_WIN64)
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		num_cores = sysinfo.dwNumberOfProcessors;
	#else
		num_cores = sysconf(_SC_NPROCESSORS_ONLN);
		if (num_cores < 1)
		{
			perror("sysconf error\n");
			return (1);
		}
	#endif
	return (num_cores);
}

void	join_threads(pthread_t *threads, int num_rows, int num_cols)
{
	int	i;
	int	j;

	j = -1;
	while (++j < num_rows)
	{
		i = -1;
		while (++i < num_cols)
			pthread_join(threads[j * num_cols + i], NULL);
	}
}

void	set_least_diff_pair(int num, int *fact1, int *fact2)
{
	int min_diff;
	int diff;
	int i;
	int j;

	min_diff = num;
	i = 1;
	while (i <= (num / i))
	{
		if (num % i == 0)
		{
			j = num / i;
			diff = abs(i - j);
			if (diff < min_diff)
			{
				min_diff = diff;
				*fact1 = i;
				*fact2 = j;
			}
		}
		i++;
	}
}

long	get_time(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL))
	{
		putstr_fd("gettimeofday Failure\n", 2);
		exit(EXIT_FAILURE);
	}
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void print_times(long start, long end, char *title, char *msg, char *color)
{
	double duration;
	duration = (double)(end - start) / 1000;
	ft_putstr_color(title, color);
	printf(msg, duration);
}
