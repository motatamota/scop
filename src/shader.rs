use std::fs;
use std::ffi::CString;

pub struct Shader {
    pub id: u32,
}

impl Shader {
    pub fn new(vert_path: &str, frag_path: &str) -> Self {
        let vert_src = fs::read_to_string(vert_path).expect("Failed to read vertex shader");
        let frag_src = fs::read_to_string(frag_path).expect("Failed to read fragment shader");

        unsafe {
            let vert = compile(gl::VERTEX_SHADER, &vert_src);
            let frag = compile(gl::FRAGMENT_SHADER, &frag_src);

            let id = gl::CreateProgram();
            gl::AttachShader(id, vert);
            gl::AttachShader(id, frag);
            gl::LinkProgram(id);
            gl::DeleteShader(vert);
            gl::DeleteShader(frag);

            Shader { id }
        }
    }

    pub fn use_program(&self) {
        unsafe { gl::UseProgram(self.id); }
    }
}

unsafe fn compile(kind: u32, src: &str) -> u32 {
    let shader = gl::CreateShader(kind);
    let c_src = CString::new(src).unwrap();
    gl::ShaderSource(shader, 1, &c_src.as_ptr(), std::ptr::null());
    gl::CompileShader(shader);
    shader
}
