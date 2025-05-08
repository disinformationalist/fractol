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

#include <immintrin.h> // For AVX SIMD intrinsics
#include <float.h>
#include <time.h>
#include <xoro128.h>//better rand()

# define ERROR_MESSAGE "Syntax: \n\t\"./fractol mandelbrot <width> <height>\" \
\n or \t\"./fractol julia <value_1> <value_2> <width> <height>\" \
\n or \t\"./fractol buddha <width> <height> <type>\" \
\n or \t\"./fractol fern <width> <height>\"\n"

typedef long double ld_;

typedef struct s_complex
{
	double	x;
	double	y;
}	t_complex;

# define CUBE(x) ((x) * (x) * (x))
# define DEG_TO_RAD 0.01745329251 //(M_PI / 180.0)
# define RAD_TO_DEG 57.2957795131 //(180.0 / M_PI)

typedef pthread_mutex_t mtx_t;
typedef t_complex (*f)(t_complex);

typedef double (*num_f)(double);
typedef double (*den_f)(double, double);

typedef void (*f_simd)(__m256d, __m256d, __m256d*, __m256d*);


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
	MEAN,
	ADJUST,
	GAUSS,
} t_ftype;

typedef struct s_buddha
{
	t_btype	type;
	t_ftype ftype;

	double	n;
	double	map_n;
	
	double	rpow;
	double	gpow;
	double	bpow;

	double	high_r;
	double	high_g;
	double	high_b;

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
	int		fchan;
	int		flevel;

	bool	fast;
	bool	copy_half;

	void	*mlx_win_map;
} t_buddha;

typedef struct s_fractal
{
	t_img			img;
	t_img			img_2;
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
	mtx_t			mutex;
	double			aspect;
	
	//--------fern
	int				species;
	t_4colors		colors;
	int				**fdensity;
	
	//----------------buddha stuff
	t_buddha		*buddha;
	mtx_t 			rand_mtx;
	double			**density;
	double			*cdf;
	double			**pdf;
    f				complex_f;
	f_simd			complex_fsimd;
	double 			***densities;
	
	int				b_min_i;
	int				b_max_i;
	int				histograms;
	int				hist_num;
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
	double	n;
	double	samples;
	double	width;
	double	height;
	int		b_max_i;
	int		b_min_i;

	double	slopex_to;
	double	slopey_to;
	double 	slopex_back;
	double 	slopey_back;
	
	int		x;
	int		y;
	int		hist;

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

	double	**density;
	double	**pdf;

	double	**subpdf;
	double	sn;
	int		nn;
	int		ss;

	double	step;
	double	bound;

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
	t_fractal	*fractal;
	Xoro128		rng;


	//for testing
	unsigned int	thread_color;
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
void		*buddha_set_map(void *arg);
void		*fern_set(void *arg);
void		fern(t_fractal *fractal);

//some buddha utils
void		render_buddha(t_fractal *fractal);
double 		randf();
t_comps		set_comps(t_fractal *fractal, bool map);
void		zero_matrix(double **matrix, int width, int height);
void		init_matricies(t_fractal *fractal);
void		zero_densities(t_fractal *fractal);
void		free_3d_array_i(double ***array3d, int k, int height);
void		zero_int_matrix(int **matrix, int width, int height);
void		increase_counts(t_fractal *fractal, int k, int j, int i, double count_hits);
void		color_buddha(t_fractal *fractal);
int			binary_search(double *cdf, int size, double value);
void		set_vals(t_fractal *fractal, int min_1, int min_2, int min_3, int max_1, int max_2, int max_3, f complex_f);
void		buddha_iter_fullmap(t_fractal *fractal, t_complex c, double slope_x, double slope_y, f complex_f);
void		buddha_iteration(t_fractal *fractal, t_complex c, double weight, t_comps comps);

double		pow_ft(double num, double power);

//fast_buddha, sampling points all at once within pixel in accordance with importance
//using half copying for symmetric and a work balanced multithread.
void		fast_buddha(t_fractal *fractal);


/****UTILS****/
void		render(t_fractal *fractal);
double		ft_atof(char *s);
void		set_least_diff_pair(int num, int *fact1, int *fact2);
int			ft_round(double d);

int			ft_strncmp(char *s1, char *s2, int n);
void		putstr_fd(char *s, int fd);
int			ft_putchar_fd(char c, int fd);
int			ft_putstr_color_fd(int fd, char *s, char *color);
void		fractal_init(t_fractal *fractal);
void		*ft_memset(void *s, int c, size_t n);
double		**matcpy(double **dest, double **src, int width, int height);
void		put_pixel2(int x, int y, t_fractal *fractal, unsigned int color);


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

/***SIMD***/
__m256d 	sum_complex_simd(__m256d z1_x, __m256d z1_y, __m256d z2_x, __m256d z2_y);
void 		square_complex_simd(__m256d z_x, __m256d z_y, __m256d* res_x, __m256d* res_y);
void 		square_complex_conj_simd(__m256d z_x, __m256d z_y, __m256d* res_x, __m256d* res_y);

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
void 		buddha_handler(int keysym, t_fractal *fractal);
void		print_buddha_vals(t_buddha *buddha, t_fractal *fractal);



/***PRINT_UTILS****/
void		ft_putstr_color(char *s, char *color);
png_text 	*build_fractal_text(t_fractal *fractal);

/****THREAD UTILS****/
void		set_pieces(t_fractal *fractal, t_piece piece[][fractal->num_cols], int i, int j);
void		set_pieces_half(t_fractal *fractal, t_piece piece[][fractal->num_cols], int i, int j);

/***CLEAN UP***/
int			close_handler(t_fractal *fractal);
void		clear_all(t_fractal *fractal);
void		thread_error(t_fractal *fractal, int i);
void		free_matrices(t_fractal *fractal);

/***GUIDES***/
void		print_mj_guide(void);
void		print_buddha_guide(void);

#endif
