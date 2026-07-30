#include "fractol.h"

int	buddha_init_worker_histograms(t_fractal *fractal)
{
	size_t	bytes;
	int		worker;

	fractal->worker_histograms = NULL;
	fractal->worker_histogram_count = 0;
	fractal->worker_histogram_count = fractal->num_rows * fractal->num_cols;
	fractal->worker_histograms = calloc(fractal->worker_histogram_count,
			sizeof(*fractal->worker_histograms));
	if (!fractal->worker_histograms)
		return (-1);
	worker = -1;
	while (++worker < fractal->worker_histogram_count)
	{
		fractal->worker_histograms[worker] = calloc((size_t)fractal->size,
				sizeof(*fractal->worker_histograms[worker]));
		if (!fractal->worker_histograms[worker])
		{
			buddha_free_worker_histograms(fractal);
			return (-1);
		}
	}
	if (buddha_profile_enabled())
	{
		bytes = (size_t)fractal->worker_histogram_count
			* (size_t)fractal->size * sizeof(double);
		printf("[buddha] worker histograms=%d (reused for %d buffers) "
			"memory=%.2f MiB\n", fractal->worker_histogram_count,
			fractal->buffs,
			(double)bytes / (1024.0 * 1024.0));
	}
	return (0);
}

void	buddha_free_worker_histograms(t_fractal *fractal)
{
	int	worker;

	if (!fractal->worker_histograms)
		return ;
	worker = -1;
	while (++worker < fractal->worker_histogram_count)
		free(fractal->worker_histograms[worker]);
	free(fractal->worker_histograms);
	fractal->worker_histograms = NULL;
	fractal->worker_histogram_count = 0;
}

void	buddha_clear_worker_histograms(t_fractal *fractal)
{
	long	start;
	int		worker;

	if (!fractal->worker_histograms)
		return ;
	start = get_time();
	worker = -1;
	while (++worker < fractal->worker_histogram_count)
		memset(fractal->worker_histograms[worker], 0,
			(size_t)fractal->size * sizeof(double));
	if (buddha_profile_enabled())
		printf("[buddha] %-18s %8.3f s\n", "worker clear",
			(double)(get_time() - start) / 1000.0);
}

static void	*reduce_worker_histograms(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;
	double		sum;
	size_t		index;
	size_t		end;
	int			worker;
	int			x;
	int			y;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	index = (size_t)piece->x_s;
	end = (size_t)piece->x_e;
	while (index < end)
	{
		y = (int)(index / (size_t)fractal->width);
		x = (int)(index - (size_t)y * (size_t)fractal->width);
		while (x < fractal->width && index < end)
		{
			sum = 0.0;
			worker = -1;
			while (++worker < fractal->worker_histogram_count)
				sum += fractal->worker_histograms[worker][index];
			fractal->densities[fractal->hist_num
				+ piece->buffer * 3][y][x] = sum;
			x++;
			index++;
		}
	}
	return (NULL);
}

static void	reduce_histograms_parallel(t_fractal *fractal, int buffer)
{
	t_piece	pieces[fractal->worker_histogram_count];
	int		worker;

	worker = -1;
	while (++worker < fractal->worker_histogram_count)
	{
		pieces[worker].x_s = (int)((size_t)worker
				* (size_t)fractal->size
				/ (size_t)fractal->worker_histogram_count);
		pieces[worker].x_e = (int)((size_t)(worker + 1)
				* (size_t)fractal->size
				/ (size_t)fractal->worker_histogram_count);
		pieces[worker].fractal = fractal;
		pieces[worker].buffer = buffer;
		if (pthread_create(&fractal->threads[worker], NULL,
				reduce_worker_histograms, &pieces[worker]) != 0)
			thread_error(fractal, worker);
	}
	join_threads(fractal->threads, fractal->worker_histogram_count, 1);
}

void	buddha_reduce_worker_histograms(t_fractal *fractal, int buffer)
{
	long	start;

	if (!fractal->worker_histograms)
		return ;
	start = get_time();
	reduce_histograms_parallel(fractal, buffer);
	if (buddha_profile_enabled())
		printf("[buddha] %-18s %8.3f s\n", "worker reduction",
			(double)(get_time() - start) / 1000.0);
}
