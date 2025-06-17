#include "fractol.h"

void	put_pixel2(int x, int y, t_fractal *fractal, unsigned int color)
{
	if (fractal->hist_num == 1)
		color = color << 8;
	else if (fractal->hist_num == 2)
		color = color << 16;

	if (!fractal->supersample)
	{
		//my_pixel_put(x, y, &fractal->img_2, color);
		my_pixel_put_plus(x, y, &fractal->img_2, color);
	}
	else
		fractal->pixels_xl[y][x] = color;
}

double	high_hit_count(int width, int height, double **density)
{
	int		i;
	int		j;
	double	high;

	j = -1;
	high = 0.0;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
		{
			if (high < density[j][i])
				high = density[j][i];
		}
	}
	return ((high));
}

void create_cdf(t_fractal *fractal, double **pdf)
{
	int		width = fractal->width;
	int		height = fractal->height;
	int		size = width * height;
	double	*cdf = fractal->cdf;
	double	cumulative_sum = 0.0;
    int		index = 0;
    
	for (int j = 0; j < height; j++)
	{
        for (int i = 0; i < width; i++)
		{
            cumulative_sum += pdf[j][i];
            cdf[index] = cumulative_sum;
            index++;
        }
    }
    // Normalize the CDF so that the last element is exactly 1.0
	double norm_factor =  1.0 / cumulative_sum;
	for (int k = 0; k < size; k++)
      cdf[k] *= norm_factor;
}

void	show_map(t_fractal *fractal, double **density)
{
	int i, j;
	double	importance;
    double	*density_row;
	int high_density = high_hit_count(fractal->width, fractal->height, density);
	j = -1;
	while (++j < fractal->height)
	{
		i = -1;
		density_row = density[j];
		while (++i < fractal->width)
		{
			importance = fmin(255, density_row[i] * 10 / (double)high_density);
			put_pixel2(j, i, fractal, (importance * 255.0));		
		}
	}
	fractal->buddha->mlx_win_map = mlx_new_window(fractal->mlx_connect, fractal->width, \
		fractal->height, "Importance map");
	if (fractal->mlx_win == NULL)
		clear_all(fractal);
	mlx_put_image_to_window(fractal->mlx_connect,
		fractal->buddha->mlx_win_map, fractal->img_2.img_ptr, 0, 0);
}

void	build_importance_map(t_fractal *fractal, double **density)
{
	int		i;
	int		j;
	double	sum;
	double 	norm_factor;
	double	**pdf = fractal->pdf;
    double	*densities_row;
	double	prob;
	
	sum = get_matrix_sum(density, fractal->width, fractal->height);
	norm_factor = 1.0 / sum;
	j = -1;
	while (++j < fractal->height)
	{
		i = -1;
		densities_row = density[j];
		while (++i < fractal->width)
		{
			prob = (densities_row[i] * norm_factor);
			pdf[j][i] = prob;
		}
	}
	create_cdf(fractal, fractal->pdf);
}

void	set_channel(t_fractal *fractal, int buddha_min, int buddha_iters, char channel)
{
	fractal->b_min_i = buddha_min;
	fractal->b_max_i = buddha_iters;
	
	if (channel == 'b')
		fractal->hist_num = 0;
	else if (channel == 'g')
		fractal->hist_num = 1;
	else if (channel == 'r')
		fractal->hist_num = 2;
}




void	combine_buff_set_var(double ***densities, int hist, int buffs, int width, int height)
{
	int		j;
	int		i;
	int		k;
	double	avg;
	double	var;
	double	sum;

		j = -1;
		while (++j < height)
		{
			i = -1;
			while (++i < width)
			{
				k = -1;
				sum = 0;
				//get avg
				while (++k < buffs)
					sum += densities[hist + k * 3][j][i];
				avg = sum / buffs;
				k = -1;
				sum = 0;
				//get var
				while (++k < buffs)
				{
					double v1 = avg - densities[hist + k * 3][j][i];
					sum += v1 * v1;
				}
				var = sum / buffs;
				densities[hist][j][i] = avg;
				densities[hist + 3][j][i] = var;
			}
		}
}

double	**nlm_channel(double **color, double **var, int width, int height)
{
	int f = 2; //patch_rad;
	int r = 10; //search_rad;
	double kc = 1.0;
	double eps = 1e-8;
	double **out;

	out = malloc_matrix(width, height);
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
            out[j][i] = (total_weight > 0) ? filtered_val / total_weight : color[j][i];
		}
	}
	free(color);
	return (out);
}


void	run_and_reset(t_fractal *fractal, int buddha_min, int buddha_iters, char channel)
{
	double		num;
	long		start;
	int			hist;
	long		map_start;
	long		run_start; 
	t_buddha	*b;

	if (buddha_iters <= 0)
		return ;
	b = fractal->buddha;
	start = get_time();
	set_channel(fractal, buddha_min, buddha_iters, channel);
	hist = fractal->hist_num;
	map_start = get_time();
	if (b->fast)
	{
		printf(MAGENTA"Mapping channel: "BLUE"%d ...\n"RESET, hist);
		buddha_map(fractal);
		build_importance_map(fractal, fractal->densities[hist]);
		//show_map(fractal, fractal->densities[fractal->hist_num]);//for diagnostics
		print_times(map_start, get_time(), "\0", "Map channel time  : %f seconds\n", MAGENTA);
	}
	run_start = get_time();
	zero_matrix(fractal->densities[hist], fractal->width, fractal->height); //only because map uses first.
	printf(MAGENTA"Running channel: "BLUE"%d ...\n"RESET, fractal->hist_num);
	if (b->fast)
	{
		fast_buddha(fractal);//using monte carlo importance
		if (fractal->buffs > 1)
		{
			combine_buff_set_var(fractal->densities, hist, fractal->buffs, fractal->width, fractal->height);//try smootherstep first..
			fractal->densities[hist] = nlm_channel(fractal->densities[hist], fractal->densities[hist + 3], fractal->width, fractal->height);
		}
	}
	else
		buddha(fractal);//for using normal random sampling method
	num = high_hit_count(fractal->width, fractal->height, fractal->densities[hist]);//hist + 3 when viewing variances
	if (channel == 'b')
		b->high_b = num;
	else if (channel == 'g')
		b->high_g = num;
	else
		b->high_r = num;
	print_times(run_start, get_time(), "\0", "Run channel time  : %f seconds\n", MAGENTA);
	printf(MAGENTA"Channel: "BLUE"%d Complete\n"RESET, fractal->hist_num);
	print_times(start, get_time(), "\0", "Total channel time: %f seconds\n\n", MAGENTA);
}

void	render_buddha(t_fractal *fractal)
{	
	long start = get_time();
	
	//compute  histos for each channel, save in 3d array fractal->densities
	run_and_reset(fractal, fractal->buddha->min1, fractal->buddha->max1, 'b');//letter swaps work
	run_and_reset(fractal, fractal->buddha->min2, fractal->buddha->max2, 'g');
	run_and_reset(fractal, fractal->buddha->min3, fractal->buddha->max3, 'r');
	color_buddha(fractal);
	print_times(start, get_time(), "RENDER COMPLETE\n", "Total render time : "GREEN"%f"RESET" seconds\n", BOLD_BLUE);
}

	