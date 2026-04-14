NAME		= scop

CXX		= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17

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

.PHONY: all clean fclean re install
