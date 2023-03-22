/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   raycast.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tplanes <tplanes@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/19 12:32:20 by tplanes           #+#    #+#             */
/*   Updated: 2023/03/21 12:38:59 by tplanes          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"

static void	set_closest_wall(t_raycast *ray, t_player *play, t_input *inp);

static void	get_h_wall(float th_ray, float *h_d, t_fpt2 *h_wall, t_meta *meta);

static void	get_v_wall(float th_ray, float *v_d, t_fpt2 *v_wall, t_meta *meta);

static int	is_wall_found(t_fpt2 *wall, t_fpt2 *offset, t_input *inp);

/*
v_dist (h_dist) is the distance to the closest vertical (horizontal) 
wall (if reachable), at vector positions v_wall (h_wall). 
v_dist and h_dist are set to high values by default in case wall not reachable.
*/
void	compute_rays(t_input *inp, t_player *play, t_meta *meta)
{
	t_raycast	ray;

	ray.p0.x = play -> x;
	ray.p0.y = play -> y;
	ray.i = -1;
	while (++ray.i < N_RAY)
	{
		ray.v_dist = (inp -> m + inp -> n) * PIX_PER_BLOCK;
		ray.h_dist = (inp -> m + inp -> n) * PIX_PER_BLOCK;
		ray.theta_ray = play -> theta - FOV / 2 + FOV / (N_RAY - 1) * ray.i;
		if (ray.theta_ray < 0)
			ray.theta_ray += 2 * PI;
		else if (ray.theta_ray > 2 * PI)
			ray.theta_ray = fmodf(ray.theta_ray, 2 * PI);
		get_h_wall(ray.theta_ray, &ray.h_dist, &ray.h_wall, meta);
		get_v_wall(ray.theta_ray, &ray.v_dist, &ray.v_wall, meta);
		set_closest_wall(&ray, play, inp);
		if (SHOW_RAYS)
			draw_line_image(&ray.p0, &ray.p1, &meta -> im2, 255 << 16);
		draw_column(ray.i, ray.wall_dist, ray.ptr_text, meta);
	}
	return ;
}

/*
off: offset vector to next possible wall intersection
theta > pi means looking up (else looking down)
For first intersection, y-eps to find wall in future casting to int, 
because wall one pix behind multiple of PixPerBlock.
*/
static void	get_h_wall(float th_ray, float *h_d, t_fpt2 *h_wall, t_meta *meta)
{
	t_fpt2		off;
	float		itan;
	t_player	*p;

	p = &meta -> play;
	itan = 1.0f / tanf(th_ray);
	if (th_ray > PI)
	{	
		h_wall -> y = (p -> y / PIX_PER_BLOCK) * PIX_PER_BLOCK - 0.001f;
		off.y = -PIX_PER_BLOCK;
	}
	else
	{
		h_wall -> y = (p -> y / PIX_PER_BLOCK + 1) * PIX_PER_BLOCK;
		off.y = PIX_PER_BLOCK;
	}
	h_wall -> x = p -> x + (h_wall -> y - p -> y) * itan;
	off.x = off.y * itan;
	if (is_wall_found(h_wall, &off, &meta -> input))
		*h_d = sqrtf(powf(h_wall -> x - (float)p -> x, 2)
				+ powf(h_wall -> y - (float)p -> y, 2));
	return ;
}

/*
off: offset vector to next possible wall intersection
theta > pi/2 and < 3pi/2 means looking left (else looking right)
For first intersection, x-eps to find wall in future casting to int, 
because wall one pix behind multiple of PixPerBlock.
*/
static void	get_v_wall(float th_ray, float *v_d, t_fpt2 *v_wall, t_meta *meta)
{
	t_fpt2		off;
	float		tan;
	t_player	*p;

	p = &meta -> play;
	tan = tanf(th_ray);
	if (th_ray > PI / 2 && th_ray < 3 * PI / 2)
	{	
		v_wall -> x = (p -> x / PIX_PER_BLOCK) * PIX_PER_BLOCK - 0.001f;
		off.x = -PIX_PER_BLOCK;
	}
	else
	{
		v_wall -> x = (p -> x / PIX_PER_BLOCK + 1) * PIX_PER_BLOCK;
		off.x = PIX_PER_BLOCK;
	}	
	v_wall -> y = p -> y + (v_wall -> x - p -> x) * tan;
	off.y = off.x * tan;
	if (is_wall_found(v_wall, &off, &meta -> input))
		*v_d = sqrtf(powf(v_wall -> x - (float)p -> x, 2)
				+ powf(v_wall -> y - (float)p -> y, 2));
	return ;
}

// Increase ray until finds wall intersection or get outta map
static int	is_wall_found(t_fpt2 *wall, t_fpt2 *offset, t_input *inp)
{
	int	i;
	int	j;

	while (1)
	{
		i = (int)wall -> y / PIX_PER_BLOCK;
		j = (int)wall -> x / PIX_PER_BLOCK;
		if (i < 0 || i > inp -> m - 1 || j < 0 || j > inp -> n -1)
			break ;
		if (inp -> map[i][j] == 1)
			return (1);
		wall -> y += offset -> y;
		wall -> x += offset -> x;
	}
	return (0);
}

/*
add horizontal offset to texture ptr
sometimes seems like glitchy column appears,
need to investigate
*/
static void	set_closest_wall(t_raycast *ray, t_player *play, t_input *inp)
{
	if (ray -> h_dist < ray -> v_dist)
	{
		ray -> p1.x = (int)round(ray -> h_wall.x);
		ray -> p1.y = (int)round(ray -> h_wall.y);
		ray -> wall_dist = ray -> h_dist
			* cosf(fabsf(ray -> theta_ray - play -> theta));
		if (ray -> theta_ray > PI)
			ray -> ptr_text = inp -> text[1] + ray -> p1.x % PIX_PER_BLOCK;
		else
			ray -> ptr_text = inp -> text[0]
				+ PIX_PER_BLOCK - 1 - (ray -> p1.x % PIX_PER_BLOCK);
	}
	else
	{
		ray -> p1.x = (int)round(ray -> v_wall.x);
		ray -> p1.y = (int)round(ray -> v_wall.y);
		ray -> wall_dist = ray -> v_dist
			* cosf(fabsf(ray -> theta_ray - play -> theta));
		if (ray -> theta_ray < 0.5 * PI || ray -> theta_ray > 1.5 * PI)
			ray -> ptr_text = inp -> text[3] + ray -> p1.y % PIX_PER_BLOCK;
		else
			ray -> ptr_text = inp -> text[2]
				+ PIX_PER_BLOCK - 1 - (ray -> p1.y % PIX_PER_BLOCK);
	}
	return ;
}
/*
static inline float dist(float x0, float y0, float x1, float y1)
{
	return (sqrtf(powf(x1 - x0, 2) + powf(y1 - y0, 2)));
}

void	draw_box_around_image(t_image *im)
{	
	int	i;

	i = 0;
	while (i < im -> nx)
	{
		f_put_pixel(im, 0, i, WHITE);
		f_put_pixel(im, im -> ny - 1, i, WHITE);
		i++;
	}
	i = 0;
	while (i < im -> ny)
	{
		f_put_pixel(im, i, 0, WHITE);
		f_put_pixel(im, i, im -> nx - 1, WHITE);
		i++;
	}
	return ;
}*/
