/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chaos.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 15:13:17 by jhotchki          #+#    #+#             */
/*   Updated: 2024/01/22 17:29:42 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHAOS_H
# define CHAOS_H

# include <unistd.h>
//# include <mlx.h>
# include "../minilibx-linux/mlx.h"
# include <stdio.h>
# include <math.h>
# include <stdlib.h>
# include <time.h>
# include <X11/X.h>
# include <X11/keysym.h>
# include <stdbool.h>

# define PHI 1.61803398875
# define RGHT_STRG 65508
# define DIV 65455
# define MULT 65450
# define ALT 65513
# define STRG 65507
# define PAD_MINUS 65453
# define PAD_PLUS 65451
# define PAD_ZERO 65438
# define ALT_GR 65514
# define Ü 91

/*****FIRE*****/
# define DEEP_RED 0xFF0000
# define ORANGE 0xFFA500
# define YELLOW 0XFFFF00
# define INTENSE_PINK 0XFF1493

/*****ICE*****/
# define DEEP_PURPLE 0x4B0082
# define ICY_BLUE 0x00BFFF
# define LIGHT_CYAN 0xE0FFFF
# define SILVER 0xC0C0C0
# define BLUE_SHADE 0x00BFFF

/*****CANDY*****/
# define BUBBLEGUM_PINK 0xFF69B4
# define LEMON_YELLOW 0xFFF44F
# define MINT_GREEN 0x98FB98
# define SKY_BLUE 0x87CEEB

/*****SLIME*****/
# define TOXIC_GREEN 0x7FFF00
# define RADIOACTIVE_YELLOW 0XDAAF20
# define ALIEN_BLUE 0x00BFFF 
# define SLIMY_PURPLE 0x800080

/*****DARK MAGIC*****/
# define MYSTIC_PURPLE 0x800080
# define PURPLE_BLUE 0x241066
# define ERIE_BLUE 0x00008B 
# define SINISTER_RED 0x8B0000

typedef struct s_colors
{
	int	color_1;
	int	color_2;
	int	color_3;
	int	color_4;
}	t_colors;

typedef struct s_color_vars
{
	int	num_colors;
	int	color_index_low;
	int	color_index_high;
	int	color_low;
	int	color_high;
	int	color;
}	t_color_vars;

typedef struct s_img
{
	void	*img_ptr;
	char	*pixels_ptr;
	int		bpp;
	int		endian;
	int		line_len;
}	t_img;

typedef struct s_game
{
	t_img		img;
	t_img		img_new;
	t_colors	colors;
	int			rv[3];
	int			i;
	void		*mlx_connect;
	void		*mlx_win;
	double		move_x;
	double		move_y;
	double		rotate_l;
	double		rotate_r;
	double		zoom;
	double		max_distance;
	bool		rule_1;
	bool		rule_2;
	bool		rule_3;
	bool		rule_4;
	bool		rule_5;
	bool		rule_6;
	bool		rule_7;
	bool		rule_8;
	bool		rule_9;
	bool		rule_10;
	bool		rule_11;
	bool		rule_12;
	bool		rule_13;
	bool		rule_14;
	bool		rule_15;
	bool		rule_16;
	bool		rule_17;
	bool		rule_18;
	bool		rule_19;
	bool		rule_20;
	bool		rule_21;
	bool		rule_22;
	bool		rule_23;
	bool		rule_24;
	bool		rule_25;
	bool		rule_26;
	bool		rule_27;
	bool		rule_28;
	bool		rule_29;
	bool		disinfo_1;
	bool		disinfo_2;
	int			sides;
	int			r;
	int			iters;
	double		dist_ratio;
	bool		mouse_zoom;
	bool		supersample;
	int			s_kernal;
	int			width;
	int			height;
}		t_game;

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
	int				pix_x;
	int				pix_y;
}		t_filter;

int		ft_atoi(const char *nptr);
void	chaos_game(t_game *r, int **vertices, int x, int y);
void	my_pixel_put(int x, int y, t_img *img, int color);
void	intermed(t_game *r);
void	polygon(int ***vertices, t_game *r);
void	information(t_game *r, int v, int i);
int		ft_r(int rv[], int v, int i, t_game *r);
void	free_poly(int ***vertices, t_game *r);
void	events_init(t_game *r);
void	info_init(t_game *r);
void	r_init(t_game *r);
void	game_init(t_game *r);
void	downsample(t_game *r, t_img *img, t_img *img_new, int kern_size);
void	second_img_init(t_game *r);

/***COLORS***/
int		calc_color_4(double distance, double max_distance, t_colors *colors);
void	put_pixel(int x, int y, t_game *r, int color);

/***HOOKS***/
int		key_handler(int keysym, t_game *r);
int		key_handler_r(int keysym, t_game *r);
int		key_handler_other(int keysym, t_game *r);
int		mouse_handler(int button, int x, int y, t_game *r);
int		supersample_handler(int keysym, t_game *r);
void	zoom_iters_key(int keysym, t_game *r);
int		close_screen(t_game *r);

/***PRINT BOARD***/
void	print_board(t_game *r);
void	put_ratio(t_game *r);
void	put_bool(bool x);
void	ft_putstr(char *s);
void	ft_putstr_color(char *s, char *color);

#endif
