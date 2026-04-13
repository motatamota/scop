// .obj ファイルのパーサーと頂点バッファ管理

pub struct Mesh {
    pub vao: u32,
    pub vbo: u32,
    pub vertex_count: i32,
}

impl Mesh {
    pub fn from_obj(path: &str) -> Self {
        let src = std::fs::read_to_string(path).expect("Failed to read .obj file");
        let vertices = parse_obj(&src);
        let vertex_count = (vertices.len() / 3) as i32;

        let (mut vao, mut vbo) = (0u32, 0u32);
        unsafe {
            gl::GenVertexArrays(1, &mut vao);
            gl::GenBuffers(1, &mut vbo);

            gl::BindVertexArray(vao);
            gl::BindBuffer(gl::ARRAY_BUFFER, vbo);
            gl::BufferData(
                gl::ARRAY_BUFFER,
                (vertices.len() * std::mem::size_of::<f32>()) as isize,
                vertices.as_ptr() as *const _,
                gl::STATIC_DRAW,
            );

            // position: location = 0
            gl::VertexAttribPointer(0, 3, gl::FLOAT, gl::FALSE, 3 * 4, std::ptr::null());
            gl::EnableVertexAttribArray(0);

            gl::BindVertexArray(0);
        }

        Mesh { vao, vbo, vertex_count }
    }

    pub fn draw(&self) {
        unsafe {
            gl::BindVertexArray(self.vao);
            gl::DrawArrays(gl::TRIANGLES, 0, self.vertex_count);
            gl::BindVertexArray(0);
        }
    }
}

fn parse_obj(src: &str) -> Vec<f32> {
    let mut positions: Vec<[f32; 3]> = Vec::new();
    let mut vertices: Vec<f32> = Vec::new();

    for line in src.lines() {
        let mut parts = line.split_whitespace();
        match parts.next() {
            Some("v") => {
                let x: f32 = parts.next().unwrap_or("0").parse().unwrap_or(0.0);
                let y: f32 = parts.next().unwrap_or("0").parse().unwrap_or(0.0);
                let z: f32 = parts.next().unwrap_or("0").parse().unwrap_or(0.0);
                positions.push([x, y, z]);
            }
            Some("f") => {
                let indices: Vec<usize> = parts
                    .map(|p| {
                        p.split('/').next().unwrap().parse::<usize>().unwrap_or(1) - 1
                    })
                    .collect();
                // 三角形に分割（fan triangulation）
                for i in 1..indices.len().saturating_sub(1) {
                    for &idx in &[indices[0], indices[i], indices[i + 1]] {
                        if let Some(v) = positions.get(idx) {
                            vertices.extend_from_slice(v);
                        }
                    }
                }
            }
            _ => {}
        }
    }
    vertices
}
