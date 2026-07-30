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
	else
		str = RED"--------------- PHEONIX STATS -----------------"RESET;
	return (str);
}

char	*get_ftype(t_ftype type)
{
	char	*str;

	if (type == ADJUST)
		str = MAGENTA"ADJUSTING"RESET;
	else
		str = RED"GAUSSIAN"RESET;
	return (str);
}

void	print_minmax(char *color, char *chan, int min, int max)
{
	printf("%s%s "RESET"channel iterations\n", color, chan);
	printf("Min: %d\n", min);
	printf("Max: %d\n\n", max);
}

void	print_edges(t_buddha b)
{
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
	printf("Tone normalization: %s\n",
		buddha_normalization_name(buddha->normalization));
	printf("Negative powers   : %s\n",
		buddha_negative_mode_name(buddha->negative_mode));
	printf("Hybrid spill mix  : %.3f\n", buddha->hybrid_spill);
	printf("Hybrid palette    : background #%06X, foreground #%06X\n",
		buddha->hybrid_background, buddha->hybrid_foreground);
	printf("Global exposure  : %.3f\n",
		buddha->global_exposure);
	printf("Channel gains    : B %.3f, G %.3f, R %.3f\n",
		buddha->exposure_b, buddha->exposure_g, buddha->exposure_r);
	printf("White percentile : %.4f (%.2f%%)\n",
		buddha->white_percentile, buddha->white_percentile * 100.0);
	printf(BLUE"Blue white point : %.6f\n"RESET, buddha->white_b);
	printf(GREEN"Green white point: %.6f\n"RESET, buddha->white_g);
	printf(RED"Red white point  : %.6f\n\n"RESET, buddha->white_r);
	printf("Sampling density : n=%.3f\n", buddha->n);
	printf("Sample buffers   : %d\n", fractal->buffs);
	printf("Orbit path       : %s\n",
		(char *[2]){"generic", "specialized square"}
		[buddha->square_specialized
			&& fractal->complex_f == &square_complex]);
	printf("Orbit cache      : %s (worker-local for max <= %d)\n",
		(char *[2]){"off", "on"}[buddha->orbit_cache],
		BUDDHA_ORBIT_CACHE_POINTS);
	printf("Interior reject  : %s\n",
		(char *[2]){"off", "analytic cardioid/bulb"}
		[buddha->interior_rejection]);
	printf("Importance pilot : configured=%d effective=%d %s metric=%s\n",
		ft_round(buddha->map_n), buddha_map_scale(fractal),
		(char *[2]){"fixed", "adaptive"}[buddha->map_adaptive],
		(char *[2]){"mean hits", "RMS hits"}[buddha->importance_rms]);
	printf("NLM              : %s (patch=%d search=%d kc=%.3f; %s)\n\n",
		(char *[2]){RED"OFF"RESET, GREEN"ON"RESET}[buddha->nlm_enabled],
		buddha->nlm_patch_radius, buddha->nlm_search_radius,
		buddha->nlm_kc,
		(char *[2]){"unfiltered mean", "filtered"}
		[buddha->nlm_show_filtered]);
	printf("Importance map   : %s%s; mode=%s\n\n",
		(char *[2]){RED"OFF"RESET, GREEN"CAPTURED"RESET}
		[buddha->importance_ready],
		(char *[2]){"", " (viewing)"}[buddha->importance_view],
		buddha_importance_mode_name(buddha->importance_mode));
	y = -fractal->move_y;
	printf("Complex plane center: real=%.17g imaginary=%.17g\n",
		fractal->move_x, y);
	printf("View zoom: %.17g (larger values zoom in)\n\n", fractal->zoom);
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
		printf("Multiplier: %f\n",
			(double)((buddha->flevel - 1) >> 1) * 0.5 + 1.0);
}
