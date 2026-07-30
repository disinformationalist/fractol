#include "fractol.h"

#define NLM_EPS 1e-12
#define NLM_EXP_CUTOFF 12.0
#define NLM_EXP_LUT_SIZE 4096

typedef struct s_nlm_ctx
{
	t_fractal	*fractal;
	double		**mean[3];
	double		**variance[3];
	double		**output[3];
	int			patch_radius;
	int			search_radius;
	double		kc2;
	double		noise_scale;
	double		noise_floor[3];
	double		weight[3];
	double		weight_sum;
	double		patch_norm;
}	t_nlm_ctx;

typedef struct s_nlm_piece
{
	t_piece		piece;
	t_nlm_ctx	ctx;
}	t_nlm_piece;

static double	g_nlm_exp_lut[NLM_EXP_LUT_SIZE + 1];
static bool		g_nlm_exp_lut_ready;

static int	max_int(int a, int b)
{
	if (a > b)
		return (a);
	return (b);
}

static int	min_int(int a, int b)
{
	if (a < b)
		return (a);
	return (b);
}

static void	init_exp_lut(void)
{
	double	x;
	int		i;

	if (g_nlm_exp_lut_ready)
		return ;
	i = -1;
	while (++i <= NLM_EXP_LUT_SIZE)
	{
		x = ((double)i / (double)NLM_EXP_LUT_SIZE) * NLM_EXP_CUTOFF;
		g_nlm_exp_lut[i] = exp(-x);
	}
	g_nlm_exp_lut_ready = true;
}

static double	nlm_exp_weight(double distance)
{
	double	position;
	double	fraction;
	int		index;

	if (distance >= NLM_EXP_CUTOFF)
		return (g_nlm_exp_lut[NLM_EXP_LUT_SIZE]);
	position = distance
		* ((double)NLM_EXP_LUT_SIZE / NLM_EXP_CUTOFF);
	index = (int)position;
	fraction = position - (double)index;
	return (g_nlm_exp_lut[index]
		+ (g_nlm_exp_lut[index + 1] - g_nlm_exp_lut[index]) * fraction);
}

static t_nlm_ctx	make_nlm_context(t_fractal *fractal)
{
	t_nlm_ctx	ctx;
	t_buddha	*b;
	double		white;
	int			channel;
	int			patch_width;

	b = fractal->buddha;
	ctx.fractal = fractal;
	channel = -1;
	while (++channel < 3)
	{
		ctx.mean[channel] = fractal->densities[channel];
		ctx.variance[channel] = fractal->densities[channel + 3];
		ctx.output[channel] = fractal->densities[channel + 6];
	}
	ctx.patch_radius = b->nlm_patch_radius;
	ctx.search_radius = b->nlm_search_radius;
	ctx.kc2 = b->nlm_kc * b->nlm_kc;
	ctx.noise_scale = b->nlm_noise_scale;
	ctx.weight[0] = b->nlm_b_weight;
	ctx.weight[1] = b->nlm_g_weight;
	ctx.weight[2] = b->nlm_r_weight;
	ctx.weight_sum = ctx.weight[0] + ctx.weight[1] + ctx.weight[2];
	if (ctx.weight_sum <= 0.0)
		ctx.weight_sum = 3.0;
	channel = -1;
	while (++channel < 3)
	{
		white = buddha_density_percentile(ctx.mean[channel],
				fractal->width, fractal->height, b->white_percentile);
		if (white <= 0.0)
			white = 1.0;
		ctx.noise_floor[channel] = white * white * b->nlm_noise_floor;
	}
	patch_width = 2 * ctx.patch_radius + 1;
	ctx.patch_norm = (double)(patch_width * patch_width) * ctx.weight_sum;
	return (ctx);
}

static double	smooth_variance_value(double **variance, int x, int y,
		int width, int height)
{
	double	sum;
	int		count;
	int		sx;
	int		sy;

	sum = 0.0;
	count = 0;
	sy = max_int(0, y - 1) - 1;
	while (++sy <= min_int(height - 1, y + 1))
	{
		sx = max_int(0, x - 1) - 1;
		while (++sx <= min_int(width - 1, x + 1))
		{
			sum += variance[sy][sx];
			count++;
		}
	}
	return (sum / (double)count);
}

static void	smooth_variance_channel(t_fractal *fractal, int channel,
		double *rows)
{
	double	**variance;
	double	*previous;
	double	*current;
	double	*temporary;
	int		x;
	int		y;

	variance = fractal->densities[channel + 3];
	previous = rows;
	current = rows + fractal->width;
	y = -1;
	while (++y < fractal->height)
	{
		x = -1;
		while (++x < fractal->width)
			current[x] = smooth_variance_value(variance, x, y,
					fractal->width, fractal->height);
		if (y > 0)
			memcpy(variance[y - 1], previous,
				(size_t)fractal->width * sizeof(**variance));
		temporary = previous;
		previous = current;
		current = temporary;
	}
	if (fractal->height > 0)
		memcpy(variance[fractal->height - 1], previous,
			(size_t)fractal->width * sizeof(**variance));
}

static int	smooth_variances(t_fractal *fractal)
{
	double	*rows;
	int	channel;

	if (!fractal->buddha->nlm_smooth_var)
		return (0);
	rows = (double *)malloc((size_t)fractal->width * 2 * sizeof(*rows));
	if (!rows)
		return (-1);
	channel = -1;
	while (++channel < 3)
		smooth_variance_channel(fractal, channel, rows);
	free(rows);
	return (0);
}

static double	nlm_term(double value1, double value2,
		double variance1, double variance2, int channel,
		const t_nlm_ctx *ctx)
{
	double	difference;
	double	variance;

	difference = value1 - value2;
	variance = (variance1 + variance2) * ctx->noise_scale;
	if (variance < ctx->noise_floor[channel])
		variance = ctx->noise_floor[channel];
	return ((difference * difference - variance)
		/ (NLM_EPS + ctx->kc2 * variance));
}

static double	shifted_term(int x, int y, int dx, int dy,
		const t_nlm_ctx *ctx)
{
	double		distance;

	distance = 0.0;
	distance += nlm_term(ctx->mean[0][y][x],
			ctx->mean[0][y + dy][x + dx], ctx->variance[0][y][x],
			ctx->variance[0][y + dy][x + dx], 0, ctx) * ctx->weight[0];
	distance += nlm_term(ctx->mean[1][y][x],
			ctx->mean[1][y + dy][x + dx], ctx->variance[1][y][x],
			ctx->variance[1][y + dy][x + dx], 1, ctx) * ctx->weight[1];
	distance += nlm_term(ctx->mean[2][y][x],
			ctx->mean[2][y + dy][x + dx], ctx->variance[2][y][x],
			ctx->variance[2][y + dy][x + dx], 2, ctx) * ctx->weight[2];
	return (distance);
}

static double	patch_distance_border(int x, int y, int qx, int qy,
		const t_nlm_ctx *ctx)
{
	t_fractal	*fractal;
	double		distance;
	int			terms;
	int			px;
	int			py;

	fractal = ctx->fractal;
	distance = 0.0;
	terms = 0;
	py = -ctx->patch_radius - 1;
	while (++py <= ctx->patch_radius)
	{
		px = -ctx->patch_radius - 1;
		while (++px <= ctx->patch_radius)
		{
			if (x + px < 0 || x + px >= fractal->width
				|| y + py < 0 || y + py >= fractal->height
				|| qx + px < 0 || qx + px >= fractal->width
				|| qy + py < 0 || qy + py >= fractal->height)
				continue ;
			distance += nlm_term(ctx->mean[0][y + py][x + px],
					ctx->mean[0][qy + py][qx + px],
					ctx->variance[0][y + py][x + px],
					ctx->variance[0][qy + py][qx + px],
					0, ctx) * ctx->weight[0];
			distance += nlm_term(ctx->mean[1][y + py][x + px],
					ctx->mean[1][qy + py][qx + px],
					ctx->variance[1][y + py][x + px],
					ctx->variance[1][qy + py][qx + px],
					1, ctx) * ctx->weight[1];
			distance += nlm_term(ctx->mean[2][y + py][x + px],
					ctx->mean[2][qy + py][qx + px],
					ctx->variance[2][y + py][x + px],
					ctx->variance[2][qy + py][qx + px],
					2, ctx) * ctx->weight[2];
			terms++;
		}
	}
	if (terms > 0)
		distance /= (double)terms * ctx->weight_sum;
	if (distance < 0.0)
		distance = 0.0;
	return (distance);
}

static void	nlm_pixel_border(int x, int y, const t_nlm_ctx *ctx)
{
	t_fractal	*fractal;
	double		filtered[3];
	double		total_weight;
	double		distance;
	double		weight;
	int			dx;
	int			dy;
	int			qx;
	int			qy;

	fractal = ctx->fractal;
	memset(filtered, 0, sizeof(filtered));
	total_weight = 0.0;
	dy = max_int(-ctx->search_radius, -y) - 1;
	while (++dy <= min_int(ctx->search_radius, fractal->height - 1 - y))
	{
		dx = max_int(-ctx->search_radius, -x) - 1;
		while (++dx <= min_int(ctx->search_radius, fractal->width - 1 - x))
		{
			qx = x + dx;
			qy = y + dy;
			distance = patch_distance_border(x, y, qx, qy, ctx);
			if (distance > NLM_EXP_CUTOFF)
				continue ;
			weight = nlm_exp_weight(distance);
			total_weight += weight;
			filtered[0] += weight * ctx->mean[0][qy][qx];
			filtered[1] += weight * ctx->mean[1][qy][qx];
			filtered[2] += weight * ctx->mean[2][qy][qx];
		}
	}
	if (total_weight > 0.0)
	{
		ctx->output[0][y][x] = filtered[0] / total_weight;
		ctx->output[1][y][x] = filtered[1] / total_weight;
		ctx->output[2][y][x] = filtered[2] / total_weight;
	}
	else
	{
		ctx->output[0][y][x] = ctx->mean[0][y][x];
		ctx->output[1][y][x] = ctx->mean[1][y][x];
		ctx->output[2][y][x] = ctx->mean[2][y][x];
	}
}

static void	build_horizontal_sums(double *horizontal, double *terms,
		int x0, int x1, int extended_y0, int extended_height,
		int dx, int dy, const t_nlm_ctx *ctx)
{
	double	sum;
	int		inner_width;
	int		patch_width;
	int		term_x0;
	int		row;
	int		slot;
	int		x;
	int		y;
	int		term;

	inner_width = x1 - x0;
	patch_width = 2 * ctx->patch_radius + 1;
	term_x0 = x0 - ctx->patch_radius;
	row = -1;
	while (++row < extended_height)
	{
		y = extended_y0 + row;
		sum = 0.0;
		term = -1;
		while (++term < patch_width)
		{
			terms[term] = shifted_term(term_x0 + term, y, dx, dy, ctx);
			sum += terms[term];
		}
		horizontal[row * inner_width] = sum;
		slot = 0;
		x = 0;
		while (++x < inner_width)
		{
			sum -= terms[slot];
			terms[slot] = shifted_term(term_x0 + x + patch_width - 1,
					y, dx, dy, ctx);
			sum += terms[slot];
			horizontal[row * inner_width + x] = sum;
			slot++;
			if (slot == patch_width)
				slot = 0;
		}
	}
}

static void	accumulate_offset(double *weights, double *horizontal,
		int bounds[4], int dx, int dy, const t_nlm_ctx *ctx)
{
	double		vertical_sum;
	double		distance;
	double		weight;
	int			patch_width;
	int			inner_width;
	int			x;
	int			y;

	inner_width = bounds[1] - bounds[0];
	patch_width = 2 * ctx->patch_radius + 1;
	x = bounds[0] - 1;
	while (++x < bounds[1])
	{
		vertical_sum = 0.0;
		y = -1;
		while (++y < patch_width)
			vertical_sum += horizontal[y * inner_width + x - bounds[0]];
		y = bounds[2] - 1;
		while (++y < bounds[3])
		{
			distance = vertical_sum / ctx->patch_norm;
			if (distance < 0.0)
				distance = 0.0;
			if (distance <= NLM_EXP_CUTOFF)
			{
				weight = nlm_exp_weight(distance);
				weights[(y - bounds[2]) * inner_width
					+ x - bounds[0]] += weight;
				ctx->output[0][y][x] += weight
					* ctx->mean[0][y + dy][x + dx];
				ctx->output[1][y][x] += weight
					* ctx->mean[1][y + dy][x + dx];
				ctx->output[2][y][x] += weight
					* ctx->mean[2][y + dy][x + dx];
			}
			if (y + 1 < bounds[3])
			{
				vertical_sum -= horizontal[(y - bounds[2])
					* inner_width + x - bounds[0]];
				vertical_sum += horizontal[(y - bounds[2] + patch_width)
					* inner_width + x - bounds[0]];
			}
		}
	}
}

static void	finalize_inner(double *weights, int bounds[4],
		const t_nlm_ctx *ctx)
{
	double		total_weight;
	int			inner_width;
	int			x;
	int			y;

	inner_width = bounds[1] - bounds[0];
	y = bounds[2] - 1;
	while (++y < bounds[3])
	{
		x = bounds[0] - 1;
		while (++x < bounds[1])
		{
			total_weight = weights[(y - bounds[2]) * inner_width
				+ x - bounds[0]];
			if (total_weight > 0.0)
			{
				ctx->output[0][y][x] /= total_weight;
				ctx->output[1][y][x] /= total_weight;
				ctx->output[2][y][x] /= total_weight;
			}
			else
			{
				ctx->output[0][y][x] = ctx->mean[0][y][x];
				ctx->output[1][y][x] = ctx->mean[1][y][x];
				ctx->output[2][y][x] = ctx->mean[2][y][x];
			}
		}
	}
}

static void	zero_inner(int bounds[4], const t_nlm_ctx *ctx)
{
	size_t	bytes;
	int	y;

	bytes = (size_t)(bounds[1] - bounds[0]) * sizeof(double);
	y = bounds[2] - 1;
	while (++y < bounds[3])
	{
		memset(ctx->output[0][y] + bounds[0], 0, bytes);
		memset(ctx->output[1][y] + bounds[0], 0, bytes);
		memset(ctx->output[2][y] + bounds[0], 0, bytes);
	}
}

static int	nlm_fast_inner(int bounds[4], const t_nlm_ctx *ctx)
{
	double	*weights;
	double	*horizontal;
	double	*terms;
	size_t	pixels;
	int		inner_width;
	int		inner_height;
	int		extended_height;
	int		extended_y0;
	int		dx;
	int		dy;

	inner_width = bounds[1] - bounds[0];
	inner_height = bounds[3] - bounds[2];
	if (inner_width <= 0 || inner_height <= 0)
		return (1);
	pixels = (size_t)inner_width * (size_t)inner_height;
	weights = calloc(pixels, sizeof(*weights));
	extended_height = inner_height + 2 * ctx->patch_radius;
	horizontal = malloc((size_t)inner_width * (size_t)extended_height
			* sizeof(*horizontal));
	terms = malloc((size_t)(2 * ctx->patch_radius + 1) * sizeof(*terms));
	if (!weights || !horizontal || !terms)
	{
		free(weights);
		free(horizontal);
		free(terms);
		return (0);
	}
	zero_inner(bounds, ctx);
	extended_y0 = bounds[2] - ctx->patch_radius;
	dy = -ctx->search_radius - 1;
	while (++dy <= ctx->search_radius)
	{
		dx = -ctx->search_radius - 1;
		while (++dx <= ctx->search_radius)
		{
			build_horizontal_sums(horizontal, terms, bounds[0], bounds[1],
				extended_y0, extended_height, dx, dy, ctx);
			accumulate_offset(weights, horizontal, bounds, dx, dy, ctx);
		}
	}
	finalize_inner(weights, bounds, ctx);
	free(terms);
	free(horizontal);
	free(weights);
	return (1);
}

static void	nlm_piece(t_piece *piece, const t_nlm_ctx *ctx)
{
	t_fractal	*fractal;
	int			bounds[4];
	int			margin;
	int			x;
	int			y;

	fractal = ctx->fractal;
	margin = ctx->search_radius + ctx->patch_radius;
	bounds[0] = max_int(piece->x_s, margin);
	bounds[1] = min_int(piece->x_e, fractal->width - margin);
	bounds[2] = max_int(piece->y_s, margin);
	bounds[3] = min_int(piece->y_e, fractal->height - margin);
	y = piece->y_s - 1;
	while (++y < piece->y_e)
	{
		x = piece->x_s - 1;
		while (++x < piece->x_e)
		{
			if (x >= bounds[0] && x < bounds[1]
				&& y >= bounds[2] && y < bounds[3])
				continue ;
			nlm_pixel_border(x, y, ctx);
		}
	}
	if (bounds[0] < bounds[1] && bounds[2] < bounds[3])
	{
		if (nlm_fast_inner(bounds, ctx))
			return ;
		y = bounds[2] - 1;
		while (++y < bounds[3])
		{
			x = bounds[0] - 1;
			while (++x < bounds[1])
				nlm_pixel_border(x, y, ctx);
		}
	}
}

static void	*run_nlm_piece(void *arg)
{
	t_nlm_piece	*piece;

	piece = (t_nlm_piece *)arg;
	nlm_piece(&piece->piece, &piece->ctx);
	return (NULL);
}

static void	set_piece(t_nlm_piece *piece, t_fractal *fractal,
		t_nlm_ctx ctx, int column, int row)
{
	piece->piece.x_s = column * (fractal->width / fractal->num_cols);
	piece->piece.x_e = (column + 1) * (fractal->width / fractal->num_cols);
	piece->piece.y_s = row * (fractal->height / fractal->num_rows);
	piece->piece.y_e = (row + 1) * (fractal->height / fractal->num_rows);
	if (column + 1 == fractal->num_cols)
		piece->piece.x_e = fractal->width;
	if (row + 1 == fractal->num_rows)
		piece->piece.y_e = fractal->height;
	piece->piece.fractal = fractal;
	piece->ctx = ctx;
}

static void	swap_nlm_output(t_fractal *fractal)
{
	double	**temporary;
	int		channel;

	channel = -1;
	while (++channel < 3)
	{
		temporary = fractal->densities[channel];
		fractal->densities[channel] = fractal->densities[channel + 6];
		fractal->densities[channel + 6] = temporary;
	}
}

void	buddha_nlm_reset(t_fractal *fractal)
{
	if (!fractal || !fractal->buddha)
		return ;
	if (fractal->densities
		&& fractal->histograms >= BUDDHA_NLM_MATRICES
		&& fractal->buddha->nlm_filtered_ready
		&& fractal->buddha->nlm_show_filtered)
		swap_nlm_output(fractal);
	fractal->buddha->nlm_variance_ready = false;
	fractal->buddha->nlm_filtered_ready = false;
	fractal->buddha->nlm_show_filtered = false;
}

bool	buddha_nlm_toggle_output(t_fractal *fractal)
{
	t_buddha	*b;

	if (!fractal || !fractal->buddha || !fractal->densities
		|| fractal->histograms < BUDDHA_NLM_MATRICES)
		return (false);
	b = fractal->buddha;
	if (!b->nlm_filtered_ready)
		return (false);
	swap_nlm_output(fractal);
	b->nlm_show_filtered = !b->nlm_show_filtered;
	return (true);
}

int	buddha_nlm(t_fractal *fractal)
{
	t_nlm_piece	pieces[fractal->num_rows * fractal->num_cols];
	t_nlm_ctx	context;
	t_buddha	*b;
	long		phase_start;
	int			column;
	int			row;
	int			index;
	bool		was_showing_filtered;

	if (!fractal->buddha || !fractal->densities
		|| fractal->buffs < 2
		|| fractal->histograms < BUDDHA_NLM_MATRICES)
		return (-1);
	b = fractal->buddha;
	was_showing_filtered = b->nlm_show_filtered;
	if (was_showing_filtered)
	{
		swap_nlm_output(fractal);
		b->nlm_show_filtered = false;
	}
	phase_start = get_time();
	if (!b->nlm_variance_ready)
	{
		if (smooth_variances(fractal) != 0)
		{
			if (was_showing_filtered)
			{
				swap_nlm_output(fractal);
				b->nlm_show_filtered = true;
			}
			return (-1);
		}
		b->nlm_variance_ready = true;
	}
	buddha_profile_phase("nlm variance", phase_start);
	init_exp_lut();
	context = make_nlm_context(fractal);
	if (buddha_profile_enabled())
		printf("[buddha] nlm patch=%d search=%d kc=%.3f "
			"noise-scale=%.5f term-cache=patch-ring "
			"variance-scratch=two-row:%.3fMiB\n",
			context.patch_radius, context.search_radius, sqrt(context.kc2),
			context.noise_scale, (double)fractal->width * 2.0
			* sizeof(double) / (1024.0 * 1024.0));
	phase_start = get_time();
	row = -1;
	while (++row < fractal->num_rows)
	{
		column = -1;
		while (++column < fractal->num_cols)
		{
			index = row * fractal->num_cols + column;
			set_piece(&pieces[index], fractal, context, column, row);
			if (pthread_create(&fractal->threads[index], NULL,
					run_nlm_piece, &pieces[index]) != 0)
				thread_error(fractal, index);
		}
	}
	join_threads(fractal->threads, fractal->num_rows, fractal->num_cols);
	swap_nlm_output(fractal);
	b->nlm_filtered_ready = true;
	b->nlm_show_filtered = true;
	buddha_profile_phase("nlm matching", phase_start);
	return (0);
}
