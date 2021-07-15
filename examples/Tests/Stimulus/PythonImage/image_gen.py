import gzip
import math

import numpy as np


class Renderer:

    def reset(self):
        self.fmt = getvar('pixel_buffer_format')
        self.width = int(getvar('pixel_buffer_width'))
        self.height = int(getvar('pixel_buffer_height'))
        self.period = getvar('period')

        with gzip.open('background.gz') as fp:
            self.bg = np.frombuffer(fp.read(), dtype=np.uint8)

        if self.fmt in ('RGB8', 'sRGB8'):
            self.bg = self.bg[np.arange(self.bg.shape[0]) % 4 != 3]
            self.bg = np.frombuffer(self.bg, dtype=np.dtype('B, B, B'))
        elif self.fmt in ('RGBA8', 'sRGBA8'):
            self.bg = np.frombuffer(self.bg, dtype=np.dtype('B, B, B, B'))
        elif self.fmt == 'RGBA16F':
            self.bg = self.bg.astype(np.float16)
            self.bg /= 255.0
            self.bg = np.frombuffer(self.bg, dtype=np.dtype('f2, f2, f2, f2'))
        else:
            raise RuntimeError('Invalid buffer format: %r' % self.fmt)

        self.bg = self.bg.reshape((self.height, self.width))

    def render(self):
        elapsed_time = getvar('elapsed_time')
        pos = 0.5 * (1.0 - math.cos(2.0 * math.pi * elapsed_time / self.period))
        return np.roll(self.bg,
                       (round(self.height * pos), round(self.width * pos)),
                       (0, 1))


r = Renderer()
reset = r.reset
render = r.render
del r
