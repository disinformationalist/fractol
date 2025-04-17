#include "fractol.h"

static inline void set_pixel(t_fractal *fractal, int i, int j, unsigned int color)
{
	if (!fractal->supersample)
		my_pixel_put_plus(i, j, &fractal->img, color);
	else
		fractal->pixels_xl[j][i] |= color;
}

static inline void	set_blue(t_fractal *fractal, double num, double **density, t_piece *piece, den_f numf)
{
	unsigned int		color;
	int					i;
	int					j;
	double				n;
	double				factor;
	
	n = 255.0 / numf(num, fractal->buddha->bpow);
	j = piece->y_s - 1;
	while (++j < piece->y_e)
	{
		i = piece->x_s - 1;
		while (++i < piece->x_e)
		{
			factor = numf(density[j][i], fractal->buddha->bpow) * n;
			color = ft_round(factor);
			set_pixel(fractal, i, j, color);
		}
	}
}

static inline void	set_green(t_fractal *fractal, double num, double **density, t_piece *piece, den_f numf)
{
	int		color;
	int		i;
	int		j;
	double	n;
	double	factor;

	n = 255.0 / numf(num, fractal->buddha->gpow);
	j = piece->y_s - 1;
	while (++j < piece->y_e)
	{
		i = piece->x_s - 1;
		while (++i < piece->x_e)
		{
			factor = numf(density[j][i], fractal->buddha->gpow) * n;
			color = ft_round(factor) << 8;
			set_pixel(fractal, i, j, color);
		}
	}
}

static inline void	set_red(t_fractal *fractal, double num, double **density, t_piece *piece, den_f numf)
{
	int		color;
	int		i;
	int		j;
	double	n;
	double	factor;

	n = 255.0 / numf(num, fractal->buddha->rpow);
	j = piece->y_s - 1;
	while (++j < piece->y_e)
	{
		i = piece->x_s - 1;
		while (++i < piece->x_e)
		{
			factor = numf(density[j][i], fractal->buddha->rpow) * n;
			color = ft_round(factor) << 16;
			set_pixel(fractal, i, j, color);
		}
	}
}

double pow_ft(double num, double power)
{
    return pow(num, power);
}

//double log_ft()

double	**matcpy(double **dest, double **src, int width, int height)
{
	int		j;
	int		i;

	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
			dest[j][i] = src[j][i];
	}
	return (dest);
}


/* double	**matdup(double **src, int width, int height)
{
	double	**dest;

	dest = malloc_matrix(width, height);
	if (!dest)
		return (NULL);
	return (matcpy(dest, src, width, height));
} */

static inline void	*set_colors(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;
	t_buddha	*b;
	int			width;
	int			height;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	b = fractal->buddha;
	width = fractal->width;
	height = fractal->height;

	if (b->filter)
	{
		if (b->fchan == 0)
			mean_convo_matrix(fractal->densities[0], width, height, b->flevel);
		else if (b->fchan == 1)
			mean_convo_matrix(fractal->densities[1], width, fractal->height, b->flevel);
		else
			mean_convo_matrix(fractal->densities[2], width, height, b->flevel);
	}
	set_blue(fractal, b->high_b, fractal->densities[0], piece, pow_ft);
	set_green(fractal, b->high_g, fractal->densities[1], piece, pow_ft);
	set_red(fractal, b->high_r, fractal->densities[2], piece, pow_ft);
	pthread_exit(NULL);
}

static inline void	set_pieces_color(t_fractal *fractal, t_piece piece[][fractal->num_cols], int i, int j)
{
	piece[j][i].x_s = i * (fractal->width / fractal->num_cols);
	piece[j][i].x_e = (i + 1) * (fractal->width / fractal->num_cols);
	piece[j][i].y_s = j * (fractal->height / fractal->num_rows);
	piece[j][i].y_e = (j + 1) * (fractal->height / fractal->num_rows);
	piece[j][i].fractal = fractal;
}

void	color_buddha(t_fractal *fractal)
{
	t_piece	piece[fractal->num_rows][fractal->num_cols];
	int		i;
	int		j;

	j = -1;
	while (++j < fractal->num_rows)
	{
		i = -1;
		while (++i < fractal->num_cols)
		{
				set_pieces_color(fractal, piece, i, j);
			if (pthread_create(&fractal->threads[j * fractal->num_cols + i], NULL, \
				&set_colors, (void *)&piece[j][i]) != 0)
				thread_error(fractal, j * fractal->num_cols + i);
		}
	}
	join_threads(fractal->threads, fractal->num_rows, fractal->num_cols);
}
