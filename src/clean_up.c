#include "fractol.h"

void	free_matrices(t_fractal *fractal)
{
	free_matrix_i(fractal->pdf, fractal->height);
	free_3d_array_i(fractal->densities, fractal->histograms, fractal->height);
	free(fractal->cdf);
}

void	clear_all(t_fractal *fractal)
{
	if (fractal->density != NULL)
		free_matrix_i(fractal->density, fractal->height);
	if (fractal->buddha)
		free(fractal->buddha);
	if (fractal->w_colors)
		free(fractal->w_colors);
	if (fractal->img.img_ptr)
		mlx_destroy_image(fractal->mlx_connect, fractal->img.img_ptr);
	if (fractal->img_2.img_ptr)
		mlx_destroy_image(fractal->mlx_connect, fractal->img_2.img_ptr);
	if (fractal->mlx_win)
		mlx_destroy_window(fractal->mlx_connect, fractal->mlx_win);
	pthread_mutex_destroy(&fractal->mutex);
	pthread_mutex_destroy(&fractal->rand_mtx);
	mlx_destroy_display(fractal->mlx_connect);
	free(fractal->mlx_connect);
	perror("Malloc or thread error");
	exit(EXIT_FAILURE);
}

void	thread_error(t_fractal *fractal, int i)
{
	while (--i >= 0)
	{
		if (fractal->threads[i])
			pthread_join(fractal->threads[i], NULL);
	}
	pthread_mutex_destroy(&fractal->mutex);
	pthread_mutex_destroy(&fractal->rand_mtx);
	clear_all(fractal);
}

int	close_handler(t_fractal *fractal)
{
	pthread_mutex_destroy(&fractal->mutex);
	pthread_mutex_destroy(&fractal->rand_mtx);

	mlx_destroy_image(fractal->mlx_connect, fractal->img.img_ptr);
	mlx_destroy_image(fractal->mlx_connect, fractal->img_2.img_ptr);
	mlx_destroy_window(fractal->mlx_connect, fractal->mlx_win);
	mlx_destroy_display(fractal->mlx_connect);
	free(fractal->w_colors);
	free(fractal->threads);
	free(fractal->mlx_connect);
	if (fractal->id == 3)
	{
		free_matrices(fractal);
		free(fractal->buddha);
	}
	exit(EXIT_SUCCESS);
	return (0);
}