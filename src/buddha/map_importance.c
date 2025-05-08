#include "fractol.h"

void	track_importance(t_fractal *fractal, t_complex c, t_comps comps, double n, double m)
{
	t_complex	z;
	int			x;
	int			y;
	int			iterations;
	double		count_hits;
	int			i;
	int			j;

	iterations = 0;
	count_hits = 0;
	z.x = 0;
	z.y = 0;
	while (iterations < comps.b_max_i)
	{
		z = sum_complex(comps.complex_f(z), c);
		iterations++;
		x = ft_round(map_back_2((z.x - comps.move_x) * comps.zoom, comps.x_cmin, comps.slopex_back));
		y = ft_round(map_back_2((z.y + comps.move_y) * comps.zoom, comps.y_cmin, comps.slopey_back));

		if (x >= 0 && x < comps.width && y >= 0 && y < comps.height)
			count_hits++;
	}
	i = (n / comps.n);//xpix
	j = (m / comps.n);//ypix
	//if (i >= 0 && i < fractal->width && j >= 0 && j < fractal->height)
	{ 
		pthread_mutex_lock(&fractal->mutex);
		comps.density[j][i] += count_hits;
		pthread_mutex_unlock(&fractal->mutex);
	}
}
/* 
void	buddha_iteration_map(t_fractal *fractal, t_complex c, t_comps comps, double x, double y)
{
	t_complex	z;
	int			iterations;

	t_complex	tort;
	t_complex	hare;
	int			power = 1;
	int			lam = 1;
	
	iterations = 0;
	z.x = 0.0;
	z.y = 0.0;
	tort = z;
	while ((z.x * z.x) + (z.y * z.y) < comps.bound
		&& iterations < comps.b_max_i)
	{
		hare = sum_complex(comps.complex_f(z), c);
		if (fabs(hare.x - tort.x) < 1e-14 && fabs(hare.y - tort.y) < 1e-14)
		{
			iterations = comps.b_max_i;
			break; // cycle detected
		}
		if (lam == power)
		{
			tort = hare;
			power *= 2;
			lam = 0;
		}
		iterations++;
		z = hare;
		lam++;
	}
	if (iterations < comps.b_max_i && iterations > comps.b_min_i)
		track_importance(fractal, c, comps, x, y);
} */

void	buddha_iteration_map(t_fractal *fractal, t_complex c, t_comps comps, double x, double y)
{
	t_complex	z;
	int			iterations;

	iterations = 0;
	z.x = 0.0;
	z.y = 0.0;
	while ((z.x * z.x) + (z.y * z.y) < comps.bound && iterations < comps.b_max_i)
	{
		z = sum_complex(comps.complex_f(z), c);
		iterations++;
	}
	if (iterations < comps.b_max_i && iterations > comps.b_min_i)
	track_importance(fractal, c, comps, x, y);
}


t_comps	set_comps(t_fractal *fractal, bool map)
{
	t_comps comps;
	
	comps.width = (double)fractal->width;
	comps.height = (double)fractal->height;
	comps.zoom = fractal->zoom;

	comps.x_cmin = -2;
	comps.x_cmax = 2;
	comps.y_cmin = 2;
	comps.y_cmax = -2;
	
	comps.x_span = (comps.x_cmax - comps.x_cmin);
	comps.y_span = (comps.y_cmax - comps.y_cmin);
	//make a ratio of y_span to xspan to use as aspect1, screen is aspect 2
	if (!map)
	{
		comps.n = fractal->buddha->n;
		//mapping to complex
		comps.slopex_to = comps.x_span / (comps.width);
		comps.slopey_to = comps.y_span / (comps.height);
		//stuff for subpix map during actual run
		comps.sn = comps.zoom * 2;//match with zoom
		comps.step = 1.0 / comps.sn;
		comps.nn = (int)(comps.sn);
		comps.ss = 3;// comps.ss² = samples per subpixel in submap
		/* comps.subpdf = malloc_matrix(comps.nn, comps.nn);
		if (!comps.subpdf)
			clear_all(fractal); */
	}
	else
	{
		comps.n = fractal->buddha->map_n;
		//mapping to complex
		comps.slopex_to = comps.x_span / (comps.width * comps.n);
		comps.slopey_to = comps.y_span / (comps.height * comps.n);
	}
	//mapping back
	//slope = (new_max - new_min) / (old_max - old_min)
	comps.slopex_back = (comps.width - 1.0) / comps.x_span;
	comps.slopey_back = (comps.height - 1.0) / comps.y_span;

	comps.hist = fractal->hist_num;
	comps.samples = (double)fractal->size * SQ(comps.n);
	
	comps.move_x = fractal->move_x;
	comps.move_y = fractal->move_y;
	comps.inv_zoom = 1.0f / comps.zoom;

	comps.complex_f = fractal->complex_f;
	comps.density = fractal->densities[comps.hist];
	comps.pdf = fractal->pdf;
	comps.b_max_i = fractal->b_max_i;
	comps.b_min_i = fractal->b_min_i;
	comps.bound = fractal->bound;

	return (comps);
}

//grid mapping is used here as the fastest most accurate method to assign avg pixel importance

void	*buddha_set_map(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;
	t_complex	c;
	t_comps		comps;
	
	double		x;
	double		y;
	
	piece = (t_piece *)arg;
	fractal = piece->fractal;
	Xoro128		*rng = &piece->rng;
	
	comps = set_comps(fractal, true);
	y = (double)piece->y_s - 1;
	while (++y < piece->y_e)
	{
		x = (double)piece->x_s - 1;
		while (++x < piece->x_e)
		{
		
			c.x = map_2(x + .5, comps.x_cmin, comps.slopex_to);
			c.y = map_2(y + .5, comps.y_cmin, comps.slopey_to);
			buddha_iteration_map(fractal, c, comps, x, y);
			
			//trying the within window map
			/* c.x = map_2(x, comps.x_cmin, comps.slopex_to) * comps.inv_zoom + comps.move_x;
			c.y = map_2(y, comps.y_cmin, comps.slopey_to) * comps.inv_zoom - comps.move_y;
			buddha_iteration_map(fractal, c, comps, x, y);	 */
		}
	}
	pthread_exit(NULL);
}
