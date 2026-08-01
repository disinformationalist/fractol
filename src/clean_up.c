#include "fractol.h"

void	free_matrices(t_fractal *fractal)
{
	int	height;
	int	histograms;

	height = fractal->buddha_workspace_height;
	histograms = fractal->buddha_workspace_histograms;
	if (height <= 0)
		height = fractal->height;
	if (histograms <= 0)
		histograms = fractal->histograms;
	buddha_free_worker_histograms(fractal);
	if (fractal->buddha)
		buddha_clear_importance_proposal(fractal->buddha);
	free(fractal->sample_counts);
	if (fractal->densities)
		free_3d_array_i(fractal->densities, histograms, height);
	fractal->sample_counts = NULL;
	fractal->densities = NULL;
	fractal->buddha_workspace_width = 0;
	fractal->buddha_workspace_height = 0;
	fractal->buddha_workspace_histograms = 0;
	fractal->buddha_workspace_workers = 0;
}

void	clear_all(t_fractal *fractal)
{
	if (fractal->buddha)
	{
		if (fractal->sample_counts || fractal->densities
			|| fractal->worker_histograms)
			free_matrices(fractal);
		free(fractal->buddha->importance_values);
		free(fractal->buddha->render_pixels);
		buddha_clear_importance_proposal(fractal->buddha);
		free(fractal->buddha);
	}
	if (fractal->w_colors)
		free(fractal->w_colors);
	if (fractal->img.img_ptr)
		mlx_destroy_image(fractal->mlx_connect, fractal->img.img_ptr);
	if (fractal->img_2.img_ptr)
		mlx_destroy_image(fractal->mlx_connect, fractal->img_2.img_ptr);
	if (fractal->display_img.img_ptr)
		mlx_destroy_image(fractal->mlx_connect,
			fractal->display_img.img_ptr);
	if (fractal->mlx_win)
		mlx_destroy_window(fractal->mlx_connect, fractal->mlx_win);
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
	clear_all(fractal);
}

int	close_handler(t_fractal *fractal)
{
	mlx_destroy_image(fractal->mlx_connect, fractal->img.img_ptr);
	mlx_destroy_image(fractal->mlx_connect, fractal->img_2.img_ptr);
	if (fractal->display_img.img_ptr)
		mlx_destroy_image(fractal->mlx_connect,
			fractal->display_img.img_ptr);
	mlx_destroy_window(fractal->mlx_connect, fractal->mlx_win);
	mlx_destroy_display(fractal->mlx_connect);
	free(fractal->w_colors);
	free(fractal->threads);
	free(fractal->mlx_connect);
	if (fractal->id == 3)
	{
		free_matrices(fractal);
		free(fractal->buddha->importance_values);
		free(fractal->buddha->render_pixels);
		buddha_clear_importance_proposal(fractal->buddha);
		free(fractal->buddha);
	}
	if (fractal->fdensity)
		free_fdensity(fractal, fractal->height);
	exit(EXIT_SUCCESS);
	return (0);
}
