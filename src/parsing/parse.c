/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saeby <saeby@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/22 12:02:02 by saeby             #+#    #+#             */
/*   Updated: 2023/03/30 14:35:29 by saeby            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"

int	get_door(char *path, t_meta *meta)
{
	meta->input.door_path = ft_strdup(path);
	if (!meta->input.door_path)
		free_and_exit("Error allocating memory.", meta);
	meta->input.door_path[ft_strlen(path) - 1] = 0;
	if (open(meta->input.door_path, O_RDONLY) < 0)
		free_and_exit("Unable to open Door texture.", meta);
	return (0);
}

int	get_map_info(char *in_file, t_meta *meta)
{
	int		fd;
	char	*line;
	char	*tmp;

	fd = open(in_file, O_RDONLY);
	line = skip_textures(fd);
	while (1)
	{
		tmp = line;
		line = ft_strtrim(line, "\n");
		free(tmp);
		if (line_nok(line, meta))
		{
			close(fd);
			free(line);
			free_and_exit("Unauthorized character found.", meta);
		}
		free(line);
		meta->input.m++;
		line = get_next_line(fd);
		if (!line)
			break ;
	}
	close(fd);
	return (0);
}

void	print_map(t_meta *meta)
{
	int	x;
	int	y;

	y = 0;
	while (y < meta->input.m)
	{
		x = 0;
		while (x < meta->input.n)
		{
			ft_putchar_fd(meta->input.map[x + y * meta->input.n], 1);
			x++;
		}
		ft_putchar_fd('\n', 1);
		y++;
	}
}

static void	free_parse(t_meta *meta)
{
	free(meta->input.f_color);
	meta->input.f_color = 0;
	free(meta->input.c_color);
	meta->input.c_color = 0;
}

// parsing done in two steps
// 1: get all information as strings 
// (textures paths, floor/ceiling color as r,g,b, etc)
// 2: convert the values in the expected format and free 
// the memory allocated for the different strings
int	parse_map(char *in_file, t_meta *meta)
{
	int		fd;

	fd = open(in_file, O_RDONLY);
	if (fd < 0)
		free_and_exit("Unable to open map file.", meta);
	close(fd);
	if (check_filename(in_file))
		free_and_exit("Wrong map filename (has to end with .cub", meta);
	meta->input.textures = (t_image *) malloc(TEXTURES_COUNT * sizeof(t_image));
	if (!meta->input.textures)
		free_and_exit("Error when allocating textures memory.", meta);
	if (parse_textures(in_file, meta))
		free_and_exit("Unable to parse one of the wall texture.", meta);
	convert_colors(meta);
	free_parse(meta);
	get_map_info(in_file, meta);
	meta->input.map = ft_calloc((unsigned int)(meta->input.n * meta->input.m), \
				sizeof(char));
	if (!meta->input.map)
		free_and_exit("Error when allocating map memory.", meta);
	fill_map(in_file, meta);
	check_map(meta);
	if (meta->input.p_i == 0 || meta->input.p_j == 0)
		free_and_exit("No start position found.", meta);
	return (0);
}
