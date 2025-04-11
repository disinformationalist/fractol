# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/02/20 10:07:54 by jhotchki          #+#    #+#              #
#    Updated: 2024/02/21 05:19:13 by jhotchki         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := fractol

SRCS := main.c events.c render.c utils.c math_utils.c init.c colors.c fractal_threads.c \
mandelbrot.c julia.c buddhabrot.c utils_3.c density_utils.c
  
CFLAGS := -Wall -Wextra -Werror -Ofast -march=native -w

CC := cc

OBS := $(SRCS:.c=.o)

COLOR_RESET = \033[0m
COLOR_GREEN = \033[1;92m
COLOR_BLUE = \033[1;96m 

MAKEFLAGS += --no-print-directory

define print_colored
	@echo "$(COLOR_GREEN)$(1)$(COLOR_BLUE)$(2)$(COLOR_RESET)$(3)"
endef

.SILENT:

all: $(NAME)

$(NAME): $(OBS)
	$(CC) $(OBS) -lm -Lminilibx-linux -lmlx_Linux -lX11 -lXext -o $@
	$(call print_colored, "[SUCCESS]", "./$(NAME)", "Ready")
#$(CC) $(OBS)  -L$(IMG_PATH) -limage_processing -lm -Lminilibx-linux -lmlx_Linux -lX11 -lXext -o $@ -lpng
bonus: 
	cd The_Chaos_Game && make

clean:
	rm -rf $(OBS)
	cd The_Chaos_Game  && make clean

fclean: clean
	rm -rf $(NAME)
	rm -rf chaos
	cd The_Chaos_Game  && make fclean
	
re: fclean all

.PHONY: all clean fclean re bonus
