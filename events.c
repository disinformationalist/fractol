/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   events.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:09:07 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:09:10 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	julia_handle(int x, int y, t_fractal *fractal)
{
	if (!ft_strncmp(fractal->name, "julia", 5) && fractal->j_handle)
	{
		fractal->julia_x = map(x, -2, +2, WIDTH) \
		* fractal->zoom + fractal->move_x;
		fractal->julia_y = map(y, +2, -2, HEIGHT) \
		* fractal->zoom - fractal->move_y;
		render(fractal);
	}
	return (0);
}

int	key_handler_3(int keysym, t_fractal *fractal)
{
	if (keysym == 119)
		fractal->move_col_y -= .5;
	else if (keysym == 115)
		fractal->move_col_y += .5;
	else if (keysym == 97)
		fractal->move_col_x -= .5;
	else if (keysym == 100)
		fractal->move_col_x += .5;
	else if (keysym == 113)
	{
		fractal->move_col_y = 0;
		fractal->move_col_x = 0;
	}
	else if (keysym == ALT)
		fractal->zoom_iter = !fractal->zoom_iter;
	return (0);
}

int	key_handler_2(int keysym, t_fractal *fractal)
{
	if (keysym == NUM_DOWN)
		fractal->julia_y -= .005 * (fractal->zoom + fractal->move_x);
	else if (keysym == NUM_UP)
		fractal->julia_y += .005 * (fractal->zoom - fractal->move_x);
	else if (keysym == NUM_LEFT)
		fractal->julia_x -= .005 * (fractal->zoom + fractal->move_y);
	else if (keysym == NUM_RIGHT)
		fractal->julia_x += .005 * (fractal->zoom - fractal->move_y);
	else if (keysym == RGHT_STRG)
		fractal->mouse_zoom = !fractal->mouse_zoom;
	else if (keysym == 49)
		fractal->color_spectrum = !fractal->color_spectrum;
	else if (keysym == NUM_ZERO)
	{
		fractal->zoom = 1;
		fractal->max_i = 80;
		fractal->move_x = 0;
		fractal->move_y = 0;
	}
	else
		key_handler_3(keysym, fractal);
	return (0);
}

int	key_handler(int keysym, t_fractal *fractal)
{
	if (keysym == XK_Escape)
		close_handler(fractal);
	else if (keysym == XK_Left)
		fractal->move_x -= (0.5 * fractal->zoom);
	else if (keysym == XK_Right)
		fractal->move_x += (0.5 * fractal->zoom);
	else if (keysym == XK_Up)
		fractal->move_y -= (0.5 * fractal->zoom);
	else if (keysym == XK_Down)
		fractal->move_y += (0.5 * fractal->zoom);
	else if (keysym == PLUS)
		fractal->max_i += 10;
	else if (keysym == MINUS)
		fractal->max_i -= 10;
	else if (keysym == 32)
	{
		fractal->toggle_color = !fractal->toggle_color;
		fractal->col_i = (fractal->col_i - 7 + 360) % 360;
	}
	else if (keysym == LFT_STRG)
		fractal->j_handle = !fractal->j_handle;
	else
		key_handler_2(keysym, fractal);
	render(fractal);
	return (0);
}


void mouse_zoom_iters(int button, t_fractal *fractal)
{
	if (button == 5)
	{
		fractal->zoom *= 1.1;
		if (fractal->zoom_iter)
			fractal->max_i -= 2;
	}
	else if (button == 4)
	{
		fractal->zoom /= 1.1;
		if (fractal->zoom_iter)
			fractal->max_i += 2;
	}
}

void	mouse_handler_2(int button, int x, int y, t_fractal *fractal)
{
	double	mouse_x;
	double	mouse_y;
	double	old_zoom;
	
	if (!fractal->mouse_zoom)
		mouse_zoom_iters(button, fractal);
	else
	{
		old_zoom = fractal->zoom;
		mouse_zoom_iters(button, fractal);
		mouse_x = map(x, -2, +2, WIDTH);
		mouse_y = map(y, +2, -2, HEIGHT);

		//original for clarity
		//fractal->move_x = mouse_x * old_zoom - mouse_x * fractal->zoom + fractal->move_x;
		//fractal->move_y = -mouse_y * old_zoom  + mouse_y * fractal->zoom + fractal->move_y; 
		//opti	
		fractal->move_x = mouse_x * (old_zoom - fractal->zoom) + fractal->move_x;
		fractal->move_y = mouse_y * (fractal->zoom - old_zoom) + fractal->move_y;
	}
}

int	mouse_handler(int button, int x, int y, t_fractal *fractal)
{
	if (button == 1)
		fractal->move_x = 0;
	else if (button == 3)
		fractal->move_y = 0;
	else
		mouse_handler_2(button, x, y, fractal);
	render(fractal);
	return (0);
}

/* int	mouse_handler(int button, int x, int y, t_fractal *fractal)
{
	double	mouse_x;
	double	mouse_y;

	if (button == 5)
	{
		fractal->zoom *= 1.1;
		if (fractal->zoom_iter)
			fractal->max_i -= 2;
	}
	else if (button == 4)
	{
		fractal->zoom /= 1.1;
		if (fractal->zoom_iter)
			fractal->max_i += 2;
	}
	if (fractal->mouse_zoom)
	{
		mouse_x = map(x, -2, +2, WIDTH) * fractal->zoom + fractal->move_x;
		mouse_y = map(y, +2, -2, HEIGHT) * fractal->zoom - fractal->move_y;
		fractal->move_x = mouse_x;
		fractal->move_y = -mouse_y;
	}
	render(fractal);
	return (0);
} */
