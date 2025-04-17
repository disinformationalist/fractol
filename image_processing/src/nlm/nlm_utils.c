#include "image_processing.h"

void	send_tpixd_to_img(t_pixeld **out, t_img *img, int width, int height)
{
	int				i;
	int				j;
	unsigned int	color;

	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
		{
			color = (((int)out[j][i].red << 16) | ((int)out[j][i].green << 8) | (int)out[j][i].blue);
			my_pixel_put(j, i, img, color);
		}
	}
	free_pix_d_matrix(out, height);
}

t_img	*convert_tpixd_to_img(void *mlx_ptr, t_pixeld **out, int width, int height)
{
	int				i;
	int				j;
	unsigned int	color;
	t_img			*new;

	new = create_img(mlx_ptr, width, height);
	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
		{
			color = (((int)out[j][i].red << 16) | ((int)out[j][i].green << 8) | (int)out[j][i].blue);
			my_pixel_put(j, i, new, color);
		}
	}
	free_pix_d_matrix(out, height);
	return (new);
}

void	thread_err(pthread_t *threads, int i)
{
	while (--i >= 0)
	{
		if (threads[i])
			pthread_join(threads[i], NULL);
	}
	//free all stuff
	//pthread_mutex_destroy(&fractal->mutex);
	//clear_all(fractal);
}

void	img_to_tpix(t_img *img, t_pixeld ***p_img, int width, int height)
{
	unsigned int	pixel;
	t_pixeld		pixel_t;
	int				x;
	int				y;

	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < width)
		{	
			pixel = pixel_color_get(x, y, img);
			pixel_t.red = (pixel >> 16) & 0xFF;
			pixel_t.green = (pixel >> 8) & 0xFF;
			pixel_t.blue = pixel & 0xFF;
			(*p_img)[y][x] = pixel_t;
		}
	}
}

double	gauss_weight(double dist, double h)
{
	return (exp(-dist / (h * h)));
}