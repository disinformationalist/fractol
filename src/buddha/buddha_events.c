#include "fractol.h"

void	filter(t_fractal *fractal, t_buddha *b)
{
	if (b->ftype == MEAN)
	{
		if (!b->filter)
			matcpy(fractal->densities[b->fchan], fractal->pdf, fractal->width, fractal->height);
		else 
			matcpy(fractal->pdf, fractal->densities[b->fchan], fractal->width, fractal->height);
	}
	else
	{	
		if (b->filter)
		{
			img_cpy(&fractal->img_2, &fractal->img, fractal->width, fractal->height);
			if (b->ftype == ADJUST)
				adjust_pixels_rgb(&fractal->img, fractal->width, fractal->height, (double)((b->flevel - 1) >> 1) * .5 + 1, 0, RGB);
			else if (b->ftype == GAUSS)
				gaussian_convo_filter(fractal->mlx_connect, &fractal->img, fractal->width, fractal->height, 5, ((b->flevel - 1) >> 1) * .5);//lev cannot exceed 7....
		}
		else
			img_cpy(&fractal->img, &fractal->img_2, fractal->width, fractal->height);
	}
}

	//some other filters for playing with
	//adjust_pixels_rgb(&fractal->img, fractal->width, fractal->height, 1, -30, B);
	//mean_convo_filter(fractal->mlx_connect, &fractal->img, fractal->width, fractal->height, 3);
	//gaussian_convo_filter(fractal->mlx_connect, &fractal->img, fractal->width, fractal->height, 5, 1);
	//gamma_correct_rgb(&fractal->img, fractal->width, fractal->height, 1.4, RGB);
	//adjust_pixels_rgb(&fractal->img, fractal->width, fractal->height, 2, 10, B);
	//nlm_denoise(&fractal->img, fractal->width, fractal->height, 3, 5, 35);//p_size, win_size, h //needs rework + variance


void	change_flevel(int keysym, t_buddha *b, t_fractal *fractal)
{
	if (keysym == Z)
	{
		b->flevel += 2;
		if (b->flevel > 33)
			b->flevel = 33;
	}
	else
	{
		b->flevel -= 2;
		if (b->flevel < 3)
			b->flevel = 3;
	}
	if (b->filter && b->ftype == MEAN)
		matcpy(fractal->densities[b->fchan], fractal->pdf, fractal->width, fractal->height);
}

void	change_powers_layer2(int keysym, t_buddha *b)
{
	if (keysym == Q)
		b->bpow += .01;
	else if (keysym == A)
		b->bpow -= .01;
	else if (keysym == W)
		b->gpow += .01;
	else if (keysym == S)
		b->gpow -= .01;
	else if (keysym == E)
		b->rpow += .01;	
	else if (keysym == D)
		b->rpow -= .01;
}

void	change_powers(int keysym, t_buddha *b)
{
	if (keysym == Q)
		b->bpow += .05;
	else if (keysym == A)
		b->bpow -= .05;
	else if (keysym == W)
		b->gpow += .05;
	else if (keysym == S)
		b->gpow -= .05;
	else if (keysym == E)
		b->rpow += .05;	
	else if (keysym == D)
		b->rpow -= .05;
	else
		return ;
}

void	change_edges(int keysym, t_buddha *b)
{
	if (keysym == U)
		b->edge0_b += .05;
	else if (keysym == J)
		b->edge0_b -= .05;
	else if (keysym == I)
		b->edge0_g += .05;
	else if (keysym == K)
		b->edge0_g -= .05;
	else if (keysym == O)
		b->edge0_r += .05;	
	else if (keysym == L)
		b->edge0_r -= .05;
	else
		return ;
}

void	change_edges2(int keysym, t_buddha *b)
{
	if (keysym == U)
		b->edge1_b += .05;
	else if (keysym == J)
		b->edge1_b -= .05;
	else if (keysym == I)
		b->edge1_g += .05;
	else if (keysym == K)
		b->edge1_g -= .05;
	else if (keysym == O)
		b->edge1_r += .05;	
	else if (keysym == L)
		b->edge1_r -= .05;
	else
		return ;
}

void	change_filter(t_buddha *b)
{
	b->ftype++;
	if (b->ftype == 3)
		b->ftype = 0;
}

void buddha_handler(int keysym, t_fractal *fractal)
{
	t_buddha *b;

	b = fractal->buddha;
	if (!fractal->layer)
		change_powers(keysym, b);
	else
		change_powers_layer2(keysym, b);
	if (!fractal->layer)
		change_edges(keysym, b);
	else
		change_edges2(keysym, b);
	if (keysym == Z || keysym == H)
		change_flevel(keysym, b, fractal);
	if (keysym == F)
		change_filter(b);
	else if (keysym == RGHT_STRG)
	{
		b->filter = !b->filter;
		filter(fractal, b);
	}
	else if (keysym == SPACE)
	{
		supersample_handle(keysym, fractal);
		render(fractal);
		return ;
	}
	else if (keysym == N_0)
	{
		print_buddha_vals(b, fractal);
		return ;
	}
	else if (keysym == N_9)
		b->smootherstep = !b->smootherstep;
	if (!fractal->supersample)
	{
		ft_memset(fractal->img.pixels_ptr, 0, fractal->width_orig * fractal->height_orig * (fractal->img.bpp / 8));	
		color_buddha(fractal);
		if (b->filter && b->ftype != MEAN)
			filter(fractal, b);
		mlx_put_image_to_window(fractal->mlx_connect,
			fractal->mlx_win, fractal->img.img_ptr, 0, 0);
	}
	if (keysym == F2)
			print_buddha_guide();
	else if (keysym == F3)
			export(keysym, fractal);
	else
		return ;	
}