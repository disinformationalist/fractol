/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   buddhabrot.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:16:43 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:16:46 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	ft_round(double value)
{
	return ((int)floor(value + 0.5));
}

static t_complex	square_step(t_complex z, t_complex c)
{
	t_complex	next;

	next = square_complex(z);
	next.x += c.x;
	next.y += c.y;
	return (next);
}

static bool	known_square_interior(t_complex c, t_comps comps)
{
	double	x;
	double	y2;
	double	q;

	if (!comps.square_formula || !comps.interior_rejection
		|| comps.bound < 4.0)
		return (false);
	x = c.x - 0.25;
	y2 = c.y * c.y;
	q = x * x + y2;
	if (q * (q + x) < 0.25 * y2)
		return (true);
	x = c.x + 1.0;
	return (x * x + y2 < 0.0625);
}

int	buddha_escape_length(t_complex c, t_comps comps, int max_iterations)
{
	t_complex	z;
	int			iterations;

	if (known_square_interior(c, comps))
		return (max_iterations);
	iterations = 0;
	z = (t_complex){0.0, 0.0};
	if (comps.square_formula)
	{
		while (z.x * z.x + z.y * z.y < comps.bound
			&& iterations < max_iterations)
		{
			z = square_step(z, c);
			iterations++;
		}
	}
	else
	{
		while (z.x * z.x + z.y * z.y < comps.bound
			&& iterations < max_iterations)
		{
			z = sum_complex(comps.complex_f(z), c);
			iterations++;
		}
	}
	return (iterations);
}

int	buddha_escape_orbit(t_complex c, t_comps comps, int max_iterations,
		t_complex *orbit, int capacity)
{
	t_complex	z;
	int			iterations;

	if (!orbit || capacity <= 0 || !comps.square_formula)
		return (buddha_escape_length(c, comps, max_iterations));
	if (known_square_interior(c, comps))
		return (max_iterations);
	z = (t_complex){0.0, 0.0};
	iterations = 0;
	while (z.x * z.x + z.y * z.y < comps.bound
		&& iterations < max_iterations)
	{
		z = square_step(z, c);
		if (iterations < capacity)
			orbit[iterations] = z;
		iterations++;
	}
	return (iterations);
}

static bool	project_orbit(t_complex z, t_comps comps,
		int *screen_x, int *screen_y)
{
	*screen_y = ft_round(map_back_2(
				(z.x - comps.move_x) * comps.zoom,
				comps.x_cmin, comps.slopex_back));
	*screen_x = ft_round(map_back_2(
				(z.y + comps.move_y) * comps.zoom,
				comps.y_cmin, comps.slopey_back));
	return (*screen_x >= 0 && *screen_x < comps.width
		&& *screen_y >= 0 && *screen_y < comps.height);
}

int	buddha_visible_hits(t_complex c, t_comps comps, int orbit_length)
{
	t_complex	z;
	int			screen_x;
	int			screen_y;
	int			iteration;
	int			hits;

	z = (t_complex){0.0, 0.0};
	iteration = 0;
	hits = 0;
	while (iteration++ < orbit_length)
	{
		if (comps.square_formula)
			z = square_step(z, c);
		else
			z = sum_complex(comps.complex_f(z), c);
		if (project_orbit(z, comps, &screen_x, &screen_y))
			hits++;
	}
	return (hits);
}

int	buddha_cached_visible_hits(const t_complex *orbit, t_comps comps,
		int orbit_length)
{
	int	screen_x;
	int	screen_y;
	int	iteration;
	int	hits;

	iteration = -1;
	hits = 0;
	while (++iteration < orbit_length)
		if (project_orbit(orbit[iteration], comps, &screen_x, &screen_y))
			hits++;
	return (hits);
}

static void	footprint_grid(t_comps comps, int grid[2])
{
	int	long_axis;

	long_axis = comps.width;
	if (comps.height > long_axis)
		long_axis = comps.height;
	grid[0] = (comps.width * BUDDHA_SCORE_TILE_AXIS + long_axis - 1)
		/ long_axis;
	grid[1] = (comps.height * BUDDHA_SCORE_TILE_AXIS + long_axis - 1)
		/ long_axis;
	if (grid[0] < 1)
		grid[0] = 1;
	if (grid[1] < 1)
		grid[1] = 1;
}

static void	footprint_begin(t_buddha_score_scratch *scratch)
{
	scratch->generation++;
	if (scratch->generation == 0)
	{
		memset(scratch->stamp, 0, sizeof(scratch->stamp));
		scratch->generation = 1;
	}
}

static void	footprint_add(t_complex z, t_comps comps, int grid[2],
		t_buddha_score_scratch *scratch, int *hits, long double *sum_squares)
{
	size_t		index;
	uint32_t	previous;
	int			screen_x;
	int			screen_y;

	if (!project_orbit(z, comps, &screen_x, &screen_y))
		return ;
	(*hits)++;
	index = (size_t)(screen_y * grid[1] / comps.height)
		* BUDDHA_SCORE_TILE_AXIS
		+ (size_t)(screen_x * grid[0] / comps.width);
	previous = 0;
	if (scratch->stamp[index] == scratch->generation)
		previous = scratch->visits[index];
	else
		scratch->stamp[index] = scratch->generation;
	scratch->visits[index] = previous + 1;
	*sum_squares += 2.0L * (long double)previous + 1.0L;
}

double	buddha_visible_footprint(t_complex c, t_comps comps,
		int orbit_length, t_buddha_score_scratch *scratch, int *hits)
{
	t_complex	z;
	long double	sum_squares;
	int			grid[2];
	int			iteration;

	footprint_grid(comps, grid);
	footprint_begin(scratch);
	z = (t_complex){0.0, 0.0};
	sum_squares = 0.0L;
	*hits = 0;
	iteration = 0;
	while (iteration++ < orbit_length)
	{
		if (comps.square_formula)
			z = square_step(z, c);
		else
			z = sum_complex(comps.complex_f(z), c);
		footprint_add(z, comps, grid, scratch, hits, &sum_squares);
	}
	return (sqrt((double)sum_squares));
}

double	buddha_cached_visible_footprint(const t_complex *orbit,
		t_comps comps, int orbit_length, t_buddha_score_scratch *scratch,
		int *hits)
{
	long double	sum_squares;
	int			grid[2];
	int			iteration;

	footprint_grid(comps, grid);
	footprint_begin(scratch);
	sum_squares = 0.0L;
	*hits = 0;
	iteration = -1;
	while (++iteration < orbit_length)
		footprint_add(orbit[iteration], comps, grid, scratch, hits,
			&sum_squares);
	return (sqrt((double)sum_squares));
}

static void	track_orbit(t_complex c, double weight, t_comps comps,
		int orbit_length)
{
	t_complex	z;
	size_t		index;
	int			screen_x;
	int			screen_y;
	int			iteration;

	z = (t_complex){0.0, 0.0};
	iteration = 0;
	while (iteration++ < orbit_length)
	{
		if (comps.square_formula)
			z = square_step(z, c);
		else
			z = sum_complex(comps.complex_f(z), c);
		if (project_orbit(z, comps, &screen_x, &screen_y))
		{
			index = (size_t)screen_y * (size_t)comps.width
				+ (size_t)screen_x;
			comps.density_flat[index] += weight;
		}
	}
}

static void	track_cached_orbit(const t_complex *orbit, double weight,
		t_comps comps, int orbit_length)
{
	size_t	index;
	int		screen_x;
	int		screen_y;
	int		iteration;

	iteration = -1;
	while (++iteration < orbit_length)
	{
		if (project_orbit(orbit[iteration], comps, &screen_x, &screen_y))
		{
			index = (size_t)screen_y * (size_t)comps.width
				+ (size_t)screen_x;
			comps.density_flat[index] += weight;
		}
	}
}

void	buddha_iteration(t_complex c, double weight, t_comps comps,
		t_complex *orbit_cache)
{
	int	iterations;

	if (orbit_cache && comps.square_formula)
		iterations = buddha_escape_orbit(c, comps, comps.b_max_i,
				orbit_cache, BUDDHA_ORBIT_CACHE_POINTS);
	else
		iterations = buddha_escape_length(c, comps, comps.b_max_i);
	if (iterations < comps.b_max_i && iterations > comps.b_min_i)
	{
		if (orbit_cache && comps.square_formula
			&& iterations <= BUDDHA_ORBIT_CACHE_POINTS)
			track_cached_orbit(orbit_cache, weight, comps, iterations);
		else
			track_orbit(c, weight, comps, iterations);
	}
}

void	*buddha_set(void *arg)
{
	t_fractal	*fractal;
	t_piece		*piece;
	t_complex	c;
	t_comps		comps;
	size_t		samples;
	size_t		sample;
	Xoro128		*rng;
	t_complex	orbit_cache[BUDDHA_ORBIT_CACHE_POINTS];
	t_complex	*cache;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	comps = set_comps(fractal, false);
	samples = (size_t)llround(comps.samples / (double)fractal->buffs
			/ (double)fractal->worker_histogram_count);
	rng = &piece->rng;
	comps.density_flat = fractal->worker_histograms[piece->id];
	cache = NULL;
	if (fractal->buddha->orbit_cache && comps.square_formula
		&& comps.b_max_i <= BUDDHA_ORBIT_CACHE_POINTS)
		cache = orbit_cache;
	sample = 0;
	while (sample++ < samples)
	{
		c.x = (xoro128d(rng) * comps.sample_x_span + comps.sample_x_min)
			* comps.inv_zoom + comps.move_x;
		c.y = (xoro128d(rng) * -comps.sample_y_span - comps.sample_y_min)
			* comps.inv_zoom - comps.move_y;
		buddha_iteration(c, 1.0, comps, cache);
	}
	return (NULL);
}
