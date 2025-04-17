#include "fractol.h"

char	*get_type(t_buddha *b)//totest.. and test enter at main
{
	char	*str;

	if (b->type == BUDDHA1)
		str = GREEN"------------- STANDARD BUDDHA STATS -------------"RESET;
	else if (b->type == BUDDHA2)
		str = MAGENTA"-------------- CUSTOM BUDDHA STATS ------------"RESET;
	else if (b->type == LOTUS)
		str = BLUE"----------------- LOTUS STATS -----------------"RESET;
	else //(b->type == PHEONIX)
		str = RED"--------------- PHEONIX STATS -----------------"RESET;
	return (str);
}

void	print_buddha_vals(t_buddha *buddha, t_fractal *fractal)
{
	char *s;

	printf("\n%s\n\n", get_type(buddha));
	printf(BLUE"Blue power value : %f\n"RESET, buddha->bpow);
	printf(GREEN"Green power value: %f\n"RESET, buddha->gpow);
	printf(RED"Red power value  : %f\n\n"RESET, buddha->rpow);

	printf("Complex plane coordinates of center:  x: %f  i: %f\n", fractal->move_x, -fractal->move_y);
	printf("Zoom factor: %f \n\n", fractal->zoom);

	printf(BLUE"Blue "RESET"channel iterations\n");
	printf("Min: %d\n", buddha->min1);
	printf("Max: %d\n\n", buddha->max1);

	printf(GREEN"Green "RESET"channel iterations\n");
	printf("Min: %d\n", buddha->min2);
	printf("Max: %d\n\n", buddha->max2);

	printf(RED"Red "RESET"channel iterations\n");
	printf("Min: %d\n", buddha->min3);
	printf("Max: %d\n\n", buddha->max3);

	if (buddha->filter)
		s = GREEN"ON"RESET;
	else
		s = RED"OFF"RESET;

	printf("Averaging filter: %s  channel: %d  level: %d\n", s, buddha->fchan, buddha->flevel);
}

void	toggle_filter(t_fractal *fractal, t_buddha *b)
{
	b->filter = !b->filter;
	if (!b->filter)
		matcpy(fractal->densities[b->fchan], fractal->pdf, fractal->width, fractal->height);
	else 
		matcpy(fractal->pdf, fractal->densities[b->fchan], fractal->width, fractal->height);
}

void	change_flevel(int keysym, t_buddha *b, t_fractal *fractal)
{
	if (keysym == U)
	{
		b->flevel += 2;
		if (b->flevel > 21)
			b->flevel = 21;
	}
	else
	{
		b->flevel -= 2;
		if (b->flevel < 3)
			b->flevel = 3;
	}
	if (b->filter)
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
}

void buddha_handler(int keysym, t_fractal *fractal)//todo fchan change
{
	t_buddha *b;

	b = fractal->buddha;
	if (!fractal->layer)
		change_powers(keysym, b);
	else
		change_powers_layer2(keysym, b);
	if (keysym == U)
		change_flevel(keysym, b, fractal);	
	else if (keysym == J)
		change_flevel(keysym, b, fractal);
	else if (keysym == RGHT_STRG)
		toggle_filter(fractal, b);
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
	else if (keysym == F2)
		print_buddha_guide();
	else if (keysym == F3)
		export(keysym, fractal);
	if (!fractal->supersample)
	{
		ft_memset(fractal->img.pixels_ptr, 0, fractal->width_orig * fractal->height_orig * (fractal->img.bpp / 8));	
		color_buddha(fractal);
		mlx_put_image_to_window(fractal->mlx_connect,
		fractal->mlx_win, fractal->img.img_ptr, 0, 0);
	}
}