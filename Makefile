NAME	= scop
CARGO	= $(HOME)/.cargo/bin/cargo

all:
	$(CARGO) build --release
	cp target/release/$(NAME) .

install:
	@echo "Installing system dependencies..."
	sudo apt update
	sudo apt install -y \
		libx11-dev \
		libxinerama-dev \
		libxcursor-dev \
		libxi-dev \
		libxrandr-dev \
		libgl1-mesa-dev
	@echo "Installing Rust..."
	@if ! command -v rustc > /dev/null 2>&1; then \
		curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y; \
		. $(HOME)/.cargo/env; \
	else \
		echo "Rust already installed: $$(rustc --version)"; \
	fi

clean:
	$(CARGO) clean

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re install
