use glfw::{Action, Context, Key};

pub struct Window {
    glfw:   glfw::Glfw,
    window: glfw::PWindow,
    events: glfw::GlfwReceiver<(f64, glfw::WindowEvent)>,
}

impl Window {
    pub fn new(width: u32, height: u32, title: &str) -> Self {
        let mut glfw = glfw::init(glfw::fail_on_errors).unwrap();

        glfw.window_hint(glfw::WindowHint::ContextVersion(4, 1));
        glfw.window_hint(glfw::WindowHint::OpenGlProfile(glfw::OpenGlProfileHint::Core));

        let (mut window, events) = glfw
            .create_window(width, height, title, glfw::WindowMode::Windowed)
            .expect("Failed to create GLFW window");

        window.make_current();
        window.set_key_polling(true);

        gl::load_with(|s| window.get_proc_address(s) as *const _);

        Window { glfw, window, events }
    }

    pub fn run(&mut self) {
        while !self.window.should_close() {
            self.glfw.poll_events();
            let events: Vec<_> = glfw::flush_messages(&self.events).collect();
            for (_, event) in events {
                self.handle_event(event);
            }

            unsafe {
                gl::ClearColor(0.1, 0.1, 0.1, 1.0);
                gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
            }

            self.window.swap_buffers();
        }
    }

    fn handle_event(&mut self, event: glfw::WindowEvent) {
        if let glfw::WindowEvent::Key(Key::Escape, _, Action::Press, _) = event {
            self.window.set_should_close(true);
        }
    }
}
