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
	fractal->buddha->mlx_win_map = mlx_new_window(fractal->mlx_connect, fractal->width, fractal->height, "Importance map");
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
	zero_matrix(fractal->densities[hist], fractal->width, fractal->height);
	printf(MAGENTA"Running channel: "BLUE"%d ...\n"RESET, fractal->hist_num);
	if (b->fast)
		fast_buddha(fractal);//using monte carlo importance
	else
		buddha(fractal);//for using normal random sampling method
	num = high_hit_count(fractal->width, fractal->height, fractal->densities[hist]);
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

	