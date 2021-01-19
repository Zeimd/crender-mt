
Software renderer test application

Usage

    soft-render-test {image.bmp}

    The optional texture file must be a BMP file in 24-bit RGB, and header type must be Windows bitmap version 3.

    If this file is not given, program tries to load "texture.bmp". If that fails, internal missing texture
    fallback is shown.

    The fallback is an orange X on white background.


    If a texture file is loaded, the texture sampling mode is set to bilinear. If the missing texture fallback
    is used, texture sampling is set to nearest neighbour to make the image immediately recognizable.

Controls

    w,a,s,d : standard game movement controls for left/right and forward/backward

    q,e : move up/down

    arrow keys: camera rotation pitch (up/down) and yaw (left/right)

    home, end : roll camera

    1,2 : rotate drawn surface

    F1 : change texture sampling to nearest neighbor

    F2 : change texture sampling to bilinear

    ESC : quit



