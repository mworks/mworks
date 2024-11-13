import numpy as np
#from scipy import ndimage, signal
#from skimage.filters import gabor_kernel
import matplotlib.pyplot as plt
import tifffile as tfl
import matplotlib as mpl
#import imageio
#import math
#import os
import seaborn as sns
#from ipywidgets import interact, fixed
#import ipywidgets as widgets

r_ = np.r_
a_ = np.asarray
pi = np.pi






##################################




class AngleOutOfBoundsException(Exception):
    pass


class OriNoiseGenerator():

    def __init__(self, nrows, ncols,
                 f0=1, sigma_f=1, theta0=0, sigma_theta=1,
                 mask_sigma=10):
        """
        Notes: we don't save the parameters being passed in unless we need them.  Can always
        add as attributes later as needed.
        """
        self.nrows = nrows
        self.ncols = ncols
        self.theta0 = theta0
        self._fmask = self._create_fft_mask(nrows, ncols, f0, sigma_f, theta0, sigma_theta)
        self._gabormask = self._create_gabor_mask(nrows, ncols, mask_sigma)

    def _create_gabor_mask(self, nrows, ncols, mask_sigma):
        """
        Generate the Gaussian mask to create the gabor patch. Only needs to be called once.
        Returns: gabor mask image
        """
        # coord vectors
        x = np.linspace(-nrows // 2, nrows // 2, nrows)
        y = np.linspace(-ncols // 2, ncols // 2, ncols)
        X, Y = np.meshgrid(x, y)

        r = np.sqrt(X ** 2 + Y ** 2)
        return np.exp(-(1. / 2) * ((r - 0) / mask_sigma) ** 2)

    def _create_fft_mask(self, nrows, ncols, f0, sigma_f, theta0, sigma_theta):
        """
        Generate the fft mask image (in fourier space).  Only needs to be called once.
        Returns: mask image, shape (nrows,ncols)
        """
        # coord vectors
        y = np.linspace(-nrows // 2, nrows // 2, nrows)
        x = np.linspace(-ncols // 2, ncols // 2, ncols)
        X, Y = np.meshgrid(x, y)

        # fxns from Cullen paper to calculate circular guassian mask
        _wrap = lambda thetas: np.mod(thetas,2*pi)-pi
        def Sn(fx, fy, f0, sigma_f, theta0, sigma_theta):
            """fx,fy vectors, the rest are params"""
            fr = np.sqrt(fx ** 2 + fy ** 2)
            theta = np.arctan2(fy, fx)
            G_r = (np.exp(-(1./2) * ((fr - f0) / sigma_f) ** 2))
            G_theta = (np.exp(-(1./2) * (_wrap(theta - theta0) / sigma_theta) ** 2)
                       + np.exp(-(1./2) * (_wrap(theta - (theta0 + pi)) / sigma_theta) ** 2))
            return G_r * G_theta

        if theta0 > pi:
            raise AngleOutOfBoundsException("Angle (in radians) not in range [0, pi]")
        if sigma_theta > pi/2:
            raise AngleOutOfBoundsException("sigma_theta should be in range [0, pi/2], larger causes overlap and homogeneity")
        fmask = Sn(X, Y, f0=f0, sigma_f=sigma_f, theta0=theta0, sigma_theta=sigma_theta)
        return fmask

    def _make_noise_frame(self):
        """
        Returns:
            Filtered noise frame, norm to mean 0 and max[-1,1] as described in the Mullen paper.
        Notes:
            - this normalization can result in (slight) changes in contrast per frame,
            We may wish to normalize by one constant for each set of noise frames.
            - Note that norm='ortho' doesn't help because the amount of smoothing changes the max
            amp.
        """
        # create and reshape noise
        noise1D = np.random.uniform(0, 1, size=self.ncols * self.nrows)
        noise2D = noise1D.reshape((self.nrows, self.ncols))
        ftimage = np.fft.fftshift(np.fft.fft2(noise2D))  # 2d DFT and centre the frequencies
        ftimagep = ftimage * self._fmask

        noisefr = np.abs(np.fft.ifft2(ftimagep))  # inverse transform
        # normalize as described above
        noisefr = noisefr - noisefr.mean()
        maxamp = np.max(np.abs(noisefr))
        noisefr = noisefr / maxamp
        return noisefr

    def _make_gabor_noise_frame(self):
        """
        Generate a stimulus noise frame and apply a gabor mask in frequency domain
        Returns: image of gabor patch noise stimulus
        """
        fr = self._make_noise_frame()  # ranges from 0-1
        return fr * self._gabormask

    def generate_gabor_noise_movie(self, frames, save=False, savename=''):
        movie = []
        for _ in range(frames):
            movie.append(self._make_gabor_noise_frame())
        if save:
            if not savename:
                raise ValueError('Empty string "savename". Specify a save path with file name.')
            else:
                tfl.imsave(savename, a_(movie, dtype='Float32'))
                print('File saved at location: ', savename)
        return movie

    def plot_interactive_movie(self, movie, fig_size, Frame):
        sns.set_style('darkgrid')
        # generate image
        fig = plt.figure()
        plt.imshow(movie[Frame], cmap='gray', clim=r_[-1, 1] * 1.0)
        # set figure elements
        plt.colorbar()
        fig.set_size_inches(w=fig_size[0], h=fig_size[1])
        fig.tight_layout()