#include "fractol.h"

#define REFINE_SCORE_BINS 1024
#define REFINE_CANDIDATE_DIVISOR 100
#define REFINE_CANDIDATE_MIN 64
#define REFINE_CANDIDATE_MAX 8192
#define REFINE_AUTO_SUPPORT_DIVISOR 20
#define REFINE_INITIAL_GRID 4
#define REFINE_BEAM_WIDTH 2
#define REFINE_LEVELS 2

typedef struct s_refine_node
{
	double	u0;
	double	v0;
	double	size;
	double	rank;
	double	scores[BUDDHA_CHANNELS];
}	t_refine_node;

void	buddha_clear_importance_proposal(t_buddha *buddha)
{
	free(buddha->proposal_leaves);
	buddha->proposal_leaves = NULL;
	buddha->proposal_leaf_count = 0;
	memset(buddha->proposal_leaf_total, 0,
		sizeof(buddha->proposal_leaf_total));
}

static void	channel_limits(t_buddha *b, int minimum[3], int maximum[3])
{
	minimum[0] = b->min1;
	minimum[1] = b->min2;
	minimum[2] = b->min3;
	maximum[0] = b->max1;
	maximum[1] = b->max2;
	maximum[2] = b->max3;
}

static bool	channel_active(int channel, int active_channel)
{
	return (active_channel < 0 || channel == active_channel);
}

static int	adaptive_escape_length(t_complex c, t_comps comps)
{
	return (buddha_escape_length(c, comps, comps.b_max_i));
}

static double	adaptive_visible_score(t_piece *piece, t_complex c,
		t_comps comps, int orbit_length, t_buddha_score_scratch *scratch,
		int *hits)
{
	if (!piece->fractal->buddha->importance_recurrence)
	{
		*hits = buddha_visible_hits(c, comps, orbit_length);
		return ((double)*hits);
	}
	return (buddha_visible_footprint(c, comps, orbit_length, scratch, hits));
}

static void	record_adaptive_sample(t_piece *piece, int channel,
		bool eligible, int hits, double score)
{
	t_buddha_pilot_stats	*stats;

	stats = &piece->pilot_stats;
	stats->samples[channel]++;
	stats->adaptive_samples[channel]++;
	if (eligible)
		stats->eligible[channel]++;
	if (hits > 0)
	{
		stats->useful[channel]++;
		stats->adaptive_useful[channel]++;
	}
	stats->visible_hits[channel] += (uint64_t)hits;
	stats->adaptive_visible_hits[channel] += (uint64_t)hits;
	stats->score_sum[channel] += (long double)score;
	stats->adaptive_score_sum[channel] += (long double)score;
}

static void	evaluate_parameter(t_piece *piece, t_complex c, t_comps comps,
		t_buddha_score_scratch *scratch, double scores[3])
{
	int	minimum[3];
	int	maximum[3];
	int	orbit_length;
	int	hits;
	int	channel;
	double	score;
	bool	eligible[3];

	memset(scores, 0, sizeof(double) * BUDDHA_CHANNELS);
	channel_limits(piece->fractal->buddha, minimum, maximum);
	orbit_length = adaptive_escape_length(c, comps);
	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
		eligible[channel] = channel_active(channel, piece->active_channel)
			&& orbit_length > minimum[channel]
			&& orbit_length < maximum[channel];
	if (!eligible[0] && !eligible[1] && !eligible[2])
	{
		channel = -1;
		while (++channel < BUDDHA_CHANNELS)
			if (channel_active(channel, piece->active_channel))
				record_adaptive_sample(piece, channel, false, 0, 0.0);
		return ;
	}
	score = adaptive_visible_score(piece, c, comps, orbit_length,
			scratch, &hits);
	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
	{
		if (!channel_active(channel, piece->active_channel))
			continue ;
		if (eligible[channel])
			scores[channel] = score;
		record_adaptive_sample(piece, channel, eligible[channel],
			hits * eligible[channel], scores[channel]);
	}
}

static double	score_rank(t_piece *piece, double scores[3])
{
	double	rank;
	double	normalized;
	int		channel;

	rank = 0.0;
	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
	{
		if (!channel_active(channel, piece->active_channel))
			continue ;
		normalized = scores[channel];
		if (piece->score_norm[channel] > 0.0)
			normalized /= piece->score_norm[channel];
		if (normalized > rank)
			rank = normalized;
	}
	return (rank);
}

static void	evaluate_node(t_piece *piece, t_comps comps, int x, int y,
		t_refine_node *node, t_buddha_score_scratch *scratch,
		double sums[3], double squares[3], int *samples)
{
	t_complex	c;
	double		scores[3];
	double		sample_x;
	double		sample_y;
	int			channel;

	sample_x = ((double)x + node->u0 + node->size * 0.5)
		* (double)comps.map_scale;
	sample_y = ((double)y + node->v0 + node->size * 0.5)
		* (double)comps.map_scale;
	c = buddha_sample_point(comps, sample_x, sample_y);
	evaluate_parameter(piece, c, comps, scratch, scores);
	node->rank = score_rank(piece, scores);
	memcpy(node->scores, scores, sizeof(node->scores));
	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
	{
		sums[channel] += scores[channel];
		squares[channel] += scores[channel] * scores[channel];
	}
	(*samples)++;
}

static void	select_beam(t_refine_node *nodes, int node_count,
		t_refine_node beam[REFINE_BEAM_WIDTH])
{
	bool	used[REFINE_INITIAL_GRID * REFINE_INITIAL_GRID];
	double	best_rank;
	int		best;
	int		selected;
	int		index;

	memset(used, 0, sizeof(used));
	best_rank = 0.0;
	index = -1;
	while (++index < node_count)
		if (nodes[index].rank > best_rank)
			best_rank = nodes[index].rank;
	if (best_rank == 0.0)
	{
		beam[0] = nodes[0];
		beam[1] = nodes[node_count - 1];
		return ;
	}
	selected = -1;
	while (++selected < REFINE_BEAM_WIDTH)
	{
		best = -1;
		index = -1;
		while (++index < node_count)
			if (!used[index] && (best < 0
					|| nodes[index].rank > nodes[best].rank))
				best = index;
		used[best] = true;
		beam[selected] = nodes[best];
	}
}

static void	initial_refinement(t_piece *piece, t_comps comps, int x, int y,
		t_buddha_score_scratch *scratch, t_refine_node beam[2],
		double sums[3], double squares[3], int *samples)
{
	t_refine_node	nodes[REFINE_INITIAL_GRID * REFINE_INITIAL_GRID];
	double			size;
	int				u;
	int				v;
	int				index;

	size = 1.0 / (double)REFINE_INITIAL_GRID;
	index = 0;
	v = -1;
	while (++v < REFINE_INITIAL_GRID)
	{
		u = -1;
		while (++u < REFINE_INITIAL_GRID)
		{
			nodes[index] = (t_refine_node){u * size, v * size, size, 0.0,
			{0.0, 0.0, 0.0}};
			evaluate_node(piece, comps, x, y, &nodes[index], scratch,
				sums, squares, samples);
			index++;
		}
	}
	select_beam(nodes, index, beam);
}

static void	refine_beam_level(t_piece *piece, t_comps comps, int x, int y,
		t_buddha_score_scratch *scratch, t_refine_node beam[2],
		double sums[3], double squares[3], int *samples)
{
	t_refine_node	nodes[REFINE_BEAM_WIDTH * 4];
	double			size;
	int				parent;
	int				child;
	int				index;

	index = 0;
	parent = -1;
	while (++parent < REFINE_BEAM_WIDTH)
	{
		size = beam[parent].size * 0.5;
		child = -1;
		while (++child < 4)
		{
			nodes[index] = (t_refine_node){beam[parent].u0
				+ (double)(child & 1) * size, beam[parent].v0
				+ (double)(child >> 1) * size, size, 0.0,
				{0.0, 0.0, 0.0}};
			evaluate_node(piece, comps, x, y, &nodes[index], scratch,
				sums, squares, samples);
			index++;
		}
	}
	select_beam(nodes, index, beam);
}

static void	fold_refinement(t_piece *piece, size_t index,
		double sums[3], double squares[3], int samples, int base_samples)
{
	t_fractal	*fractal;
	double		baseline;
	double		combined;
	int			x;
	int			y;
	int			channel;

	fractal = piece->fractal;
	x = (int)(index % (size_t)fractal->width);
	y = (int)(index / (size_t)fractal->width);
	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
	{
		if (!channel_active(channel, piece->active_channel))
			continue ;
		baseline = fractal->densities[channel][y][x];
		if (fractal->buddha->importance_rms)
		{
			combined = baseline * baseline * (double)base_samples
				+ squares[channel];
			fractal->densities[channel][y][x] = sqrt(combined
					/ (double)(base_samples + samples));
		}
		else
			fractal->densities[channel][y][x] = (baseline
					* (double)base_samples + sums[channel])
				/ (double)(base_samples + samples);
	}
}

static void	refine_cell(t_piece *piece, t_comps comps, size_t index,
		t_buddha_score_scratch *scratch, size_t slot)
{
	t_refine_node	beam[REFINE_BEAM_WIDTH];
	double			sums[3];
	double			squares[3];
	int				samples;
	int				x;
	int				y;
	int				level;

	memset(sums, 0, sizeof(sums));
	memset(squares, 0, sizeof(squares));
	x = (int)(index % (size_t)comps.width);
	y = (int)(index / (size_t)comps.width);
	samples = 0;
	initial_refinement(piece, comps, x, y, scratch, beam,
		sums, squares, &samples);
	level = -1;
	while (++level < REFINE_LEVELS)
		refine_beam_level(piece, comps, x, y, scratch, beam,
			sums, squares, &samples);
	level = -1;
	while (piece->leaf_slots && ++level < REFINE_BEAM_WIDTH)
	{
		piece->leaf_slots[slot + (size_t)level].cell = (uint32_t)index;
		piece->leaf_slots[slot + (size_t)level].u0 = (float)beam[level].u0;
		piece->leaf_slots[slot + (size_t)level].v0 = (float)beam[level].v0;
		piece->leaf_slots[slot + (size_t)level].size = (float)beam[level].size;
		memcpy(piece->leaf_slots[slot + (size_t)level].cumulative,
			beam[level].scores, sizeof(beam[level].scores));
	}
	fold_refinement(piece, index, sums, squares, samples,
		comps.map_scale * comps.map_scale);
	piece->pilot_stats.refined_cells++;
}

static void	*refine_worker(void *argument)
{
	t_piece				*piece;
	t_comps				comps;
	t_buddha_score_scratch	scratch;
	size_t				work;
	size_t				end;

	piece = (t_piece *)argument;
	comps = set_comps(piece->fractal, true);
	if (piece->active_channel < 0)
	{
		comps.b_max_i = piece->fractal->buddha->max1;
		if (piece->fractal->buddha->max2 > comps.b_max_i)
			comps.b_max_i = piece->fractal->buddha->max2;
		if (piece->fractal->buddha->max3 > comps.b_max_i)
			comps.b_max_i = piece->fractal->buddha->max3;
	}
	if (piece->fractal->buddha->importance_recurrence)
		memset(&scratch, 0, sizeof(scratch));
	work = (size_t)piece->x_s;
	end = (size_t)piece->x_e;
	while (work < end)
	{
		refine_cell(piece, comps, piece->work_indices[work], &scratch,
			work * REFINE_BEAM_WIDTH);
		work++;
	}
	return (NULL);
}

static double	cell_rank(t_fractal *fractal, size_t index,
		int active_channel, double maximum[3])
{
	double	rank;
	double	value;
	int		x;
	int		y;
	int		channel;

	x = (int)(index % (size_t)fractal->width);
	y = (int)(index / (size_t)fractal->width);
	rank = 0.0;
	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
	{
		if (!channel_active(channel, active_channel) || maximum[channel] <= 0.0)
			continue ;
		value = fractal->densities[channel][y][x] / maximum[channel];
		if (isfinite(value) && value > rank)
			rank = value;
	}
	return (rank);
}

static void	map_maximums(t_fractal *fractal, int active_channel,
		double maximum[3])
{
	size_t	index;
	double	value;
	int		x;
	int		y;
	int		channel;

	memset(maximum, 0, sizeof(double) * BUDDHA_CHANNELS);
	index = 0;
	while (index < (size_t)fractal->size)
	{
		x = (int)(index % (size_t)fractal->width);
		y = (int)(index / (size_t)fractal->width);
		channel = -1;
		while (++channel < BUDDHA_CHANNELS)
		{
			if (!channel_active(channel, active_channel))
				continue ;
			value = fractal->densities[channel][y][x];
			if (isfinite(value) && value > maximum[channel])
				maximum[channel] = value;
		}
		index++;
	}
}

static bool	map_needs_refinement(t_fractal *fractal, int active_channel,
		double maximum[3])
{
	size_t	support;
	size_t	index;
	size_t	threshold;

	if (fractal->buddha->importance_refinement_force)
		return (true);
	threshold = (size_t)fractal->size / REFINE_AUTO_SUPPORT_DIVISOR;
	support = 0;
	index = 0;
	while (index < (size_t)fractal->size && support <= threshold)
	{
		if (cell_rank(fractal, index, active_channel, maximum) > 0.0)
			support++;
		index++;
	}
	return (support <= threshold);
}

static size_t	candidate_limit(t_fractal *fractal)
{
	size_t	limit;

	limit = (size_t)fractal->size / REFINE_CANDIDATE_DIVISOR;
	if (limit < REFINE_CANDIDATE_MIN)
		limit = REFINE_CANDIDATE_MIN;
	if (limit > REFINE_CANDIDATE_MAX)
		limit = REFINE_CANDIDATE_MAX;
	if (limit > (size_t)fractal->size)
		limit = (size_t)fractal->size;
	return (limit);
}

static void	mark_neighbors(t_fractal *fractal, size_t index)
{
	int	x;
	int	y;
	int	dx;
	int	dy;

	x = (int)(index % (size_t)fractal->width);
	y = (int)(index / (size_t)fractal->width);
	dy = -2;
	while (++dy <= 1)
	{
		dx = -2;
		while (++dx <= 1)
			if (x + dx >= 0 && x + dx < fractal->width
				&& y + dy >= 0 && y + dy < fractal->height)
				fractal->sample_counts[(size_t)(y + dy)
					* (size_t)fractal->width + (size_t)(x + dx)] = 1;
	}
}

static int	rank_bin(double rank)
{
	int	bin;

	if (rank <= 0.0)
		return (-1);
	bin = (int)floor(rank * (double)(REFINE_SCORE_BINS - 1));
	if (bin < 0)
		bin = 0;
	if (bin >= REFINE_SCORE_BINS)
		bin = REFINE_SCORE_BINS - 1;
	return (bin);
}

static int	candidate_threshold(t_fractal *fractal, int active_channel,
		double maximum[3], size_t limit)
{
	size_t	bins[REFINE_SCORE_BINS];
	size_t	index;
	size_t	cumulative;
	int		bin;

	memset(bins, 0, sizeof(bins));
	index = 0;
	while (index < (size_t)fractal->size)
	{
		bin = rank_bin(cell_rank(fractal, index, active_channel, maximum));
		if (bin >= 0)
			bins[bin]++;
		index++;
	}
	cumulative = 0;
	bin = REFINE_SCORE_BINS;
	while (--bin >= 0)
	{
		cumulative += bins[bin];
		if (cumulative >= limit)
			return (bin);
	}
	if (cumulative > 0)
		return (0);
	return (-1);
}

static void	mark_top_candidates(t_fractal *fractal, int active_channel,
		double maximum[3], size_t limit)
{
	size_t	selected;
	size_t	index;
	int		threshold;
	int		bin;

	threshold = candidate_threshold(fractal, active_channel, maximum, limit);
	if (threshold < 0)
		return ;
	selected = 0;
	index = 0;
	while (index < (size_t)fractal->size)
	{
		bin = rank_bin(cell_rank(fractal, index, active_channel, maximum));
		if (bin > threshold)
		{
			mark_neighbors(fractal, index);
			selected++;
		}
		index++;
	}
	index = 0;
	while (index < (size_t)fractal->size && selected < limit)
	{
		bin = rank_bin(cell_rank(fractal, index, active_channel, maximum));
		if (bin == threshold)
		{
			mark_neighbors(fractal, index);
			selected++;
		}
		index++;
	}
}

static void	window_bounds(t_comps comps, double bounds[4])
{
	double	first;
	double	second;

	first = comps.move_x + comps.x_cmin * comps.inv_zoom;
	second = comps.move_x + comps.x_cmax * comps.inv_zoom;
	bounds[0] = fmin(first, second);
	bounds[1] = fmax(first, second);
	first = -comps.move_y + comps.y_cmin * comps.inv_zoom;
	second = -comps.move_y + comps.y_cmax * comps.inv_zoom;
	bounds[2] = fmin(first, second);
	bounds[3] = fmax(first, second);
}

static bool	cell_overlaps_window(t_comps comps, size_t index,
		double bounds[4])
{
	t_complex	center;
	double		half_x;
	double		half_y;
	int			x;
	int			y;

	x = (int)(index % (size_t)comps.width);
	y = (int)(index / (size_t)comps.width);
	center = buddha_sample_point(comps, (double)x + 0.5, (double)y + 0.5);
	half_x = fabs(comps.slopex_to) * 0.5;
	half_y = fabs(comps.slopey_to) * 0.5;
	return (center.x + half_x >= bounds[0] && center.x - half_x <= bounds[1]
		&& center.y + half_y >= bounds[2] && center.y - half_y <= bounds[3]);
}

static void	mark_small_window(t_fractal *fractal, size_t limit)
{
	t_comps	comps;
	double	bounds[4];
	size_t	window_cells;
	size_t	index;

	comps = set_comps(fractal, false);
	window_bounds(comps, bounds);
	window_cells = 0;
	index = 0;
	while (index < (size_t)fractal->size)
	{
		if (cell_overlaps_window(comps, index, bounds))
			window_cells++;
		index++;
	}
	if (window_cells > limit)
		return ;
	index = 0;
	while (index < (size_t)fractal->size)
	{
		if (cell_overlaps_window(comps, index, bounds))
			mark_neighbors(fractal, index);
		index++;
	}
}

static size_t	*build_candidate_list(t_fractal *fractal,
		int active_channel, double maximum[3], size_t *candidate_count)
{
	size_t	*indices;
	size_t	limit;
	size_t	index;
	size_t	output;

	memset(fractal->sample_counts, 0,
		(size_t)fractal->size * sizeof(*fractal->sample_counts));
	limit = candidate_limit(fractal);
	mark_top_candidates(fractal, active_channel, maximum, limit);
	mark_small_window(fractal, limit);
	*candidate_count = 0;
	index = 0;
	while (index < (size_t)fractal->size)
		*candidate_count += fractal->sample_counts[index++] != 0;
	if (*candidate_count == 0)
		return (NULL);
	indices = malloc(*candidate_count * sizeof(*indices));
	if (!indices)
	{
		fprintf(stderr, "Buddha map refinement skipped: "
			"candidate allocation failed\n");
		memset(fractal->sample_counts, 0,
			(size_t)fractal->size * sizeof(*fractal->sample_counts));
		return (NULL);
	}
	output = 0;
	index = 0;
	while (index < (size_t)fractal->size)
	{
		if (fractal->sample_counts[index])
			indices[output++] = index;
		fractal->sample_counts[index++] = 0;
	}
	return (indices);
}

static void	add_refinement_stats(t_buddha_pilot_stats *total,
		t_buddha_pilot_stats *worker)
{
	int	channel;

	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
	{
		total->samples[channel] += worker->samples[channel];
		total->eligible[channel] += worker->eligible[channel];
		total->useful[channel] += worker->useful[channel];
		total->visible_hits[channel] += worker->visible_hits[channel];
		total->score_sum[channel] += worker->score_sum[channel];
		total->adaptive_samples[channel]
			+= worker->adaptive_samples[channel];
		total->adaptive_useful[channel]
			+= worker->adaptive_useful[channel];
		total->adaptive_visible_hits[channel]
			+= worker->adaptive_visible_hits[channel];
		total->adaptive_score_sum[channel]
			+= worker->adaptive_score_sum[channel];
	}
	total->refined_cells += worker->refined_cells;
}

static bool	leaf_has_score(t_buddha_leaf *leaf)
{
	int	channel;

	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
		if (isfinite(leaf->cumulative[channel])
			&& leaf->cumulative[channel] > 0.0)
			return (true);
	return (false);
}

static void	finalize_proposal_leaves(t_buddha *buddha, size_t slots)
{
	t_buddha_leaf	*leaf;
	t_buddha_leaf	*shrunk;
	double			area;
	double			mass;
	size_t			input;
	size_t			output;
	int				channel;

	if (!buddha->proposal_leaves)
		return ;
	output = 0;
	input = 0;
	while (input < slots)
	{
		if (leaf_has_score(&buddha->proposal_leaves[input]))
			buddha->proposal_leaves[output++] = buddha->proposal_leaves[input];
		input++;
	}
	buddha->proposal_leaf_count = output;
	if (output == 0)
		return (buddha_clear_importance_proposal(buddha));
	memset(buddha->proposal_leaf_total, 0,
		sizeof(buddha->proposal_leaf_total));
	input = 0;
	while (input < output)
	{
		leaf = &buddha->proposal_leaves[input];
		area = (double)leaf->size * (double)leaf->size;
		channel = -1;
		while (++channel < BUDDHA_CHANNELS)
		{
			mass = leaf->cumulative[channel] * area;
			if (!isfinite(mass) || mass < 0.0)
				mass = 0.0;
			buddha->proposal_leaf_total[channel] += mass;
			leaf->cumulative[channel] = buddha->proposal_leaf_total[channel];
		}
		input++;
	}
	shrunk = realloc(buddha->proposal_leaves,
			output * sizeof(*buddha->proposal_leaves));
	if (shrunk)
		buddha->proposal_leaves = shrunk;
	if (buddha_profile_enabled())
		printf("[buddha] retained leaves=%zu memory=%.3f MiB "
			"mass=[%.6g,%.6g,%.6g]\n", output,
			(double)(output * sizeof(*buddha->proposal_leaves))
			/ (1024.0 * 1024.0), buddha->proposal_leaf_total[0],
			buddha->proposal_leaf_total[1], buddha->proposal_leaf_total[2]);
}

static void	allocate_proposal_leaves(t_fractal *fractal,
		size_t candidate_count)
{
	size_t	slots;

	if (!fractal->buddha->proposal_mixture
		|| candidate_count > SIZE_MAX / REFINE_BEAM_WIDTH)
		return ;
	slots = candidate_count * REFINE_BEAM_WIDTH;
	fractal->buddha->proposal_leaves = malloc(slots
			* sizeof(*fractal->buddha->proposal_leaves));
	if (!fractal->buddha->proposal_leaves)
		fprintf(stderr, "Buddha retained-leaf proposal disabled: "
			"allocation failed\n");
}

void	buddha_refine_importance(t_fractal *fractal, int active_channel)
{
	double	maximum[3];
	size_t	*indices;
	size_t	candidate_count;
	int		workers;
	int		worker;
	t_piece	pieces[fractal->worker_histogram_count];

	if (!fractal->buddha->importance_refinement)
		return ;
	map_maximums(fractal, active_channel, maximum);
	if (!map_needs_refinement(fractal, active_channel, maximum))
		return ;
	indices = build_candidate_list(fractal, active_channel,
			maximum, &candidate_count);
	if (!indices)
		return ;
	allocate_proposal_leaves(fractal, candidate_count);
	workers = fractal->worker_histogram_count;
	memset(pieces, 0, sizeof(pieces));
	worker = -1;
	while (++worker < workers)
	{
		pieces[worker].x_s = (int)((size_t)worker * candidate_count
				/ (size_t)workers);
		pieces[worker].x_e = (int)((size_t)(worker + 1) * candidate_count
				/ (size_t)workers);
		pieces[worker].fractal = fractal;
		pieces[worker].work_indices = indices;
		pieces[worker].leaf_slots = fractal->buddha->proposal_leaves;
		pieces[worker].active_channel = active_channel;
		memcpy(pieces[worker].score_norm, maximum, sizeof(maximum));
		if (pthread_create(&fractal->threads[worker], NULL,
				refine_worker, &pieces[worker]) != 0)
			thread_error(fractal, worker);
	}
	join_threads(fractal->threads, workers, 1);
	worker = -1;
	while (++worker < workers)
		add_refinement_stats(&fractal->buddha->pilot_stats,
			&pieces[worker].pilot_stats);
	finalize_proposal_leaves(fractal->buddha,
		candidate_count * REFINE_BEAM_WIDTH);
	free(indices);
}
