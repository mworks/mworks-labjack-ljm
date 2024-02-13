import numpy as np


def get_data(period):
    data_interval = getvar('data_interval')
    time = np.linspace(0.0,
                       2.0 * np.pi,
                       int(1e6 * period / data_interval),
                       endpoint = False)

    output_min = getvar('output_min')
    output_max = getvar('output_max')
    amplitude = (output_max - output_min) / 2
    offset = (output_min + output_max) / 2
    data = -amplitude * np.cos(time) + offset

    return data
