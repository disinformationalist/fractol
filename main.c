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

void	thread_error(t_fractal *fractal, int num_threads)
{
	int	i;

	i = 0;
	while (i < num_threads)
	{
		if (fractal->threads[i])
		{
			if (pthread_cancel(fractal->threads[i]) != 0)
				break ;
		}
	}
	pthread_mutex_destroy(&fractal->mutex);
	clear_all(fractal);
}

int	close_handler(t_fractal *fractal)
{
	pthread_mutex_destroy(&fractal->mutex);
	mlx_destroy_image(fractal->mlx_connect, fractal->img.img_ptr);
	mlx_destroy_image(fractal->mlx_connect, fractal->img_new.img_ptr);
	mlx_destroy_window(fractal->mlx_connect, fractal->mlx_win);
	mlx_destroy_display(fractal->mlx_connect);
	free_density(fractal, WIDTH);
	free(fractal->co->colors);
	fractal->co->colors = NULL;
	free(fractal->co);
	fractal->co = NULL;
	free(fractal->mlx_connect);
	exit(EXIT_SUCCESS);
	return (0);
}

int	main(int ac, char **av)
{
	t_fractal	fractal;

	if ((ac == 2 && !ft_strncmp(av[1], "mandelbrot", 10))
		|| (ac == 4 && !ft_strncmp(av[1], "julia", 5))
		|| (ac == 2 && !ft_strncmp(av[1], "buddha", 6)))
	{
		fractal.name = av[1];
		if (av[2] && av[3])
		{
			fractal.julia_x = atof(av[2]);
			fractal.julia_y = atof(av[3]);
		}
		fractal_init(&fractal);
		render(&fractal);
		mlx_loop(fractal.mlx_connect);
	}
	else
	{
		putstr_fd(ERROR_MESSAGE, STDERR_FILENO);
		exit(EXIT_FAILURE);
	}
}
