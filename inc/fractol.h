/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fractol.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:07:29 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:07:35 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FRACTOL_H
# define FRACTOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include "../minilibx-linux/mlx.h"
#include <X11/X.h>
#include <X11/keysym.h>
#include "../image_processing/inc/image_processing.h"
#include "ansi_colors.h"
#include <pthread.h>
#include "keyboard.h"
//# include "keyboard (42).h"

#include <immintrin.h>
#include <float.h>
#include <time.h>
#include <xoro128.h>//better rand()

# define ERROR_MESSAGE "Syntax: \n\t\"./fractol mandelbrot <width> <height>\" \
\n or \t\"./fractol julia <value_1> <value_2> <width> <height>\" \
\n or \t\"./fractol buddha <width> <height> <type> [--importance-map] \
[--center <real> <imaginary>] [--zoom <scale>]\" \
\n or \t\"./fractol fern <width> <height>\"\n"
# define BUDDHA_WHITE_PERCENTILE 0.999
# define BUDDHA_MAP_REFERENCE_SIZE 640
# define BUDDHA_CHANNELS 3
# define BUDDHA_NLM_MATRICES 9
# define BUDDHA_ORBIT_CACHE_POINTS 512
# define BUDDHA_HYBRID_BACKGROUND 0x00B3D38E
# define BUDDHA_HYBRID_FOREGROUND 0x0029314F
# define BUDDHA_HYBRID_SPILL 0.2

typedef struct s_complex
{
	double	x;
	double	y;
}	t_complex;

# define CUBE(x) ((x) * (x) * (x))
# define DEG_TO_RAD 0.01745329251 //(M_PI / 180.0)
# define RAD_TO_DEG 57.2957795131 //(180.0 / M_PI)

typedef t_complex (*f)(t_complex);

/***STRUCTS***/

typedef struct s_4colors
{
	int	color_1;
	int	color_2;
	int	color_3;
	int	color_4;
}	t_4colors;

typedef struct s_color_vars
{
	int	num_colors;
	int	color_index_low;
	int	color_index_high;
	int	color_low;
	int	color_high;
	int	color;
}	t_color_vars;

typedef enum e_btype
{
	BUDDHA1,
	BUDDHA2,
	LOTUS,
	PHEONIX,
} t_btype;

typedef enum e_ftype
{
	ADJUST,
	GAUSS,
} t_ftype;

typedef enum e_buddha_importance_mode
{
	BUDDHA_IMPORTANCE_RGB,
	BUDDHA_IMPORTANCE_BLUE,
	BUDDHA_IMPORTANCE_GREEN,
	BUDDHA_IMPORTANCE_RED,
	BUDDHA_IMPORTANCE_MODE_COUNT,
}	t_buddha_importance_mode;

typedef enum e_buddha_normalization
{
	BUDDHA_NORM_CHANNEL_PERCENTILE,
	BUDDHA_NORM_LINKED_PERCENTILE,
	BUDDHA_NORM_LEGACY_MAX,
}	t_buddha_normalization;

typedef enum e_buddha_negative_mode
{
	BUDDHA_NEGATIVE_INVERSE,
	BUDDHA_NEGATIVE_SPILL,
	BUDDHA_NEGATIVE_HYBRID,
}	t_buddha_negative_mode;

typedef struct s_buddha_tone_state
{
	double					power[3];
	double					edge0[3];
	double					edge1[3];
	double					exposure[3];
	double					global_exposure;
	double					white_percentile;
	t_buddha_normalization	normalization;
	t_buddha_negative_mode	negative_mode;
	double					hybrid_spill;
	unsigned int			hybrid_background;
	unsigned int			hybrid_foreground;
	bool					smootherstep;
}	t_buddha_tone_state;

typedef struct s_buddha_channel
{
	int	histogram;
	int	min_iter;
	int	max_iter;
}	t_buddha_channel;

typedef struct s_buddha
{
	t_btype	type;
	t_ftype ftype;

	double	n;
	double	map_n;
	bool	map_adaptive;
	bool	importance_rms;
	bool	square_specialized;
	bool	orbit_cache;
	bool	interior_rejection;
	
	double	rpow;
	double	gpow;
	double	bpow;

	double	high_r;
	double	high_g;
	double	high_b;
	double	white_r;
	double	white_g;
	double	white_b;
	double	white_percentile;
	double	exposure_r;
	double	exposure_g;
	double	exposure_b;
	double	global_exposure;
	t_buddha_normalization	normalization;
	t_buddha_tone_state		tone_default;
	t_buddha_tone_state		tone_alternate;
	bool					tone_gain_mode;
	bool					tone_extra_fine;

	int		min1;
	int		min2;
	int		min3;
	int		max1;
	int		max2;
	int		max3;

	double	edge0_r;
	double	edge0_g;
	double	edge0_b;
	double	edge1_r;
	double	edge1_g;
	double	edge1_b;

	bool	filter;
	bool	smootherstep;
	t_buddha_negative_mode	negative_mode;
	double					hybrid_spill;
	unsigned int			hybrid_background;
	unsigned int			hybrid_foreground;
	int		flevel;

	bool	nlm_enabled;
	bool	nlm_smooth_var;
	int		nlm_patch_radius;
	int		nlm_search_radius;
	double	nlm_kc;
	double	nlm_noise_scale;
	double	nlm_noise_floor;
	double	nlm_r_weight;
	double	nlm_g_weight;
	double	nlm_b_weight;
	bool	nlm_variance_ready;
	bool	nlm_filtered_ready;
	bool	nlm_show_filtered;

	bool	fast;
	double	change;
	bool	view_center_set;
	bool	view_zoom_set;
	double	view_center_real;
	double	view_center_imaginary;
	double	view_zoom;

	bool			importance_enabled;
	bool			importance_ready;
	bool			importance_view;
	t_buddha_importance_mode	importance_mode;
	uint16_t		*importance_values;
	unsigned int	*render_pixels;
} t_buddha;

typedef struct s_fractal
{
	t_img			img;
	t_img			img_2;
	t_img			display_img;
	bool			display_rotated;
	int				display_width;
	int				display_height;
	int				col_i;
	int				max_i;
	char			*name;
	void			*mlx_connect;
	void			*mlx_win;
	double			bound;
	double			move_x;
	double			move_y;
	double			zoom;
	double			julia_x;
	double			julia_y;
	bool			toggle_color;
	bool			j_handle;
	bool			color_spectrum;
	bool			layer;
	double			move_col_x;
	double			move_col_y;
	bool			mouse_zoom;
	int				height;
	int				width;
	bool			supersample;
	bool			zoom_iter;
	int				s_kernel;
	pthread_t		*threads;
	double			aspect;
	
	//--------fern
	int				species;
	t_4colors		colors;
	int				**fdensity;
	
	//----------------buddha stuff
	t_buddha		*buddha;
	uint32_t		*sample_counts;
    f				complex_f;
	double 			***densities;
	double			**worker_histograms;
	int				worker_histogram_count;
	int				buddha_workspace_width;
	int				buddha_workspace_height;
	int				buddha_workspace_histograms;
	int				buddha_workspace_workers;
	
	int				b_min_i;
	int				b_max_i;
	int				histograms;
	int				hist_num;
	int				buffs;
	//----------------end buddha stuff

	int				size;
	int				num_colors;
	int				width_orig;
	int				height_orig;
	int				*w_colors;
	unsigned int 	**pixels_xl;
	int 			num_rows;
	int 			num_cols;
	int 			id;
}	t_fractal;

//comps for buddha 

typedef struct s_comps
{
	double	samples;
	size_t	sample_budget;
	int		width;
	int		height;
	int		map_scale;
	int		b_max_i;
	int		b_min_i;

	double	slopex_to;
	double	slopey_to;
	double 	slopex_back;
	double 	slopey_back;
	
	double	move_x;
	double	move_y;
	double	zoom;
	double	inv_zoom;

	double	x_cmin;
	double	x_cmax;
	double	y_cmin;
	double	y_cmax;
	double	x_span;
	double	y_span;
	double	sample_x_min;
	double	sample_y_min;
	double	sample_x_span;
	double	sample_y_span;

	double	*density_flat;
	uint32_t	*sample_counts;
	double	bound;
	bool	square_formula;
	bool	interior_rejection;

	f		complex_f;

}	t_comps;

//thread data for each thread

typedef struct s_piece
{
	int			id;
	int			x_s;
	int			x_e;
	int			y_s;
	int			y_e;
	int			buffer;
	t_fractal	*fractal;
	Xoro128		rng;
}	t_piece;


/***FRACTALS***/

//multithread functions

void		mandelbrot(t_fractal *fractal);
void		*mandel_set(void *arg);
void		julia(t_fractal *fractal);
void		*julia_set(void *arg);
void		buddha(t_fractal *fractal);
void		*buddha_set(void *arg);
void		buddha_map(t_fractal *fractal);
void		buddha_map_all(t_fractal *fractal);
void		*buddha_set_map(void *arg);
void		*fern_set(void *arg);
void		fern(t_fractal *fractal);

//some buddha utils
void		render_buddha(t_fractal *fractal);
int			parse_buddha_options(t_buddha *buddha, int count,
				char **options);
void		print_buddha_usage(int fd);
void		buddha_apply_start_view(t_fractal *fractal);
void		buddha_render_channel(t_fractal *fractal, int min_iter,
				int max_iter, int histogram);
void		build_importance_map(t_fractal *fractal, double **density);
size_t		buddha_sample_budget(t_fractal *fractal);
int			buddha_map_scale(t_fractal *fractal);
void		buddha_importance_begin(t_fractal *fractal);
void		buddha_importance_capture(t_fractal *fractal, int histogram);
bool		buddha_importance_toggle(t_fractal *fractal);
bool		buddha_importance_cycle(t_fractal *fractal);
void		buddha_importance_draw(t_fractal *fractal);
void		buddha_importance_pixel_rgb(t_fractal *fractal, int x, int y,
				double rgb[3]);
char		*buddha_importance_mode_name(t_buddha_importance_mode mode);
void		buddha_importance_store_render(t_fractal *fractal);
t_comps		set_comps(t_fractal *fractal, bool map);
t_complex	buddha_sample_point(t_comps comps, double pixel_x,
				double pixel_y);
void		zero_matrix(double **matrix, int width, int height);
void		init_matricies(t_fractal *fractal);
int			buddha_ensure_workspace(t_fractal *fractal);
void		free_3d_array_i(double ***array3d, int k, int height);
int			buddha_init_worker_histograms(t_fractal *fractal);
void		buddha_free_worker_histograms(t_fractal *fractal);
void		buddha_clear_worker_histograms(t_fractal *fractal);
void		buddha_reduce_worker_histograms(t_fractal *fractal, int buffer);
void		color_buddha(t_fractal *fractal);
void		buddha_pixel_rgb(t_fractal *fractal, int x, int y,
				double rgb[3]);
void		set_vals(t_fractal *fractal, int min_1, int min_2, int min_3, int max_1, int max_2, int max_3, f complex_f);
void		buddha_iteration(t_complex c, double weight, t_comps comps,
				t_complex *orbit_cache);
int			buddha_escape_length(t_complex c, t_comps comps,
				int max_iterations);
int			buddha_escape_orbit(t_complex c, t_comps comps,
				int max_iterations, t_complex *orbit, int capacity);
int			buddha_visible_hits(t_complex c, t_comps comps,
				int orbit_length);
int			buddha_cached_visible_hits(const t_complex *orbit,
				t_comps comps, int orbit_length);

void		combine_buff_set_var(double ***densities, int hist, int buffs, int width, int height);
int			buddha_nlm(t_fractal *fractal);
void		buddha_nlm_reset(t_fractal *fractal);
bool		buddha_nlm_toggle_output(t_fractal *fractal);
double		buddha_density_percentile(double **density, int width,
				int height, double percentile);
double		buddha_density_percentile_rgb(double ***densities, int width,
				int height, double percentile);
double		buddha_curve_value(double density, double reference,
				double power);
double		buddha_adjust_power(double power, double direction, bool fine,
				bool extra_fine);
unsigned int	buddha_legacy_spill_color(double density, double reference,
				double power, double exposure, unsigned int shift);
double		buddha_smootherstep_value(double edge0, double edge1,
				double value);
double		buddha_tone_value(double density, double white_point,
				double power, double edge0, double edge1, double exposure);
void		buddha_tone_store_defaults(t_buddha *buddha);
void		buddha_tone_reset(t_buddha *buddha);
void		buddha_tone_swap_alternate(t_buddha *buddha);
char		*buddha_normalization_name(t_buddha_normalization normalization);
char		*buddha_negative_mode_name(t_buddha_negative_mode mode);
void		buddha_update_white_points(t_fractal *fractal);
bool		buddha_profile_enabled(void);
void		buddha_profile_phase(char *name, long start);
void		buddha_profile_density(t_fractal *fractal, int histogram);
void		buddha_set_channel_statistics(t_fractal *fractal, int histogram);
double		high_hit_count(int width, int height, double **density);


//fast_buddha, sampling points within each input cell by importance
void		fast_buddha(t_fractal *fractal);


/****UTILS****/
void		render(t_fractal *fractal);
void		display_fractal_image(t_fractal *fractal);
bool		buddha_should_rotate_display(int width, int height,
				int screen_height);
void		display_to_render_coordinates(t_fractal *fractal, int *x, int *y);
double		ft_atof(char *s);
void		set_least_diff_pair(int num, int *fact1, int *fact2);
int			ft_round(double d);

int			ft_strncmp(char *s1, char *s2, int n);
void		putstr_fd(char *s, int fd);
int			ft_putchar_fd(char c, int fd);
int			ft_putstr_color_fd(int fd, char *s, char *color);
void		fractal_init(t_fractal *fractal);
void		*ft_memset(void *s, int c, size_t n);


double		map(double unscaled_num, double new_min, double new_max, double old_max);
double		map_back(double unscaled_num, double new_max, double old_min, double old_max);
double		map_2(double unscaled_num, double new_min, double slope);
double		map_back_2(double unscaled_num, double old_min, double slope);

/***COMPLEX OPS***/
t_complex	sum_complex(t_complex z1, t_complex z2);
t_complex	square_complex(t_complex z);
t_complex	ship(t_complex z);
t_complex	celtic(t_complex z);
t_complex	cube_complex(t_complex z);
t_complex	cube_ship(t_complex z);
t_complex	pow4_complex(t_complex z);
t_complex	pow5_complex(t_complex z);
t_complex	pow6_complex(t_complex z);
t_complex	square_complex_conj(t_complex z);
t_complex	cube_complex_conj(t_complex z);
t_complex	pow4_complex_conj(t_complex z);
t_complex	pow5_complex_conj(t_complex z);
t_complex	pow6_complex_conj(t_complex z);
t_complex	inv_square_complex(t_complex z);

/***COLORS***/
int			iter_color(double curr_iter, double max_iter, int base_color);
int			get_color(double x, double y, t_fractal *fractal);
void		color_option(t_fractal *fractal, t_position p, double iter);
void		put_pixel(int x, int y, t_fractal *fractal, unsigned int color);

/***BARNSLEY FERN***/
int			render_barnsleyfern(t_fractal *fractal);
void		set_fern_species(int species, double **values);
double		**alloc_fern(t_fractal *fractal, double **vals);
void		free_fern(double **vals, int j);
void		density_color(t_fractal *fractal);

void		init_fdensity(t_fractal *fractal);
double		high_hit_countf(long width, long height, int **density);
void		free_fdensity(t_fractal *fractal, int j);
void		zero_fdensity(t_fractal *fractal);
int			calc_color_4(double distance, double max_distance, t_4colors *colors);

/***EVENTS***/
int			key_handler(int keysym, t_fractal *fractal);
int			mouse_handler(int button, int x, int y, t_fractal *fractal);
int			julia_handle(int x, int y, t_fractal *fractal);
int			supersample_handle(int keysym, t_fractal *fractal);
void		export(int keysym, t_fractal *fractal);
int			export_png16(const char *filename, t_fractal *fractal,
				png_text *text);
void 		buddha_handler(int keysym, t_fractal *fractal);
void		print_buddha_vals(t_buddha *buddha, t_fractal *fractal);



/***PRINT_UTILS****/
void		ft_putstr_color(char *s, char *color);
png_text 	*build_fractal_text(t_fractal *fractal);

/****THREAD UTILS****/
void		set_pieces(t_fractal *fractal, t_piece piece[][fractal->num_cols], int i, int j);

/***CLEAN UP***/
int			close_handler(t_fractal *fractal);
void		clear_all(t_fractal *fractal);
void		thread_error(t_fractal *fractal, int i);
void		free_matrices(t_fractal *fractal);

/***GUIDES***/
void		print_mj_guide(void);
void		print_buddha_guide(void);

#endif
