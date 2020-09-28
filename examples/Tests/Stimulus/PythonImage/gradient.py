import numpy as np


class GradientManager:

    def __init__(self):
        self.gradients = {}

    def __len__(self):
        return len(self.gradients)

    def create(self):
        width = int(getvar('pixel_buffer_width'))
        height = int(getvar('pixel_buffer_height'))

        linear = np.linspace(0.0, 1.0, width)
        srgb = np.where(linear < 0.0031308,
                        linear * 12.92,
                        (linear ** (1.0/2.4)) * 1.055 - 0.055)

        linear_uint8 = np.around(linear * 255.0).astype(np.uint8)
        srgb_uint8 = np.around(srgb * 255.0).astype(np.uint8)
        linear_float16 = linear.astype(np.float16)

        def make_image(vals, cpp):
            vals = np.repeat(vals, cpp)
            if cpp == 4:
                alpha = (1.0 if vals.dtype.kind == 'f' else 255)
                vals[3:len(vals)+3:4] = alpha
            vals = np.broadcast_to(vals, (height, width * cpp))
            return np.ascontiguousarray(vals)

        self.gradients.update({
            'RGB8': make_image(linear_uint8, 3),
            'sRGB8': make_image(srgb_uint8, 3),
            'RGBA8': make_image(linear_uint8, 4),
            'sRGBA8': make_image(srgb_uint8, 4),
            'RGBA16F': make_image(linear_float16, 4),
            })

    def pop(self, fmt):
        return self.gradients.pop(fmt, None)


gradients = GradientManager()
