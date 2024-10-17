import os
import sys

import numpy as np
import tifffile
from tifffile.tifffile import TIFF


def main():
    output_dir = sys.argv[1]
    num_frames = int(sys.argv[2])
    width = int(sys.argv[3])
    height = int(sys.argv[4])

    with open('LinearGray.icc', 'rb') as fp:
        icc_profile_data = fp.read()

    for i in range(num_frames):
        filename = os.path.join(output_dir, 'frame_%03d.tif' % i)
        print('Creating', filename)

        data = np.random.uniform(0, 1, size=width*height).astype(np.float32)
        data = data.reshape((width, height))

        tifffile.imsave(filename, data, extratags=[
            (TIFF.TAG_NAMES['InterColorProfile'],
             'B',
             len(icc_profile_data),
             icc_profile_data,
             True)
            ])


if __name__ == '__main__':
    main()
