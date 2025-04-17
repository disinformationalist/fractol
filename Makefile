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

SRC_DIR = ./src
OBJ_DIR = ./obj
INC_DIR = ./inc

#SRCS := \
$(SRC_DIR)/main.c \
$(SRC_DIR)/events.c \
$(SRC_DIR)/render.c \
$(SRC_DIR)/utils.c \
$(SRC_DIR)/math_utils.c \
$(SRC_DIR)/init.c \
$(SRC_DIR)/colors.c \
$(SRC_DIR)/fractal_threads.c \
$(SRC_DIR)/mandelbrot.c \
$(SRC_DIR)/julia.c \
$(SRC_DIR)/thread_utils.c \
$(SRC_DIR)/print_utils.c \
$(SRC_DIR)/barnsley_fern.c \
$(SRC_DIR)/fern_utils.c \
$(SRC_DIR)/set_png_text.c \
$(SRC_DIR)/clean_up.c \
$(SRC_DIR)/complex_ops.c \
$(SRC_DIR)/print_guides.c \
$(SRC_DIR)/buddha/buddha_events.c \
$(SRC_DIR)/buddha/buddhabrot.c \
$(SRC_DIR)/buddha/complex_ops_simd.c \
$(SRC_DIR)/buddha/density_utils.c \
$(SRC_DIR)/buddha/fast_buddha.c \
$(SRC_DIR)/buddha/fullmap_importance.c \
$(SRC_DIR)/buddha/map_importance.c \
$(SRC_DIR)/buddha/render_buddha.c \
$(SRC_DIR)/buddha/set_buddha_colors.c

  
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c) 
#$(wildcard $(SRC_DIR)/*/*/*.c) $(wildcard $(SRC_DIR)/*/*/*/*.c)

#CFLAGS := -Wall -Wextra -Werror -I$(INC_DIR) -Ofast -march=native -w
CFLAGS := -Wall -Wextra -Werror -I$(INC_DIR) -g -O2 -mavx -mavx2 -march=native -Wno-unused-parameter -Wunused-result 
CFLAGS += -Wno-unused-result -Wno-unused-variable
CC := cc



#OBS := $(SRCS:.c=.o)
OBS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)


IMG_PATH = image_processing
IMG_ARCH = $(IMG_PATH)/libimage_processing.a

COLOR_RESET = \033[0m
COLOR_GREEN = \033[1;92m
COLOR_BLUE = \033[1;96m 

MAKEFLAGS += --no-print-directory

define print_colored
	@echo "$(COLOR_GREEN)$(1)$(COLOR_BLUE)$(2)$(COLOR_RESET)$(3)"
endef

.SILENT:

all: $(IMG_ARCH) $(NAME)

$(NAME): $(OBS)
	$(CC) $(OBS)  -L$(IMG_PATH) -limage_processing -lm -Lminilibx-linux -lmlx_Linux -lX11 -lXext -o $@ -lpng
	$(call print_colored, "[SUCCESS]", "./$(NAME)", "Ready")

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(IMG_ARCH):
	$(MAKE) -C $(IMG_PATH)

clean:
	rm -rf $(OBS)
	cd image_processing && make clean

fclean: clean
	rm -rf $(NAME)
	cd image_processing && make fclean
	
re: fclean all

.PHONY: all clean fclean re chaos
