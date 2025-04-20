#include "fractol.h"

char	*get_type(t_buddha *b)
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

char	*get_ftype(t_ftype type)
{
	char	*str;

	if (type == MEAN)
		str = GREEN"AVERAGING(blue only)"RESET;
	else if (type == ADJUST)
		str = MAGENTA"ADJUSTING"RESET;
	else //(type == GAUSS)
		str = RED"GAUSSIAN"RESET;
	/* else if (type == )
		str = BLUE""RESET; */
	return (str);
}

void	print_minmax(char *color, char *chan, int min, int max)// t_buddha *buddha)
{
	printf("%s%s "RESET"channel iterations\n", color, chan);
	printf("Min: %d\n", min);
	printf("Max: %d\n\n", max);
}

void	print_edges(t_buddha b)
{
	//printf("\nchannel iterations\n", color, chan);
	printf(BLUE"\nedge0: %f  edge1: %f\n"RESET, b.edge0_b, b.edge1_b);
	printf(GREEN"edge0: %f  edge1: %f\n"RESET, b.edge0_g, b.edge1_g);
	printf(RED"edge0: %f  edge1: %f\n\n"RESET, b.edge0_r, b.edge1_r);

}

void	print_buddha_vals(t_buddha *buddha, t_fractal *fractal)
{
	char	*s;
	double	y;

	printf("\n%s\n\n", get_type(buddha));
	printf(BLUE"Blue power value : %f\n"RESET, buddha->bpow);
	printf(GREEN"Green power value: %f\n"RESET, buddha->gpow);
	printf(RED"Red power value  : %f\n\n"RESET, buddha->rpow);
	y = fractal->move_y;
	if (fractal->move_y != 0)
		y = -y;
	printf("Complex plane coordinates of center:  x: %f  i: %f\n", fractal->move_x, y);
	printf("Zoom factor: %f \n\n", fractal->zoom * (fractal->width << 2));
	print_minmax(BLUE, "Blue", buddha->min1, buddha->max1);
	print_minmax(GREEN, "Green", buddha->min2, buddha->max2);
	print_minmax(RED, "Red", buddha->min3, buddha->max3);
	if (buddha->smootherstep)
		print_edges(*buddha);
	if (buddha->filter)
		s = GREEN"ON"RESET;
	else
		s = RED"OFF"RESET;
	printf("%s Filter: %s level: %d\n", get_ftype(buddha->ftype), s, buddha->flevel);
	if (buddha->ftype == GAUSS)
		printf("Sigma: %f\n", (double)((buddha->flevel - 1) >> 1) * .5);
	if (buddha->ftype == ADJUST)
		printf("Multiplier: %f\n", (double)((buddha->flevel - 1) >> 1) + 1);
}