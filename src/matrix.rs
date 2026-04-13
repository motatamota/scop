// 4x4行列（列優先）と変換関数（自前実装）

pub type Mat4 = [f32; 16];

pub fn identity() -> Mat4 {
    [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    ]
}

pub fn multiply(a: &Mat4, b: &Mat4) -> Mat4 {
    let mut result = [0.0f32; 16];
    for row in 0..4 {
        for col in 0..4 {
            for k in 0..4 {
                result[col * 4 + row] += a[k * 4 + row] * b[col * 4 + k];
            }
        }
    }
    result
}

pub fn translate(x: f32, y: f32, z: f32) -> Mat4 {
    let mut m = identity();
    m[12] = x;
    m[13] = y;
    m[14] = z;
    m
}

pub fn scale(x: f32, y: f32, z: f32) -> Mat4 {
    let mut m = identity();
    m[0]  = x;
    m[5]  = y;
    m[10] = z;
    m
}

pub fn rotate_y(angle: f32) -> Mat4 {
    let c = angle.cos();
    let s = angle.sin();
    let mut m = identity();
    m[0]  =  c;
    m[2]  = -s;
    m[8]  =  s;
    m[10] =  c;
    m
}

pub fn perspective(fov: f32, aspect: f32, near: f32, far: f32) -> Mat4 {
    let f = 1.0 / (fov / 2.0).tan();
    let mut m = [0.0f32; 16];
    m[0]  = f / aspect;
    m[5]  = f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0;
    m[14] = (2.0 * far * near) / (near - far);
    m
}
