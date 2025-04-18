#include "fractol.h"

void	copy_buddha_half_fast(double  **density, int height, int width)
{
	int i;
	int j;
	int limit = width >> 1;
	
	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < limit)//even number width only handled
		{
			density[j][i] += density[j][width - i - 1];
			density[j][width - i - 1] = density[j][i];
		}
	}
}

void	sample_pixel_rand(t_fractal *fractal, int x, int y, double total_samples, double slope_x1, double slope_y1, double slope_x, double slope_y, f complex_f, int hist)
{
	int 		i, j;
	t_complex	c;
	double 		offset;
	double		offset_x, offset_y;
	double 		sample_prob = fractal->pdf[y][x];
	int 		pix_samples = ft_round(total_samples * sample_prob);
	int 		limit = ft_round(sqrt(pix_samples));


	offset = 1.0 / limit;
	double half_off = offset / 2;

	j  = -1;
	offset_y = half_off;
	while(++j < limit - 1)
	{
		i = -1;
		offset_x = half_off;
		while(++i < limit - 1)
		{
			c.x = map_2((double)x + offset_x, -2, slope_x1);
			c.y = map_2((double)y + offset_y, 2, slope_y1);
			offset_x += offset;

			buddha_iteration(fractal, c, sample_prob, hist, slope_x, slope_y, complex_f);
		}
		offset_y += offset;
	}
}

void	sample_pixel(t_fractal *fractal, int x, int y, double total_samples, double slope_x1, double slope_y1, double slope_x, double slope_y, f complex_f, int hist)
{
	int 		i, j;
	t_complex	c;
	double 		offset;
	double		offset_x, offset_y;
	double 		sample_prob = fractal->pdf[y][x];
	int 		pix_samples = ft_round(total_samples * sample_prob);
	//int 		limit = ft_round(sqrt(pix_samples));

//-------- if limit < threshold use random instead, seems to be working at n + 3 or 4 threshold(some wierd stuff), a little bit slower with it as expected
	//if (limit < fractal->n + 4)
	{
		i = -1;
		while (++i < pix_samples)
		{
			pthread_mutex_lock(&fractal->rand_mtx);
			offset_x = randf();
			offset_y = randf();
			pthread_mutex_unlock(&fractal->rand_mtx);

			c.x = map_2((double)x + offset_x, -2, slope_x1);
			c.y = map_2((double)y + offset_y, 2, slope_y1);
			buddha_iteration(fractal, c, sample_prob, hist, slope_x, slope_y, complex_f);
		}
	}
	/* else
	{//this is using grid method
	offset = 1.0 / limit;
	double half_off = offset / 2;

	j  = -1;
	offset_y = half_off;
	while(++j < limit - 1)
	{
		i = -1;
		offset_x = half_off;
		while(++i < limit - 1)
		{
			c.x = map_2((double)x + offset_x, -2, slope_x1);
			c.y = map_2((double)y + offset_y, 2, slope_y1);
			offset_x += offset;

			buddha_iteration(fractal, c, sample_prob, hist, slope_x, slope_y, complex_f);
		}
		offset_y += offset;
	}
	} */
}


void	*buddha_set_fast(void *arg)
{
	t_piece 	*piece;
	t_fractal	*fractal;
	int 		i;
	
	
	piece = (t_piece *)arg;
	fractal = piece->fractal;

	double		offset_x;
	double		offset_y;
	double 		sample_prob;
	double		width;
	double		height;
	double		total_samples;

	width = (double)fractal->width;
	height = (double)fractal->height;
	f 			complex_f = fractal->complex_f;
	double 		slope_x = ((double)fractal->width - 1.0) / 4.0;//slope = (new_max - new_min) / (old_max - old_min);
	double 		slope_y = ((double)fractal->height - 1.0) / -4.0;//for the map_back...
	double 		slope_x1 = 4.0 / width;
	double 		slope_y1 = -4.0 / height;
	int 		hist = fractal->hist_num;
	int 		x, y;

	total_samples = (double)fractal->size * SQ(fractal->buddha->n);
	
	y = piece->y_s;
	x = piece->x_s - 1;
	while (++x < width)//first row
			sample_pixel(fractal, x, y, total_samples, slope_x1, slope_y1, slope_x, slope_y, complex_f, hist);
	while (++y < piece->y_e)//body
	{
		x = -1;
		while (++x < width)
		{
			//work each pixel in two loop grid sampling scheme
			sample_pixel(fractal, x, y, total_samples, slope_x1, slope_y1, slope_x, slope_y, complex_f, hist);	
		}
	}
	x = -1;
	while (++x < piece->x_e)//last row
		sample_pixel(fractal, x, y, total_samples, slope_x1, slope_y1, slope_x, slope_y, complex_f, hist);	

	pthread_exit(NULL);
}

void	fast_buddha(t_fractal *fractal)//sampling points all at once in accordance with importance. setting thread data here for balanced work
{
	int num_threads = fractal->num_rows * fractal->num_cols;
	t_piece	piece[num_threads];
	int		i;
	double	limit;
	int		index;
	int		x;
	int		y;
	int		x_prev;
	int		y_prev;

	if (!fractal->move_y && fractal->buddha->copy_half)//half_copy
		limit = 1.0 / (2.0 * (double)num_threads);
	else
		limit = 1.0 / (double)num_threads;
	x_prev = 0;
	y_prev = 0;
	i = -1;
	while (++i < num_threads - 1)
	{
		index = binary_search(fractal->cdf, fractal->size, (double)(i + 1) * limit);
		x = index % fractal->width;
		y = index / fractal->width;

		piece[i].x_s = x_prev;
		piece[i].x_e = x;
		piece[i].y_s = y_prev;
		piece[i].y_e = y;
		piece[i].fractal = fractal;
			
		x_prev = x;
		y_prev = y;
		//printf("thread: %d, x_s: %d, y_s: %d, x_e: %d, y_e: %d\n", i, piece[i].x_s, piece[i].y_s, piece[i].x_e, piece[i].y_e);// for troubshoot
		

		if (pthread_create(&fractal->threads[i], NULL, \
			buddha_set_fast, (void *)&piece[i]) != 0)
			thread_error(fractal, i);
		/* if (pthread_create(&fractal->threads[i], NULL, \
			buddha_set_fast_simd, (void *)&piece[i]) != 0)
			thread_error(fractal, i); */
	}
		piece[i].x_s = x_prev;
		piece[i].x_e = fractal->width;
		piece[i].y_s = y_prev;
		
		if (!fractal->move_y && fractal->buddha->copy_half)//half_copy
			piece[i].y_e = (fractal->height >> 1) - 1;
		else
			piece[i].y_e = fractal->height - 1;
		piece[i].fractal = fractal;
		//printf("thread: %d, x_s: %d, y_s: %d, x_e: %d, y_e: %d\n", i, piece[i].x_s, piece[i].y_s, piece[i].x_e, piece[i].y_e);//for troubshoot
		
		if (pthread_create(&fractal->threads[i], NULL, \
			buddha_set_fast, (void *)&piece[i]) != 0)
			thread_error(fractal, i);
			
		/* if (pthread_create(&fractal->threads[i], NULL, \
			buddha_set_fast_simd, (void *)&piece[i]) != 0)
			thread_error(fractal, i); */
	
			
	//long start = get_time(); //for thread tes
	i = -1;
	while (++i < num_threads)
	{
		pthread_join(fractal->threads[i], NULL);
		/* printf(GREEN"Thread: "BLUE"%d\n"RESET, i); //for thread comparison diagnostics
		print_times(start, get_time(), "\0", "Total thread time: %f seconds\n", BOLD_BLUE); */
	}
	if (!fractal->move_y && fractal->buddha->copy_half)
	{
		copy_buddha_half_fast(fractal->densities[fractal->hist_num], fractal->height, fractal->width);
		//copy_buddha_half_map(fractal->densities[fractal->hist_num], fractal->height, fractal->width);//FOR USE WITH X AND Y NOT SWAPPED.
	}
}
