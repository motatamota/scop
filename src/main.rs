mod window;
mod shader;
mod mesh;
mod matrix;
mod texture;

fn main() {
    let mut win = window::Window::new(800, 600, "scop");
    win.run();
}
