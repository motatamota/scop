// テクスチャ読み込み（PPM形式を自前で解析）

pub struct Texture {
    pub id: u32,
}

impl Texture {
    pub fn from_ppm(path: &str) -> Self {
        let data = std::fs::read(path).expect("Failed to read texture file");
        let (pixels, width, height) = parse_ppm(&data);

        let mut id = 0u32;
        unsafe {
            gl::GenTextures(1, &mut id);
            gl::BindTexture(gl::TEXTURE_2D, id);
            gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::REPEAT as i32);
            gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::REPEAT as i32);
            gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR as i32);
            gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR as i32);
            gl::TexImage2D(
                gl::TEXTURE_2D, 0, gl::RGB as i32,
                width as i32, height as i32, 0,
                gl::RGB, gl::UNSIGNED_BYTE,
                pixels.as_ptr() as *const _,
            );
        }
        Texture { id }
    }

    pub fn bind(&self) {
        unsafe { gl::BindTexture(gl::TEXTURE_2D, self.id); }
    }
}

fn parse_ppm(data: &[u8]) -> (Vec<u8>, usize, usize) {
    let text = std::str::from_utf8(data).unwrap_or("");
    let mut lines = text.lines().filter(|l| !l.starts_with('#'));

    let magic  = lines.next().unwrap_or("").trim();
    assert_eq!(magic, "P6", "Only binary PPM (P6) is supported");

    let dims: Vec<usize> = lines.next().unwrap_or("0 0")
        .split_whitespace()
        .map(|v| v.parse().unwrap_or(0))
        .collect();
    let (width, height) = (dims[0], dims[1]);
    lines.next(); // max value

    let header_end = data.windows(2).position(|w| w == b"\n" && {
        // skip 3 header lines
        true
    }).unwrap_or(0);
    let pixel_start = data[header_end + 1..].iter().position(|_| true).unwrap_or(0) + header_end + 1;

    let pixels = data[pixel_start..pixel_start + width * height * 3].to_vec();
    (pixels, width, height)
}
