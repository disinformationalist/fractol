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

static inline void set_width_height(t_fractal *fractal, int clamp, bool dir)
{
	if (dir)
	{
		if (fractal->s_kernel > clamp)
			fractal->s_kernel = clamp;
	}
	else
	{
		if (fractal->s_kernel < clamp)
			fractal->s_kernel = clamp;
	}
	if (fractal->supersample)
	{
		fractal->width = fractal->width_orig * fractal->s_kernel;
		fractal->height = fractal->height_orig * fractal->s_kernel;
	}
	printf("Supersample level: %d\n ", fractal->s_kernel);
}

//julia values adjust to mouse position

int	julia_handle(int x, int y, t_fractal *fractal)
{
	if (fractal->id == 2 && fractal->j_handle)
	{
		fractal->julia_x = map(x, -2, +2, fractal->width) \
		* fractal->zoom + fractal->move_x;
		fractal->julia_y = map(y, +2, -2, fractal->height) \
		* fractal->zoom - fractal->move_y;
		render(fractal);
	}
	return (0);
}

//WASD control direction of moving color source when on, Q recenter

int	key_handler_3(int keysym, t_fractal *fractal)
{
	if (keysym == W)
		fractal->move_col_y -= .5;
	else if (keysym == S)
		fractal->move_col_y += .5;
	else if (keysym == A)
		fractal->move_col_x -= .5;
	else if (keysym == D)
		fractal->move_col_x += .5;
	else if (keysym == Q)
	{
		fractal->move_col_y = 0;
		fractal->move_col_x = 0;
	}
	else if (keysym == ALT)
		fractal->zoom_iter = !fractal->zoom_iter;
	else if (keysym == COMMA)
		fractal->species = !fractal->species;
	else
		supersample_handle(keysym, fractal);
	return (0);
}

int	supersample_handle(int keysym, t_fractal *fractal)
{
	if (keysym == SPACE)
	{
		if (fractal->id == 3)
			free_matrices(fractal);
		if (fractal->supersample)
		{
			fractal->width = fractal->width_orig;
			fractal->height = fractal->height_orig;
		}
		else
		{
			fractal->width *= fractal->s_kernel;
			fractal->height *= fractal->s_kernel;
		}
		fractal->supersample = !fractal->supersample;
		if (fractal->id == 3)
			init_matricies(fractal);
	}
	return (0);
}

int	key_handler_2(int keysym, t_fractal *fractal)
{
	if (keysym == XK_Left)
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
	else if (keysym == NUM_DOWN)
		fractal->julia_y -= .005 * (fractal->zoom + fractal->move_x);
	else if (keysym == NUM_UP)
		fractal->julia_y += .005 * (fractal->zoom - fractal->move_x);
	else if (keysym == NUM_LEFT)
		fractal->julia_x -= .005 * (fractal->zoom + fractal->move_y);
	else if (keysym == NUM_RIGHT)
		fractal->julia_x += .005 * (fractal->zoom - fractal->move_y);
	else if (keysym == N_1)
		fractal->color_spectrum = !fractal->color_spectrum;
	else if (keysym == NUM_ZERO)
	{
		fractal->zoom = 1;
		fractal->max_i = 80;
		fractal->move_x = 0;
		fractal->move_y = 0;
	}
	else if (keysym == PAD_PLUS)
	{
		fractal->s_kernel += 2;
		set_width_height(fractal, 19, true);
	}
	else if (keysym == PAD_MINUS)
	{
		fractal->s_kernel -= 2;
		set_width_height(fractal, 3, false);
	}
	else
		key_handler_3(keysym, fractal);
	render(fractal);
	return (0);
}

int	key_handler_2layer(int keysym, t_fractal *fractal)
{
	if (keysym == XK_Left)
		fractal->move_x -= (0.5 * fractal->zoom);
	else if (keysym == XK_Right)
		fractal->move_x += (0.5 * fractal->zoom);
	else if (keysym == XK_Up)
		fractal->move_y -= (0.5 * fractal->zoom);
	else if (keysym == XK_Down)
		fractal->move_y += (0.5 * fractal->zoom);
	else if (keysym == PLUS)
		fractal->max_i += 500;
	else if (keysym == MINUS)
		fractal->max_i -= 500;
	else if (keysym == NUM_DOWN)//all below avail
		fractal->julia_y -= .005 * (fractal->zoom + fractal->move_x);
	else if (keysym == NUM_UP)
		fractal->julia_y += .005 * (fractal->zoom - fractal->move_x);
	else if (keysym == NUM_LEFT)
		fractal->julia_x -= .005 * (fractal->zoom + fractal->move_y);
	else if (keysym == NUM_RIGHT)
		fractal->julia_x += .005 * (fractal->zoom - fractal->move_y);
	else if (keysym == N_1)
		fractal->color_spectrum = !fractal->color_spectrum;
	else if (keysym == PAD_PLUS)
	{
		fractal->s_kernel += 2;
		set_width_height(fractal, 19, true);
	}
	else if (keysym == PAD_MINUS)
	{
		fractal->s_kernel -= 2;
		set_width_height(fractal, 3, false);
	}
	else if (keysym == NUM_ZERO)
	{
		fractal->zoom = 1;
		fractal->max_i = 80;
		fractal->move_x = 0;
		fractal->move_y = 0;
	}
	else
		key_handler_3(keysym, fractal);
	render(fractal);
	return (0);
}

//export the current img to png and save in root directory

void	export(int keysym, t_fractal *fractal)
{
	char		*name = NULL;
	png_text	*text;

	if (fractal->id == 1)
		name = get_nxt_name("mandelbrot_");
	else if (fractal->id == 2)
		name = get_nxt_name("julia_");
	else if (fractal->id == 3)
		name = get_nxt_name("buddha_");
	else if (fractal->id == 4)
		name = get_nxt_name("fern_");
	if (!name)
		close_handler(fractal);
	//text = build_fractal_text(fractal); //todo, ft to store data in image to open at the same settings
	text = NULL;//temp until function is done;
	if (export_png(name, &fractal->img, fractal->width_orig, fractal->height_orig, text) == -1)
		close_handler(fractal);
	ft_putstr_color("EXPORT COMPLETE\n", BOLD_BRIGHT_BLUE);
	if (name)
		free(name);
}

int	key_handler(int keysym, t_fractal *fractal)
{
	//printf("%d\n", keysym);
	if (keysym == XK_Escape)
		close_handler(fractal);
	else if (keysym == XK_Tab)
		fractal->layer = !fractal->layer;
	if (fractal->id == 3)
	{
		buddha_handler(keysym, fractal);
		return (0); 
	}
	else if (keysym == F2)
		print_mj_guide();
	else if (keysym == XK_2)
	{
		fractal->toggle_color = !fractal->toggle_color;
		fractal->col_i = (fractal->col_i - 7 + 720) % 720;
	}
	else if (keysym == LFT_STRG)
		fractal->j_handle = !fractal->j_handle;
	else if (keysym == F3)
		export(keysym, fractal);
	else if (keysym == RGHT_STRG)
		fractal->mouse_zoom = !fractal->mouse_zoom;
	else if (fractal->layer)
		key_handler_2layer(keysym, fractal);
	else
		key_handler_2(keysym, fractal);
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
		mouse_x = map(x, -2, +2, fractal->width);
		mouse_y = map(y, +2, -2, fractal->height);

		//next section performs mouse zoom as a move in x and y, this way is much more optimized

		//original for clarity
		/* fractal->move_x = mouse_x * old_zoom - mouse_x * fractal->zoom + fractal->move_x;
		fractal->move_y = -mouse_y * old_zoom  + mouse_y * fractal->zoom + fractal->move_y; */
		
		//opti	
		fractal->move_x = mouse_x * (old_zoom - fractal->zoom) + fractal->move_x;
		fractal->move_y = mouse_y * (fractal->zoom - old_zoom) + fractal->move_y;
	}
}

int	mouse_handler(int button, int x, int y, t_fractal *fractal)
{
	if (fractal->supersample)
	{
		x *= fractal->s_kernel;
		y *= fractal->s_kernel;
	}
	if (button == 1)
		fractal->move_x = 0;
	else if (button == 3)
		fractal->move_y = 0;
	else
		mouse_handler_2(button, x, y, fractal);
	render(fractal);
	return (0);
}