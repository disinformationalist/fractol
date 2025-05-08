/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:10:20 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:10:22 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	set_type(t_fractal *f, int n)
{
	if (n < 0 || n > 3)
	{
		putstr_fd("Buddha syntax: ./fractal buddha <width> <height> <type>\n", STDERR_FILENO);
		putstr_fd("type must be a 0, 1, 2, or 3\n\n", STDERR_FILENO);
		putstr_fd("0: standard buddha\n", STDERR_FILENO);
		putstr_fd("1: another version of buddha with custom settings\n", STDERR_FILENO);
		putstr_fd("2: 'space lotus' using the buddha method with the complex conjugate of buddha/mandelbrot \n", STDERR_FILENO);
		putstr_fd("3: 'pheonix' buddha method with the complex cube and absolute value"
			"(like burning ship but using cube instead of square)\n", STDERR_FILENO);
		exit(EXIT_FAILURE);
	}
	f->buddha->type = n;
}

void	check_width_height(t_fractal *frac)
{
	/* if (frac->width < 10 || frac->width > 10000)
	{
		putstr_fd("Width must be greater than 10 and less than 10000\n", STDERR_FILENO);
		exit(EXIT_FAILURE);
	} */
	/* else if (frac->height < 10 || frac->height > 10000)
	{
		putstr_fd("Width must be greater than 10 and less than 10000\n", STDERR_FILENO);
		exit(EXIT_FAILURE);
	} */
	if (frac->id == 3 && frac->width != frac->height)
	{
		putstr_fd("Width and height must be equal for buddhabrot\n", STDERR_FILENO);
		exit(EXIT_FAILURE);
	}
}

void	check_and_set_id(t_fractal *fractal, int ac, char **av)
{
	int n;

	if (ac == 4 && !ft_strncmp(av[1], "mandelbrot", 10))
		fractal->id = 1;
	else if (ac == 6 && !ft_strncmp(av[1], "julia", 5))
	{
		fractal->julia_x = atof(av[2]);
		fractal->julia_y = atof(av[3]);
		fractal->width = ft_atoi(av[4]);
		fractal->height = ft_atoi(av[5]);
		fractal->id = 2;	
	}
	else if (ac == 5 && !ft_strncmp(av[1], "buddha", 6))
	{
		fractal->id = 3;
		fractal->buddha = (t_buddha *)malloc(sizeof(t_buddha));
		if (!fractal->buddha)
		{
			printf("Error: Buddha Malloc failed\n");
			exit (EXIT_FAILURE);
		}
		n = ft_atoi(av[4]);
		set_type(fractal, n);
	}
	else if (ac == 4 && !ft_strncmp(av[1], "fern", 4))
		fractal->id = 4;
	else
	{
		putstr_fd(ERROR_MESSAGE, STDERR_FILENO);
		exit(EXIT_FAILURE);
	}
	fractal->name = av[1];
	fractal->name[0] -= 32;
	if (fractal->id == 1 || fractal->id == 3 || fractal->id == 4)
	{
		fractal->width = ft_atoi(av[2]);
		fractal->height = ft_atoi(av[3]);
	}
	check_width_height(fractal);
	fractal->width_orig = fractal->width;
	fractal->height_orig = fractal->height;
}

int	main(int ac, char **av)
{
	t_fractal	fractal;
	
	fractal.buddha = NULL;
	
	check_and_set_id(&fractal, ac, av);
	//set_least_diff_pair(get_num_cores(), &fractal.num_rows, &fractal.num_cols);
	fractal.num_cols = 1;
	fractal.num_rows = get_num_cores();
	fractal.threads = (pthread_t *)malloc(fractal.num_rows * fractal.num_cols * sizeof(pthread_t));
	if (!fractal.threads)
	{
		if (fractal.buddha)
			free(fractal.buddha);
		printf("Error: Thread Malloc failed\n");
		return (EXIT_FAILURE);
	}
	fractal_init(&fractal);
	render(&fractal);
	mlx_loop(fractal.mlx_connect);
}
