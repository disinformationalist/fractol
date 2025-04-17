#ifndef TOOLS_H
# define TOOLS_H

#include "image_processing.h"

typedef struct s_pixel
{
//	uint8_t alpha;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
}	t_pixel;

typedef struct s_pixeld
{
	//double alpha;
	double red;
	double green;
	double blue;
}	t_pixeld;

typedef struct s_position
{
	int	x;
	int	y;
}	t_position;

typedef struct s_nlm
{
	int			half_win;
	int			half_p;
	int			pad_layers;
	double		h;
	int			width;
	int			height;
	int			num_rows;
	int			num_cols;
	t_pixeld	**p_img_pad;
	t_pixeld	**new;
	pthread_t	*threads;
}	t_nlm;

typedef struct s_pix_piece
{
	int			x_s;
	int			x_e;
	int			y_s;
	int			y_e;
	t_nlm		*nlm;
}	t_pix_piece;

typedef struct s_filter
{
	int				half_k;
	int				x;
	int				y;
	int				i;
	int				j;
	int				x_r;
	int				y_r;
	
	double			mean_r;
	double			mean_g;
	double			mean_b;

	double			red;
	double			green;
	double			blue;
	int				offset;
	int				avg_r;
	int				avg_g;
	int				avg_b;
	unsigned int	pixel;
	double				**kernel;
	int				factor;
	double			kern_sum;
	int				pix_x;
	int				pix_y;
}		t_filter;

typedef struct s_png_io
{
	int			y;
	int			x;
	int			pixel_size;
	int			depth;
	t_pixel		temp_pixel;
	png_byte	**row_pointers;
	png_infop	info;
	png_structp	png_ptr;
	FILE		*fp;
	png_text	*text;
}	t_png_io;

typedef struct s_wheel
{
	int		*colors;
	float	hue_step;
	float	c;
	float	x;
	float	m;
	float	r_;
	float	g_;
	float	b_;
	int		hue;
}				t_wheel;

typedef struct s_color// changed to uint8_t s from unsigned char
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
}	t_color;

typedef enum e_op
{
	ZEROS,
	COLOR,
	REFLECT,
	DUPLICATE,
} t_op;

typedef enum e_channel
{
	NONE = 0,
	R = 1 << 0,
	G = 1 << 1,
	B = 1 << 2,
	RG = R | G,
	RB = R | B,
	GB = G | B,
	GR = G | R,
	BR = B | R,
	BG = B | G,
	RGB = R | G | B,
	RBG = R | B | G,
	BGR = B | G | B,
	BRG = B | R | G,
	GBR = G | B | R,
	GRB = G | R | B,
} t_channel;

#endif