#include "image_processing.h"

//---------------this one is working and multithreaded-------------------

double		patch_dist(t_pixeld **p_img_pad, int half_p, int x1, int y1, int x2, int y2)
{
	double		dist;
	int			i;
	int			j;
	t_pixeld	p1;
	t_pixeld	p2;

	dist = 0.0;
	j = -half_p - 1;
	while(++j <= half_p)
	{
		i = -half_p - 1;
		while (++i <= half_p)
		{
			p1 = p_img_pad[y1 + j][x1 + i];
			p2 = p_img_pad[y2 + j][x2 + i];
			dist += (p1.red - p2.red) * (p1.red - p2.red);
			dist += (p1.green - p2.green) * (p1.green - p2.green);
			dist += (p1.blue - p2.blue) * (p1.blue - p2.blue);
		}
	}
	return (dist);
}

void	traverse_window(t_nlm *nlm, int x, int y)
{
	double sum_weights = 0.0;
	double sum_r = 0.0;
	double sum_g = 0.0;
	double sum_b = 0.0;
	double dist;
	double weight;
	int i;
	int j;
	int xi;
	int yj;

	j = -nlm->half_win - 1;
	while (++j <= nlm->half_win)
	{
		i = -nlm->half_win - 1;
		while (++i <= nlm->half_win)
		{
			xi = x + i;
			yj = y + j;
			
			dist = patch_dist(nlm->p_img_pad, nlm->half_p, x, y, xi, yj);
			weight = gauss_weight(dist, nlm->h);
			sum_weights += weight;
			sum_r += weight * nlm->p_img_pad[xi][yj].red;
			sum_g += weight * nlm->p_img_pad[xi][yj].green;
			sum_b += weight * nlm->p_img_pad[xi][yj].blue;
		}
	}
	int yadj = y - nlm->pad_layers;
	int xadj = x - nlm->pad_layers;

	(nlm->new)[yadj][xadj].red = (int)(sum_r / sum_weights);
	(nlm->new)[yadj][xadj].green = (int)(sum_g / sum_weights);
	(nlm->new)[yadj][xadj].blue = (int)(sum_b / sum_weights);
}

void	*compute_piece(void *arg)
{
 	t_pix_piece	*piece;
	t_nlm		*nlm;	
	int 		i;
	int 		j;

	piece = (t_pix_piece *)arg;
	nlm = piece->nlm;
	j = piece->y_s + nlm->pad_layers - 1;
	while (++j < piece->y_e + nlm->pad_layers)
	{
		i = piece->x_s + nlm->pad_layers - 1;
		while (++i < piece->x_e + nlm->pad_layers)
			traverse_window(nlm, i , j);
	}
	pthread_exit(NULL);
}

void	thread_and_run(t_nlm *nlm)
{
	int i;
	int j;

	t_pix_piece piece[nlm->num_rows][nlm->num_cols];
	
	nlm->threads = (pthread_t *)malloc(nlm->num_rows * nlm->num_cols * sizeof(pthread_t));
	if (!nlm->threads)
	{
		printf("Error: Thread Malloc failed\n");
		return ;
	}
	j = -1;
	while (++j < nlm->num_rows)
	{
		i = -1;
		while (++i < nlm->num_cols)
		{
			piece[j][i].x_s = i * (nlm->width / nlm->num_cols);
			piece[j][i].x_e = (i + 1) * (nlm->width / nlm->num_cols);
			piece[j][i].y_s = j * (nlm->height / nlm->num_rows);
			piece[j][i].y_e = (j + 1) * (nlm->height / nlm->num_rows);
			piece[j][i].nlm = nlm;
			if (pthread_create(&nlm->threads[j * nlm->num_cols + i], NULL, \
				compute_piece, (void *)&piece[j][i]) != 0)//must pass in all params with piece...
				thread_err(nlm->threads, j * nlm->num_cols + i);//finish protecting.
		}
	}
	join_threads(nlm->threads, nlm->num_rows, nlm->num_cols);
	free(nlm->threads);
}

int	nlm_denoise(t_img *img, int width, int height, int patch_size, int win_size, double h)
{
	t_nlm 			nlm;
	t_pixeld		**p_img;
	
	nlm.num_cols = 1;
	nlm.num_rows = get_num_cores();
	nlm.half_p = patch_size / 2;
	nlm.half_win = win_size / 2;
	nlm.pad_layers = nlm.half_win + nlm.half_p;
	nlm.h = h;
	nlm.width = width;
	nlm.height = height;	

	p_img = malloc_pixel_d_matrix(width, height);
	img_to_tpix(img, &p_img, width, height);
	//nlm.p_img_pad = pad_mirror_pix_d(p_img, width, height, nlm.pad_layers);//have to make pad_mirror_pix_d, can also handle edges by using diff window size
	nlm.p_img_pad = pad_zeros_pix_d(p_img, width, height, nlm.pad_layers);
	free_pix_d_matrix(p_img, height);
	nlm.new = malloc_pixel_d_matrix(width, height);
	
	thread_and_run(&nlm);
	free_pix_d_matrix(nlm.p_img_pad, height + 2 * nlm.pad_layers);
	//t_img *out = convert_tpixd_to_img(mlx_ptr, nlm.new, width, height);
	
	send_tpixd_to_img(nlm.new, img, width, height);
	return (0);
}