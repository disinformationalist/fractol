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

# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <unistd.h>
# include <math.h>
//# include <mlx.h>
#include "minilibx-linux/mlx.h"

# include <X11/X.h>
# include <X11/keysym.h>
# include <pthread.h>

# define NUM_ROWS 4
# define NUM_COLS 4
# define ERROR_MESSAGE "Syntax: \n\t\"./fractol mandelbrot\" \
or \n\t\"./fractol julia <value_1> <value_2>\" \
or \n\t\"./fractol buddha\"\n"

# define WIDTH   1080
# define HEIGHT  1080
# define PLUS 93
# define MINUS 47
# define NUM_UP 65431
# define NUM_DOWN 65433
# define NUM_RIGHT 65432
# define NUM_LEFT 65430
# define NUM_ZERO 65438
# define LFT_STRG 65507
# define RGHT_STRG 65508
# define ALT 65513

/***STRUCTS***/

typedef struct s_colors
{
	int		*colors;
	double	saturation;
	double	lightness;
	float	hue_step;
	float	c;
	float	x;
	float	m;
	float	r_;
	float	g_;
	float	b_;
	int		hue;
	int		base_hue;
}				t_colors;

typedef struct s_img
{
	void	*img_ptr;
	char	*pixels_ptr;
	int		bpp;
	int		endian;
	int		line_len;
}	t_img;

typedef struct s_fractal
{
	t_img			img;
	t_img			img_new;
	t_colors		*co;
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
	double			move_col_x;
	double			move_col_y;
	bool			mouse_zoom;
	bool			zoom_iter;
	int				b_max_i;
	double			n;
	pthread_t		threads[NUM_COLS * NUM_ROWS];
	pthread_mutex_t	mutex;
	int				**density;
}	t_fractal;

typedef struct s_piece
{
	int			x_s;
	int			x_e;
	int			y_s;
	int			y_e;
	t_fractal	*fractal;
}	t_piece;

typedef struct s_complex
{
	double	x;
	double	y;
}	t_complex;

typedef struct s_position
{
	int	x;
	int	y;
}	t_position;

typedef struct s_filter
{
	int				half_k;
	int				x;
	int				y;
	int				i;
	int				j;
	int				red;
	int				green;
	int				blue;
	int				offset;
	int				avg_r;
	int				avg_g;
	int				avg_b;
	unsigned int	pixel;

}	t_filter;

/***THREADS***/

void		mandelbrot(t_fractal *fractal);
void		*mandel_set(void *arg);
void		julia(t_fractal *fractal);
void		*julia_set(void *arg);
void		buddha(t_fractal *fractal);
void		*buddha_set(void *arg);

/****UTILS****/
void		render(t_fractal *fractal);
double		ft_atof(char *s);
t_complex	sum_complex(t_complex z1, t_complex z2);
t_complex	square_complex(t_complex z);
double		map(double unscaled_num, double new_min, \
double new_max, double old_max);
double		map_back(double unscaled_num, double new_max, \
double old_min, double old_max);
int			ft_strncmp(char *s1, char *s2, int n);
void		putstr_fd(char *s, int fd);
void		fractal_init(t_fractal *fractal);
void		my_pixel_put(int x, int y, t_img *img, int color);
void		my_pixel_put_plus(int x, int y, t_img *img, int color);
void		init_density(t_fractal *fractal);
void		free_density(t_fractal *fractal, int j);
void		zero_density(t_fractal *fractal);

/***IMG PROCESSING UTILS***/
void		mean_convo_filter(t_img *img, t_img *img_new, int kern_size);
void		adjust_pixels_rgb(t_img *img, double times, double plus);

/***COLORS***/
void		get_color_wheel(int num_colors, t_colors *co, t_fractal *fractal);
int			iter_color(double curr_iter, double max_iter, int base_color);
int			get_color(double x, double y, t_fractal *fractal);
void		color_option(t_fractal *fractal, t_position p, double iter);

/***HOOKS***/
int			key_handler(int keysym, t_fractal *fractal);
int			mouse_handler(int button, int x, int y, t_fractal *fractal);
int			julia_handle(int x, int y, t_fractal *fractal);

/***FINISH***/
int			close_handler(t_fractal *fractal);
void		clear_all(t_fractal *fractal);
void		thread_error(t_fractal *fractal, int num_threads);

#endif
