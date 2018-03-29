import sys

from matplotlib import pyplot
import numpy

sys.path.insert(0, '/Library/Application Support/MWorks/Scripting/Python')
from mworks.data import MWKFile


def selection_counts(filename):
    with MWKFile(filename) as f:
        r_codec = f.reverse_codec
        red_code = r_codec['red_selected']
        green_code = r_codec['green_selected']
        blue_code = r_codec['blue_selected']

        red_count = 0
        green_count = 0
        blue_count = 0

        for evt in f.get_events_iter(codes=[red_code, green_code, blue_code]):
            if evt.data:
                if evt.code == red_code:
                    red_count += 1
                elif evt.code == green_code:
                    green_count += 1
                else:
                    assert evt.code == blue_code
                    blue_count += 1

        index = numpy.arange(3)
        pyplot.bar(index,
                   [red_count, green_count, blue_count],
                   0.5,
                   color = ['r', 'g', 'b'],
                   align = 'center')
        pyplot.xticks(index, ['Red', 'Green', 'Blue'])
        pyplot.title('Selection Counts')
        pyplot.show()


if __name__ == '__main__':
    selection_counts(sys.argv[1])
