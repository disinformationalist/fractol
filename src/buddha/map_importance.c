#include "fractol.h"

void	track_importance(t_fractal *fractal, t_complex c, double slope_x, double slope_y, f complex_f)
{
	t_complex	z;
	int			x;
	int			y;
	int			iterations;
	double		count_hits;
	int			i;
	int			j;
	int 		k = fractal->hist_num;
	double		move_x = fractal->move_x;
	double		move_y = fractal->move_y;
	double		zoom = fractal->zoom;
	double		*density_row;

	iterations = 0;
	count_hits = 0;
	z.x = 0;
	z.y = 0;
	while (iterations < fractal->b_max_i)
	{
		z = sum_complex(complex_f(z), c);
		iterations++;
		x = ft_round(map_back_2((z.x + move_x) * zoom, -2, slope_x));
		y = ft_round(map_back_2((z.y + move_y) * zoom, 2, slope_y));
		if (x >= 0 && x < fractal->width && y >= 0 && y < fractal->height)
			count_hits++;
	}
	i = ft_round(map_back_2(c.x, -2, slope_x));
	j = ft_round(map_back_2(c.y, 2, slope_y));
	pthread_mutex_lock(&fractal->mutex);
		density_row = fractal->densities[k][j];
        density_row[i] += count_hits;
	pthread_mutex_unlock(&fractal->mutex);
}

void	buddha_iteration_map(t_fractal *fractal, t_complex c, double slope_x, double slope_y, f complex_f)
{
	t_complex	z;
	int			iterations;

	iterations = 0;
	z.x = 0.0;
	z.y = 0.0;
	while ((z.x * z.x) + (z.y * z.y) < fractal->bound && iterations < fractal->b_max_i)
	{
		z = sum_complex(complex_f(z), c);
		iterations++;
	}
	if (iterations < fractal->b_max_i && iterations > fractal->b_min_i)
		track_importance(fractal, c, slope_x, slope_y, complex_f);
}

//grid mapping is used here as the fastest most accurate method to assign pixel importance

void	*buddha_set_map(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;
	t_complex	c;
	double		x;
	double		y;
	double		width;
	double		height;

	piece = (t_piece *)arg;
	fractal = piece->fractal;

	width = (double)(fractal->width) * fractal->buddha->map_n;
	height = (double)(fractal->height) * fractal->buddha->map_n;
	f 			complex_f = fractal->complex_f;
	double 		slope_x = ((double)fractal->width - 1.0) / 4.0;
	double 		slope_y = -((double)fractal->height - 1.0) / 4.0;//for the map back
	double 		slope_x1 = 4.0 / width;//for the map to
	double 		slope_y1 = -4.0 / height;

	y = (double)piece->y_s - 1;
	while (++y < piece->y_e)
	{
		x = (double)piece->x_s - 1;
		while (++x < piece->x_e)
		{
			c.x = map_2(x, -2, slope_x1);
			c.y = map_2(y, 2, slope_y1);
			buddha_iteration_map(fractal, c, slope_x, slope_y, complex_f);
		}
	}
	pthread_exit(NULL);
}

