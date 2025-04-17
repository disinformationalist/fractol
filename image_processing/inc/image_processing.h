#ifndef IMAGE_PROCESSING_H
# define IMAGE_PROCESSING_H

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#else
# include <unistd.h>
#endif
/*TO DO LIST
->IMAGE cropping top or bottom or left or right.
HIGH/LOW PASS FILTER
BUTTERWORTH
SOBEL

->UNSHARP_MASKING use convo or gauss blurs? using gauss is standard*/ 
//de noising monte, non- local means... etc.

# include <stdlib.h>
# include <stdio.h>
# include <math.h>
# include <stddef.h>
# include <png.h>
# include <stdint.h>
# include <stdbool.h>
# include <stdarg.h>
# include "../../minilibx-linux/mlx.h"
# include <pthread.h>
# include "tools.h"
# include <sys/time.h>


# define SQ(x) ((x) * (x))


/***STRUCTS***/

typedef struct s_img
{
	void	*img_ptr;
	char	*pixels_ptr;
	int		bpp;
	int		endian;
	int		line_len;
}	t_img;

t_img			*nlm_rouselle_vars(void *mlx_ptr, t_img *img, int width, int height, int patch_size);//testing
t_pixel 		**pad_zeros_pix(t_pixel **pix, int width, int height, int layers);

t_pixeld		**malloc_pixel_d_matrix(int width, int height);
void			free_pix_d_matrix(t_pixeld **matrix, int j);
t_pixeld		**pad_zeros_pix_d(t_pixeld **pix, int width, int height, int layers);
void			free_int_matrix(int **matrix, int j);


//NON LOCAL MEANS

//nlm
int				nlm_denoise(t_img *img, int width, int height, int patch_size, int win_size, double h);
double			gauss_weight(double dist, double h);
void			img_to_tpix(t_img *img, t_pixeld ***p_img, int width, int height);
void			thread_err(pthread_t *threads, int i);
t_img			*convert_tpixd_to_img(void *mlx_ptr, t_pixeld **out, int width, int height);
void			send_tpixd_to_img(t_pixeld **out, t_img *img, int width, int height);

/***LIB UTILS***/
char			*ft_strjoin_n(int count, ...);
char			*ft_strdup(const char *s);
size_t			ft_strlen(const char *s);
char			*ft_itoa(int n);
void			*ft_memset(void *s, int c, size_t n);
int				ft_atoi(const char *nptr);
double			ft_atof(char *s);
void			putstr_fd(char *s, int fd);
void			ft_putstr_color(char *s, char *color);

/**RANDOM */
void re_color_buddha(t_img *img, int width, int height, t_channel c);

/******PIXEL GET AND SET******/

void			my_pixel_put(int x, int y, t_img *img, unsigned int color);
void			my_pixel_put_plus(int x, int y, t_img *img, unsigned int color);
unsigned int	pixel_color_get(int x, int y, t_img *img);
void			get_pixel(t_pixel *pix_t, t_img *img, int x, int y);

// image creation
int				new_img_init(void *mlx_connection, t_img *img, int width, int height);
t_img 			*create_img(void *mlx_ptr, int width, int height);
t_img			*img_cpy(t_img *dest, t_img *src, int width, int height);
t_img			*img_dup(void *mlx_ptr, t_img *img, int width, int height);

/***COLORS***/
int				*set_color_wheel(int num_colors, double saturation, double lightness, int base_hue);
unsigned int	get_color_source(double x, double y, int *colors, int num_colors, int rot);
unsigned int	interpolate_two_colors(int color1, int color2, double t);


/****THREADING TOOLS****/
int				get_num_cores(void);
void			join_threads(pthread_t *threads, int num_rows, int num_cols);
void			set_least_diff_pair(int num, int *fact1, int *fact2);
long			get_time(void);
void			print_times(long start, long end, char *title, char *msg, char *color);


/******IMAGE PROCESSING FUNCTIONS******/

void			downsample(long width, long height, t_img *img, t_img *img_big, int kern_size);
int				gaussian_downsample(long width, long height, t_img *img, t_img *img_big, int kern_size, double sigma);//fix this now that pads are correct.
void			convo_down_filter_section(t_img *img, t_img *img_big, t_filter *f);
t_img			combine_images(t_img *img1, t_img *img2, int width, int height, void *mlx_connection);//needs working on

void			downsample_xl(int width, int height, t_img *img, unsigned int **pixels_xl, int kern_size);
void			downsample_double(int width, int height, double **dst, double **src, int kern_size);



int				mean_convo_filter(void *mlx_ptr, t_img *img, int width, int height, int kern_size);
int				mean_convo_matrix(double **mat, int width, int height, int kern_size);

int				gaussian_convo_filter(void *mlx_ptr, t_img *img, int width, int height, int kern_size, double sigma);
int				gaussmirror_convo_filter(void *mlx_ptr, t_img *img, int width, int height, int kern_size, int layers, double sigma);
t_img			*sobel(void *mlx_ptr, t_img *img, int kern_size, int width, int height);//change sobel like gauss whenits done

int				non_local_means(void *mlx_ptr, t_img *img, int width, int height, int region_size, double sigma);//ret -1 on failure



//padding utils
t_img			*loop_reflector(void *mlx_ptr, t_img *img, int *width, int *height, int layers, int iters);
t_img			*pad_img(void *mlx_ptr, t_img *img, int *height, int *width, int num_layers, t_op op);
t_img			*pad_zeros(void *mlx_ptr, t_img *img, int width, int height, int layers);
t_img			*pad_color(void *mlx_ptr, t_img *img, int width, int height, int layers, unsigned int color);
t_img			*pad_mirror(void *mlx_ptr, t_img *img, int width, int height, int layers);
double			**pad_mirror_matrix(double **mat, int width, int height, int layers);
t_img			*pad_mirror_circle(void *mlx_ptr, t_img *img, int width, int height, int layers, int axis_radius);
t_img			*loop_reflector_circle(void *mlx_ptr, t_img *img, int *width, int *height, int layers, int axis_radius, int iters);
unsigned int 	**pm_circle_matrix(unsigned int **in, int width, int height, int layers, int axis_radius);


t_pixel			**pad_mirror_pix(t_pixel **pix, int width, int height, int layers);
void			free_pix_matrix(t_pixel **matrix, int j);


double			**pad_mirror_kernel(t_filter *f, int width, int height, int layers);
void			copy_center(t_img *img, t_img *img_pad, int width, int height, int layers);

//processing utils
void			filter_section(t_img *img, t_img *img_new, t_filter *f);
void			apply_convo_filter(t_filter *f, t_img *img_z, t_img *img_out, int width, int height);
int				malloc_kernel(t_filter *f, int kern_size);


//matrix utils
double			get_matrix_sum(double **matrix, int width, int height);
double			**malloc_matrix(int width, int height);
unsigned char	**malloc_uc_matrix(int width, int height);
unsigned int	**malloc_ui_matrix(int width, int height);
void			zero_ui_matrix(unsigned int **pixels_xl, int width, int height);
void			free_ui_matrix(unsigned int **matrix, int j);
void			free_matrix_i(double **matrix, int i);//make generic with void?
int				**malloc_int_matrix(int width, int height);
t_pixel			**malloc_pixel_matrix(int width, int height);


//--------make kernels
int				make_gaussian(t_filter *f, int kern_size, double sigma);

//ADJUST PIXELS.
void			adjust_pixels_rgb(t_img *img, int width, int height, double times, double plus, t_channel c);
void			gamma_correct_rgb(t_img *img, int width, int height, double gamma, t_channel c);


/******EXPORT******/
int				export_png(const char *filename, t_img *img, int width, int height, png_text *text);

//export utils
char 			*get_nxt_name(char *name);
void			get_pixel(t_pixel *pix_t, t_img *img, int x, int y);
int				set_png_pixels(t_png_io *png_img, t_img *img, int height, int width);
int				init_png_structs(t_png_io *png_img, const char *filename);

// export/import utils
int				error_1(t_png_io *png_img, const char *msg);
void			free_png_rows(png_structp png_ptr, png_byte **row_pointers, int j);
void			clean_memory(t_png_io *png_img, int j, bool export);
void			init_vars(t_png_io *png_img);

/*****IMPORT******/
t_img			*import_png(void *mlx_ptr, const char *filename, int *width, int *height);

//import utils
void			*error_2(t_png_io *png_img, const char *msg);
int				error_3(t_png_io *png_img, const char *msg);

/****PRINTING UTILS****/
void			print_matrix(double **matrix, int cols, int rows);

#endif
