#include "fractol.h"
#include <errno.h>

#define BUDDHA_MIN_START_ZOOM 1e-12
#define BUDDHA_MAX_START_ZOOM 1e12

void	print_buddha_usage(int fd)
{
	putstr_fd("Buddha syntax:\n", fd);
	putstr_fd("./fractol buddha <width> <height> <type> "
		"[--importance-map] [--center <real> <imaginary>] "
		"[--zoom <scale>]\n", fd);
	putstr_fd("Options may appear in any order after <type>.\n", fd);
	putstr_fd("Zoom must be between 1e-12 and 1e12; "
		"values greater than 1 zoom in.\n", fd);
}

static int	parse_finite_double(char *text, double *number)
{
	char	*end;

	errno = 0;
	*number = strtod(text, &end);
	if (end == text || *end != '\0' || errno == ERANGE
		|| !isfinite(*number))
		return (-1);
	return (0);
}

static int	parse_center(t_buddha *buddha, int count, char **options,
		int *index)
{
	double	real;
	double	imaginary;

	if (buddha->view_center_set || *index + 2 >= count
		|| parse_finite_double(options[*index + 1], &real) != 0
		|| parse_finite_double(options[*index + 2], &imaginary) != 0)
		return (-1);
	buddha->view_center_set = true;
	buddha->view_center_real = real;
	buddha->view_center_imaginary = imaginary;
	*index += 3;
	return (0);
}

static int	parse_zoom(t_buddha *buddha, int count, char **options,
		int *index)
{
	double	zoom;

	if (buddha->view_zoom_set || *index + 1 >= count
		|| parse_finite_double(options[*index + 1], &zoom) != 0
		|| zoom < BUDDHA_MIN_START_ZOOM || zoom > BUDDHA_MAX_START_ZOOM)
		return (-1);
	buddha->view_zoom_set = true;
	buddha->view_zoom = zoom;
	*index += 2;
	return (0);
}

int	parse_buddha_options(t_buddha *buddha, int count, char **options)
{
	int	index;

	index = 0;
	while (index < count)
	{
		if (!strcmp(options[index], "--importance-map"))
		{
			if (buddha->importance_enabled)
				return (-1);
			buddha->importance_enabled = true;
			index++;
		}
		else if (!strcmp(options[index], "--center"))
		{
			if (parse_center(buddha, count, options, &index) != 0)
				return (-1);
		}
		else if (!strcmp(options[index], "--zoom"))
		{
			if (parse_zoom(buddha, count, options, &index) != 0)
				return (-1);
		}
		else
			return (-1);
	}
	return (0);
}

void	buddha_apply_start_view(t_fractal *fractal)
{
	t_buddha	*buddha;

	buddha = fractal->buddha;
	if (buddha->view_center_set)
	{
		fractal->move_x = buddha->view_center_real;
		fractal->move_y = -buddha->view_center_imaginary;
	}
	if (buddha->view_zoom_set)
		fractal->zoom = buddha->view_zoom;
}
