#include "fractol.h"

int	ft_clamp(double val, int low, int high)
{
	if (val > high)
		val = high;
	else if (val < low)
		val = low;
	return ((int)val);
}

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

//grid within each pixel according to importance

void	sample_pixel_grid(t_fractal *fractal, int x, int y, t_comps comps, Xoro128 *rng)
{
	int 		i, j;
	t_complex	c;
	double 		offset;
	double		offset_x, offset_y;
	double 		sample_prob = comps.pdf[y][x];
	int 		pix_samples = ft_round(comps.samples * sample_prob);
	int 		limit = ft_round(sqrt(pix_samples));
	double		weight = 1.0 / sample_prob;

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
			c.x = map_2((double)x + offset_x, comps.x_cmin, comps.slopex_to); //* comps.inv_zoom + comps.move_x;
			c.y = map_2((double)y + offset_y, comps.y_cmin, comps.slopey_to);// * comps.inv_zoom - comps.move_y;
			offset_x += offset;
			buddha_iteration(fractal, c, weight, comps);
		}
		offset_y += offset;
	}
}


int	get_hits(t_complex c, t_comps comps)
{
	t_complex	z;
	int			iterations;
	int			x, y, hits;

	iterations = 0;
	hits = 0;
	z.x = 0.0;
	z.y = 0.0;

	while ((z.x * z.x) + (z.y * z.y) < 4.0 && iterations < comps.b_max_i)
	{
		z = sum_complex(comps.complex_f(z), c);
		iterations++;
		
		x = ft_round(map_back_2((z.x - comps.move_x) * comps.zoom, comps.x_cmin, comps.slopex_back));
		y = ft_round(map_back_2((z.y + comps.move_y) * comps.zoom, comps.y_cmin, comps.slopey_back));
		if (x >= 0 && x < comps.width && y >= 0 && y < comps.height)
			hits++;
	}
	if (iterations < comps.b_max_i && iterations > comps.b_min_i)
		return (hits);
	else
		return (0);
}

double	**map_sub_pixel(double x, double y, int dim, t_comps comps)//base n on number of samples for pixel higher gets more subpix
{
	double		step, sx, sy, hits, total;
	int			i, j;
	t_complex	c;
	double** subpdfv = malloc_matrix(dim, dim);//--

	int ss = 3; // sub-sampling resolution per subpixel cell
	//double substep = 1.0 / (n * ss); // tiny step per fine sample
	total = 0;
	step = 1.0 / (double)dim;
	for (j = 0; j < dim; j++)
	{
		for (i = 0; i < dim; i++)
		{
			//another set of loops here maybe to improve map at level
			hits = 0;
			for (int sj = 0; sj < comps.ss; sj++)//trying with sub loops
			{
				for (int si = 0; si < comps.ss; si++)
				{
					sx = x + ((double)i + ((double)si + 0.5) / (double)comps.ss) * step;
					sy = y + ((double)j + ((double)sj + 0.5) / (double)comps.ss) * step;
					c.x = map_2(sx, comps.x_cmin, comps.slopex_to);
					c.y = map_2(sy, comps.y_cmin, comps.slopey_to);
					hits += get_hits(c, comps);
				}
			}
			//-----------
			/* sx = x + ((double)i + .5) * step;
			sy = y + ((double)j + .5) * step;
			c.x = map_2(sx, comps.x_cmin, comps.slopex_to);
			c.y = map_2(sy, comps.y_cmin, comps.slopey_to);
			hits = get_hits(c, comps); */
			subpdfv[j][i] = hits;
			total += hits;
		}
	}
	if (total == 0)
		total = 1.0;
	for (j = 0; j < dim; j++)
	{
		for (i = 0; i < dim; i++)
		{
			subpdfv[j][i] /= total;
			//comps.subpdf[j][i] /= total;
			//printf("pdfs: %f\n", pdf[j][i]);
		}
	}
	//return (comps.subpdf);
	return (subpdfv);
}

void	sample_subpix(t_fractal *fractal, double **pixpdf, double pix_samps, double y, double x, int j, int i, t_comps comps, Xoro128 *rng, double samp_prob, int dim)
{
	int 		z;
	t_complex	c;
	double 		subsamp_prob = pixpdf[j][i];
	int 		sub_samples = ft_round(pix_samps * subsamp_prob);
	

	if (subsamp_prob < 1e-8)
    subsamp_prob = 1e-8;

	double step = 1.0 / (double)(dim);	
	double	weight = 1.0 / (subsamp_prob * samp_prob);
	z = -1;
	while (++z < sub_samples)
	{	
		c.x = map_2((double)x + ((double)i + xoro128d(rng)) * step, comps.x_cmin, comps.slopex_to);
		c.y = map_2((double)y + ((double)j + xoro128d(rng)) * step, comps.y_cmin, comps.slopey_to);
		buddha_iteration(fractal, c, weight, comps);
	}
}

//random within each pixel. currently used, and has xoroshiro128 random 0 <= rand <= 1.0

void	sample_pixel(t_fractal *fractal, int x, int y, t_comps comps, Xoro128 *rng)
{
	int 		i;
	t_complex	c;
	double 		sample_prob = comps.pdf[y][x];
	int 		pix_samples = ft_round(comps.samples * sample_prob);


	if (pix_samples == 0)
		return ;
	

	if (sample_prob < 1e-8)
    	sample_prob = 1e-8;

	//if (pix_samples < 4)//try tiered sampling,  if  low < samples < high
	/* {
		double		weight = 1.0 / sample_prob;
		i = -1;
		while (++i < pix_samples)
		{
			c.x = map_2((double)x + xoro128d(rng), comps.x_cmin, comps.slopex_to);// * comps.inv_zoom + comps.move_x;
			c.y = map_2((double)y + xoro128d(rng), comps.y_cmin, comps.slopey_to);// * comps.inv_zoom - comps.move_y;
			buddha_iteration(fractal, c, weight, comps);
		}
	} */
	//else //subsamp--
	{
		int a, b;
		//int dim = ft_round(comps.zoom + 1);
		//int dim = ft_round(sqrt(pix_samples));
		//int dim = ft_round(sample_prob * sqrt(pix_samples));
//		int dim = ft_clamp(ft_round(comps.zoom * sqrt(pix_samples) * sample_prob), 2, 16);
		//int dim = ft_clamp(ft_round(comps.zoom / sample_prob), 2, 16);
		int dim = ft_round(sqrt(comps.zoom)) + 1;
		/* if (pix_samples > 3000)
		printf("samples: %d dim: %d\n", pix_samples, dim); */

		//comps.subpdf = map_sub_pixel(x, y, comps);//pack into a 1d later for speed using for fixed
		double **subpdfv = map_sub_pixel(x, y, dim, comps);
		//print_matrix(subpdf, 3, 3);
		b = 0;
		for (b = 0; b < dim; b++)
		{
			a = 0;
			for (a = 0; a < dim; a++)
				sample_subpix(fractal, subpdfv, pix_samples, y, x, b, a, comps, rng, sample_prob, dim);
			//sample_subpix(fractal, comps.subpdf, pix_samples, y, x, b, a, comps, rng, sample_prob);
				
		}
		free_matrix_i(subpdfv, dim);//--
	}
}
/* for (b = 0; b < dim; b++)
	for (a = 0; a < dim; a++)
		subpdfv[b][a] = 0.0;  */

/* //within win
c.x = map_2((double)x + offset_x, -2, comps.slopex_to) * comps.inv_zoom + comps.move_x;
c.y = map_2((double)y + offset_y, 2, comps.slopey_to) * comps.inv_zoom - comps.move_y;
buddha_iteration(fractal, c, weight, comps); */
/* c.x = ((double)x - comps.width / 2 + offset_x) / (comps.width) * comps.span / comps.zoom + comps.move_x;
c.y = ((double)y - comps.height / 2 + offset_y) / (comps.height) * comps.span / comps.zoom - comps.move_y; */

//double aspect = (double)fractal->width / (double)fractal->height;
static inline void	*buddha_set_fast(void *arg)
{
	t_piece 	*piece;
	t_fractal	*fractal;
	t_comps		comps;
	int 		x, y;
	
	piece = (t_piece *)arg;
	fractal = piece->fractal;
	comps = set_comps(fractal, false);
	
	y = piece->y_s;
	x = piece->x_s - 1;
	while (++x < comps.width)//first row
			sample_pixel(fractal, x, y, comps, &piece->rng);
	while (++y < piece->y_e)//body
	{
		x = -1;
		while (++x < comps.width)
			sample_pixel(fractal, x, y, comps, &piece->rng);
	}
	x = -1;
	while (++x < piece->x_e)//last row
		sample_pixel(fractal, x, y, comps, &piece->rng);
	//free_matrix_i(comps.subpdf, (int)comps.sn);
	pthread_exit(NULL);
}

//sampling points all at once in accordance with importance. setting thread data here for balanced work

void	fast_buddha(t_fractal *fractal)
{
	int 		num_threads = fractal->num_rows * fractal->num_cols;
	t_piece		piece[num_threads];
	int			i;
	double		limit;
	int			index;
	int			x;
	int			y;
	int			x_prev;
	int			y_prev;
	uint64_t	glob_seed;
	
	glob_seed = 0xFABDECAF;//secret number fAb DeCaF
	if (!fractal->move_y && fractal->buddha->copy_half)//half_copy
		limit = 1.0 / (2.0 * (double)num_threads);
	else
		limit = 1.0 / (double)num_threads;
	x_prev = 0;
	y_prev = 0;
	i = -1;
	//set and run all but last thread
	while (++i < num_threads - 1)
	{
		index = binary_search(fractal->cdf, fractal->size, (double)(i + 1) * limit);
		x = index % fractal->width;
		y = index / fractal->width;

		piece[i].id = i;
		piece[i].x_s = x_prev;
		piece[i].x_e = x;
		piece[i].y_s = y_prev;
		piece[i].y_e = y;
		piece[i].fractal = fractal;
		sxoro128(&piece[i].rng, glob_seed + i * 142857);
		x_prev = x;
		y_prev = y;
		if (pthread_create(&fractal->threads[i], NULL, \
			buddha_set_fast, (void *)&piece[i]) != 0)
			thread_error(fractal, i);
	}
	//set and run last thread
	piece[i].id = i;
	piece[i].x_s = x_prev;
	piece[i].x_e = fractal->width;
	piece[i].y_s = y_prev;	
	if (!fractal->move_y && fractal->buddha->copy_half)//half_copy
		piece[i].y_e = (fractal->height >> 1) - 1;
	else
		piece[i].y_e = fractal->height - 1;
	piece[i].fractal = fractal;
	sxoro128(&piece[i].rng, glob_seed + i * 142857);		
	if (pthread_create(&fractal->threads[i], NULL, \
		buddha_set_fast, (void *)&piece[i]) != 0)
		thread_error(fractal, i);	
	i = -1;
	//finish
	while (++i < num_threads)
		pthread_join(fractal->threads[i], NULL);
	if (!fractal->move_y && fractal->buddha->copy_half)
	{
		copy_buddha_half_fast(fractal->densities[fractal->hist_num], fractal->height, fractal->width);
		//copy_buddha_half_map(fractal->densities[fractal->hist_num], fractal->height, fractal->width);//FOR USE WITH X AND Y NOT SWAPPED.
	}
}
/*Diagnostic prints*/

//printf("thread limits: %d, x_s: %d, y_s: %d, x_e: %d, y_e: %d\n", i, piece[i].x_s, piece[i].y_s, piece[i].x_e, piece[i].y_e);// for troubshoot

/* long start = get_time(); //for thread test
	printf(GREEN"Thread: "BLUE"%d\n"RESET, piece->id); //for thread comparison diagnostics
	print_times(start, get_time(), "\0", "Total thread time: %f seconds\n", BOLD_BLUE); */