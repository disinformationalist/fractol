#include "fractol.h"

void	color_op_barn(double iter, t_fractal *fractal, t_position *p)
{
	unsigned int		color;
	t_complex			d;

	(void)iter;
	color = 0;
	d.x = 0;
	d.y = 0;
	if (fractal->color_spectrum)
	{
		d.x = map(p->x, -2, +2, fractal->width) - fractal->move_col_x;
		d.y = map(p->y, +2, -2, fractal->height) + fractal->move_col_y;
		color = get_color(d.x, d.y, fractal);
	}
	else
		color = fractal->w_colors[fractal->col_i];		
	put_pixel(p->x, p->y, fractal, color);
}

void	map_barnsley(t_fractal *fractal, t_position *pos, double xn, double yn, double zoom)
{
	double x_max = 2.1820;
	double y_max = 9.9983;
	double oldmax_minus_oldmin_x = 4.8378; 
	
	pos->x = (int)(((((xn * zoom - fractal->move_x) + x_max) / oldmax_minus_oldmin_x * fractal->width)));
	pos->y = (int)((((y_max - yn * zoom - fractal->move_y) / y_max * fractal->height)));
}

void	get_rand_point(double r, double *xn, double *yn, double **vals, t_fractal *fractal)
{
	double	x_old;
	double	y_old;
	double 	r_lim1;
	double	r_lim2;
	double	r_lim3;

	r_lim1 = vals[0][6];
	r_lim2 = vals[0][6] + vals[1][6];
	r_lim3 = r_lim2 + vals[2][6];
	x_old = *xn;
	y_old = *yn;
	if (r < r_lim1)//f1 STEM
	{
		*xn = vals[0][0] * x_old + vals[0][1] * y_old + vals[0][4];
		*yn =  vals[0][2] * x_old + vals[0][3] * y_old + vals[0][5];
	}
	else if (r < r_lim2)//f2 SHAPE
	{
		*xn = vals[1][0] * x_old + vals[1][1] * y_old + vals[1][4];
		*yn = vals[1][2] * x_old + vals[1][3] * y_old + vals[1][5];
	}
	else if (r < r_lim3)//f3 LEFT
	{
		*xn = vals[2][0] * x_old + vals[2][1] * y_old + vals[2][4];
		*yn = vals[2][2] * x_old + vals[2][3] * y_old + vals[2][5];
	}
	else//f4 RIGHT
	{
		*xn = vals[3][0] * x_old + vals[3][1] * y_old + vals[3][4];
		*yn = vals[3][2] * x_old + vals[3][3] * y_old + vals[3][5];
	}
}

void	calc_barnsleyfern(t_fractal *fractal, double zoom, int samples, Xoro128 *rng)
{
	t_position	*pos;
	double		**vals;
	int			n;
	double		xn;
	double		yn;

	pos = (t_position *)malloc(sizeof(t_position));
	pos->x = 0;
	pos->y = 0;
	xn = 0.0;
	yn = 0.0;
	vals = NULL;
	vals = alloc_fern(fractal, vals);
	n = -1;
	while (++n < samples)
	{
		get_rand_point(xoro128d(rng), &xn, &yn, vals, fractal);
		map_barnsley(fractal, pos, xn, yn, zoom);
		if ((pos->x >= 0 && pos->x < fractal->width) && (pos->y >= 0 && pos->y < fractal->height))
		{
			//color_op_barn(n, fractal, pos);
			fractal->fdensity[pos->y][pos->x] += 1;
		}
	}
	free_fern(vals, 4);
	free(pos);
}

int	render_barnsleyfern(t_fractal *fractal)
{
	int shift;

	shift = 63;
	fractal->colors.color_1 = fractal->w_colors[fractal->col_i + shift];
	fractal->colors.color_2 = fractal->w_colors[(fractal->col_i + 120 + shift) % 360];
	fractal->colors.color_3 = fractal->w_colors[(fractal->col_i + 240 + shift) % 360];
	fractal->colors.color_4 = fractal->w_colors[(fractal->col_i + shift) % 360];	
	if (!fractal->supersample)
		ft_memset(fractal->img.pixels_ptr, 0, fractal->width * fractal->height * (fractal->img.bpp / 8));
	fern(fractal);
	density_color(fractal);
	return (0);
}

void	*fern_set(void *arg)
{
	t_fractal	*fractal;
	t_piece		*piece;
	Xoro128		*rng;
	int			i;
	double		zoom;
	double 		samples;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	zoom = 1.0 / fractal->zoom;
	if (fractal->supersample)
		samples = (fractal->max_i * 7) * fractal->height * fractal->s_kernel * fractal->s_kernel * fractal->zoom;
	else
		samples = (fractal->max_i * 7) * fractal->height * fractal->zoom;
	samples /= (double)fractal->num_rows * (double)fractal->num_cols;
	rng = &piece->rng;
	calc_barnsleyfern(fractal, zoom, samples, rng);
	pthread_exit(NULL);
}