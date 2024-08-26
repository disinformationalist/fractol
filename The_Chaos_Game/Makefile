# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/02/23 12:09:08 by jhotchki          #+#    #+#              #
#    Updated: 2024/02/23 12:09:11 by jhotchki         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := chaos

SRCS := chaos.c colors.c events.c events_rules.c events_mouse_super.c events_other.c init.c rules.c \
 utils.c print_utils.c print_utils_2.c sample.c ft_atoi.c
  
CFLAGS := -Wall -Wextra -Werror -Ofast -march=native -w

CC := cc

OBS := $(SRCS:.c=.o)

COLOR_RESET = \033[0m
COLOR_GREEN = \033[1;92m
COLOR_BLUE = \033[1;96m

define print_colored
	@echo "$(COLOR_GREEN)$(1)$(COLOR_BLUE)$(2)$(COLOR_RESET)$(3)"
endef

.SILENT:

all: $(NAME)

$(NAME): $(OBS)
	$(CC) $(OBS) -lm -L../minilibx-linux -lmlx_Linux -lX11 -lXext -o $(NAME)
	cp $(NAME) ../$(NAME)
	$(call print_colored, "[SUCCESS]", "./$(NAME)", "Ready")
	
clean:
	rm -rf $(OBS)

fclean: clean
	rm -rf	$(NAME)
	
re: fclean all

.PHONY: all clean fclean re
