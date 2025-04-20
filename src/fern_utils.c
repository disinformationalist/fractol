#include "fractol.h"

void	density_color_pixel(t_fractal *fractal, int x, int y, double highest)
{
	unsigned int	color;
	double			log_density;

	log_density = log(fractal->fdensity[y][x]);

	color = calc_color_4(log_density, log(highest), &fractal->colors);//Prpb is here.. out of bounds color?..
	
	
	put_pixel(x, y, fractal, color);
}

void	density_color(t_fractal *fractal)
{
	int		x;
	int		y;
	double	highest;
	
	highest = high_hit_countf(fractal->width, fractal->height, fractal->fdensity);
	y = -1;
	while (++y < fractal->height)
	{
		x = -1;
		while (++x < fractal->width)
		{
			if (fractal->fdensity[y][x] > 0)
				density_color_pixel(fractal, x, y, highest);
		}
	}
}

void	free_fern(double **vals, int j)
{
	while (--j >= 0)
	{
		free (vals[j]);
		vals[j] = NULL;
	}
	free(vals);
}

double	**alloc_fern(t_fractal *fractal, double **vals)
{
	int		i;

	vals = (double **)malloc(4 * sizeof(double *));
	if (!vals)
		close_handler(fractal);
	i = -1;
	while (++i < 4)
	{
		vals[i] = (double *)malloc(7 * sizeof(double));
		if (!vals[i])
		{
			free_fern(vals, i);
			close_handler(fractal);
		}
	}
	set_fern_species(fractal->species, vals);
	return (vals);
}

void	set_fern_species(int species, double **values)//maybe also adjustment for each value in realtime
{	
	if (species == 0)//fern classic (black speenwort)
	{
		//---f1------
		values[0][0] = 0;
		values[0][1] = 0;
		values[0][2] = 0;
		values[0][3] = .16;
		values[0][4] = 0;
		values[0][5] = 0;
		values[0][6] = 0.01;
		//---f2------
		values[1][0] = .85;
		values[1][1] = .04;
		values[1][2] = -.04;
		values[1][3] = .85;
		values[1][4] = 0;
		values[1][5] = 1.60;
		values[1][6] = 0.85;
		//---f3------
		values[2][0] = 0.2; 
		values[2][1] = -.26;
		values[2][2] = .23;
		values[2][3] = .22;
		values[2][4] = 0;
		values[2][5] = 1.60;
		values[2][6] = 0.07;
		//---f4------
		values[3][0] = -.15;
		values[3][1] = .28;
		values[3][2] = .26;
		values[3][3] = .24;
		values[3][4] = 0;
		values[3][5] = .44;
		values[3][6] = 0.07;
	}
	else if (species == 1) //(thelypteridaceae)
	{
		//---f1------
		values[0][0] = 0;
		values[0][1] = 0;
		values[0][2] = 0;
		values[0][3] = .25;
		values[0][4] = 0;
		values[0][5] = -0.4;
		values[0][6] = 0.02;
		//---f2------
		values[1][0] = .95;
		values[1][1] = .005;
		values[1][2] = -.005;
		values[1][3] = .93;
		values[1][4] = -0.002;
		values[1][5] = .5;
		values[1][6] = 0.84;
		//---f3------
		values[2][0] = 0.035;
		values[2][1] = -0.2;
		values[2][2] = 0.16;
		values[2][3] = 0.04;
		values[2][4] = -0.09;
		values[2][5] = 0.02;
		values[2][6] = 0.07;
		//---f4------
		values[3][0] = -0.04;
		values[3][1] = 0.2;
		values[3][2] = 0.16;
		values[3][3] = 0.04;
		values[3][4] = 0.083;
		values[3][5] = 0.12;
		values[3][6] = 0.07;
	}
}

///----------fern density utils

void	init_fdensity(t_fractal *fractal)
{
	int	i;

	fractal->fdensity = (int **)malloc(fractal->height * sizeof(int *));
	if (fractal->fdensity == NULL)
		clear_all(fractal);
	i = -1;
	while (++i < fractal->height)
	{
		fractal->fdensity[i] = (int *)malloc(fractal->width * sizeof(int));
		if (fractal->fdensity[i] == NULL)
		{
			free_fdensity(fractal, i);
			clear_all(fractal);
		}
	}
	zero_fdensity(fractal);
}

void	zero_fdensity(t_fractal *fractal)
{
	int	i;
	int	j;

	j = -1;
	while (++j < fractal->height)
	{
		i = -1;
		while (++i < fractal->width)
			fractal->fdensity[j][i] = 0;
	}
}

double	high_hit_countf(long width, long height, int **density)
{
	int	i;
	int	j;
	int	high;

	j = -1;
	high = 0;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
		{
			if (high < density[j][i])
				high = density[j][i];
		}
	}
	return (high);
}

void	free_fdensity(t_fractal *fractal, int j)
{
	if (!fractal->fdensity)
		return ;
	while (--j >= 0)
	{
		free(fractal->fdensity[j]);
		fractal->fdensity[j] = NULL;
	}
	free(fractal->fdensity);
	fractal->fdensity = NULL;
}