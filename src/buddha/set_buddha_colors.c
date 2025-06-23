#include "fractol.h"

static inline void set_pixel(t_fractal *fractal, int i, int j, unsigned int color)
{
	if (!fractal->supersample)
		my_pixel_put_plus(i, j, &fractal->img, color);
	else
		fractal->pixels_xl[j][i] |= color;
}

// Increase contrast (simple method)
//float contrast = 1.5; // >1 increases contrast, <1 reduces it
//color = (color - 0.5) * contrast + 0.5;

/* t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0)
result = t * t * (3 - 2 * t) */


double	clampf(double f)
{
	if (f < 0.0)
		f = 0.0;
	if (f > 1.0)
		f = 1.0;
	return (f);
}

double smoothstep(double edge0, double edge1, double x)
{
	double t;

	t = clampf((x - edge0) / (edge1 - edge0));
	return (t * t * (3 - 2 * t));
}

//x * x * x * (x * (6.0f * x - 15.0f) + 10.0f)

double smootherstep(double edge0, double edge1, double x)
{
	double t;

	t = clampf((x - edge0) / (edge1 - edge0));
	return (t * t * t * (t * (6.0f * t - 15.0f) + 10.0f));
}


static inline void	set_blue(t_fractal *fractal, double num, double **density, t_piece *piece, den_f numf)
{
	unsigned int		color;
	int					i;
	int					j;
	double				n;
	double				factor;
	t_buddha			b;

	b = *fractal->buddha;

	if (b.smootherstep)
		n = 1.0 / numf(num, b.bpow);
	else
		n = 255.0 / numf(num, b.bpow);

	//n = 255.0 / numf(log(num + 1), fractal->buddha->bpow);//log scaling
	j = piece->y_s - 1;
	while (++j < piece->y_e)
	{
		i = piece->x_s - 1;
		while (++i < piece->x_e)
		{
			factor = numf(density[j][i], b.bpow) * n;
			//factor = numf(log(density[j][i] + 1), fractal->buddha->bpow) * n;
			
			if (b.smootherstep)
				color = ft_round(smootherstep(b.edge0_b, b.edge1_b, factor) * 255.0);
			else
				color = ft_round(factor);
			
			//---
			//density[j][i] = color;
			set_pixel(fractal, i, j, color);
		}
	}
}

static inline void	set_green(t_fractal *fractal, double num, double **density, t_piece *piece, den_f numf)
{
	unsigned int	color;
	int				i;
	int				j;
	double			n;
	double			factor;
	t_buddha		b;

	b = *fractal->buddha;

	if (b.smootherstep)
		n = 1.0 / numf(num, b.gpow);
	else
		n = 255.0 / numf(num, b.gpow);

	j = piece->y_s - 1;
	while (++j < piece->y_e)
	{
		i = piece->x_s - 1;
		while (++i < piece->x_e)
		{
			factor = numf(density[j][i], b.gpow) * n;

			if (b.smootherstep)
				color = ft_round(smootherstep(b.edge0_g, b.edge1_g, factor) * 255.0) << 8;
			else
				color = ft_round(factor) << 8;
				//---
			//density[j][i] = color;
			set_pixel(fractal, i, j, color);
		}
	}
}

static inline void	set_red(t_fractal *fractal, double num, double **density, t_piece *piece, den_f numf)
{
	unsigned int	color;
	int				i;
	int				j;
	double			n;
	double			factor;
	t_buddha		b;


	b = *fractal->buddha;
	if (b.smootherstep)
		n = 1.0 / numf(num, b.rpow);
	else
		n = 255.0 / numf(num, b.rpow);

//	n = 255.0 / numf(log(num + 1), fractal->buddha->rpow);//make this method too

	j = piece->y_s - 1;
	while (++j < piece->y_e)
	{
		i = piece->x_s - 1;
		while (++i < piece->x_e)
		{
			factor = numf(density[j][i], b.rpow) * n;
//			factor = numf(log(density[j][i] + 1), fractal->buddha->rpow) * n;

			if (b.smootherstep)
				color = ft_round(smootherstep(b.edge0_r, b.edge1_r, factor) * 255.0) << 16;
			else
				color = ft_round(factor) << 16;
			
				//---
			//density[j][i] = color;
			set_pixel(fractal, i, j, color);
		}
	}
}

double pow_ft(double num, double power)
{
    return pow(num, power);
}
//(log(hit + 1) / log(max_hit + 1)

//pow(log(hit + 1) / log(max_hit + 1), gamma)
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

	if (b->filter && b->ftype == MEAN)
	{
		if (b->fchan == 0)
			mean_convo_matrix(fractal->densities[0], width, height, b->flevel);
		else if (b->fchan == 1)
			mean_convo_matrix(fractal->densities[1], width, fractal->height, b->flevel);
		else
			mean_convo_matrix(fractal->densities[2], width, height, b->flevel);
	}
	/* if (fractal->buffs > 1)//need to color all buffs to get proper diffs/ avg? or just do smoother on buffs before all else
	{
		int i = -1;
		while(++i < fractal->buffs)//color all buffs, need to move set pixel...
		{
			set_blue(fractal, b->high_b, fractal->densities[0 + i * 3], piece, pow_ft);
			set_green(fractal, b->high_g, fractal->densities[1 + i * 3], piece, pow_ft);
			set_red(fractal, b->high_r, fractal->densities[2 + i * 3], piece, pow_ft);
		}
	} 
	else */
	{
		set_blue(fractal, b->high_b, fractal->densities[0], piece, pow_ft);
		set_green(fractal, b->high_g, fractal->densities[1], piece, pow_ft);
		set_red(fractal, b->high_r, fractal->densities[2], piece, pow_ft);
	}
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

void nlm_channel2(t_fractal *fractal, double **color, double **var, int width, int height)
{
	int f = 1; //patch_rad;//1 for buddha2, 2 for buddha 1
	int r = 15; //search_rad;
	double kc = 1.0;
	double eps = 1e-8;

	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			double total_weight = 0.0;
			double filtered_val = 0.0;
			  for (int dy = -r; dy <= r; dy++) {
                for (int dx = -r; dx <= r; dx++) {
                    int qx = i + dx;
                    int qy = j + dy;

                    if (qx < 0 || qx >= width || qy < 0 || qy >= height)
                        continue;

                    double d2 = 0.0;

                    for (int py = -f; py <= f; py++) {
                        for (int px = -f; px <= f; px++) {
                            int p1x = i + px, p1y = j + py;
                            int p2x = qx + px, p2y = qy + py;

                            if (p1x < 0 || p1x >= width || p1y < 0 || p1y >= height ||
                                p2x < 0 || p2x >= width || p2y < 0 || p2y >= height)
                                continue;

                            double u1 = color[p1y][p1x];
                            double u2 = color[p2y][p2x];
                            double var1 = var[p1y][p1x];
                            double var2 = var[p2y][p2x];

                            double delta = (u1 - u2) * (u1 - u2);
                            double var_corr = var1 + ((var1 < var2) ? var1 : var2);
                            double norm = eps + kc * kc * (var1 + var2);

                            d2 += (delta - var_corr) / norm;
                        }
                    }

                    d2 = fmax(0.0, d2 / ((2*f + 1)*(2*f + 1)));

                    double w = exp(-d2);
                    total_weight += w;
                    filtered_val += w * color[qy][qx];
                }
            }
			
			set_pixel(fractal, i, j, (total_weight > 0) ? filtered_val / total_weight : color[j][i]);
		}
	}
	/* free_matrix_i(color, height);
	return (out); */
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
	//for taking variances after coloring
	/* combine_buff_set_var(fractal->densities, 0, fractal->buffs, fractal->width, fractal->height);
	nlm_channel2(fractal, fractal->densities[0], fractal->densities[3], fractal->width, fractal->height);
	nlm_channel2(fractal, fractal->densities[1], fractal->densities[4], fractal->width, fractal->height);
	nlm_channel2(fractal, fractal->densities[2], fractal->densities[5], fractal->width, fractal->height); */
}
