NAME		= scop

CXX		= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17

# DEBUG=1 でデバッグログを有効化 (make DEBUG=1)
ifeq ($(DEBUG),1)
CXXFLAGS	+= -DDEBUG -g
endif

SRC_DIR		= src
OBJ_DIR		= obj
INC_DIR		= includes

SRCS		= $(wildcard $(SRC_DIR)/*.cpp)
OBJS		= $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

INCLUDES	= -I$(INC_DIR)
LIBS		= -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

install:
	@echo "Installing system dependencies..."
	sudo apt update
	sudo apt install -y \
		libglfw3-dev \
		libgl1-mesa-dev \
		libx11-dev

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

bonus:
	touch bonus

# デバッグビルド: 必ず作り直して -DDEBUG -g 付きでビルド
debug: fclean
	$(MAKE) DEBUG=1 $(NAME)
	rm -f bonus

.PHONY: all clean fclean re install debug
