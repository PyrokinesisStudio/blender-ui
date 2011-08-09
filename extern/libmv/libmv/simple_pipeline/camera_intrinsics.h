// Copyright (c) 2011 libmv authors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef LIBMV_SIMPLE_PIPELINE_CAMERA_INTRINSICS_H_
#define LIBMV_SIMPLE_PIPELINE_CAMERA_INTRINSICS_H_

#include <Eigen/Core>
typedef Eigen::Matrix<double, 3, 3> Mat3;

namespace libmv {

struct Offset;

class CameraIntrinsics {
 public:
  CameraIntrinsics();
  ~CameraIntrinsics();

  const Mat3 &K()                 const { return K_;            }
  // FIXME(MatthiasF): these should be CamelCase methods
  double      focal_length()      const { return K_(0, 0);      }
  double      focal_length_x()    const { return K_(0, 0);      }
  double      focal_length_y()    const { return K_(1, 1);      }
  double      principal_point_x() const { return K_(0, 2);      }
  double      principal_point_y() const { return K_(1, 2);      }
  int         image_width()       const { return image_width_;  }
  int         image_height()      const { return image_height_; }
  double      k1()                const { return k1_; }
  double      k2()                const { return k2_; }
  double      k3()                const { return k3_; }
  double      p1()                const { return p1_; }
  double      p2()                const { return p2_; }

  /// Set the entire calibration matrix at once.
  void SetK(const Mat3 new_k) {
    K_ = new_k;
  }

  /// Set both x and y focal length in pixels.
  void SetFocalLength(double focal_x, double focal_y) {
    K_(0, 0) = focal_x;
    K_(1, 1) = focal_y;
  }

  void SetPrincipalPoint(double cx, double cy) {
    K_(0, 2) = cx;
    K_(1, 2) = cy;
  }

  void SetImageSize(int width, int height) {
    image_width_ = width;
    image_height_ = height;
  }

  void SetRadialDistortion(double k1, double k2, double k3 = 0) {
    k1_ = k1;
    k2_ = k2;
    k3_ = k3;
  }

  void SetTangentialDistortion(double p1, double p2) {
    p1_ = p1;
    p2_ = p2;
  }

  /*!
      Apply camera intrinsics to the normalized point to get image coordinates.

      This applies the camera intrinsics to a point which is in normalized
      camera coordinates (i.e. the principal point is at (0, 0)) to get image
      coordinates in pixels.
  */
  void ApplyIntrinsics(double normalized_x, double normalized_y,
                       double *image_x, double *image_y) const;

  /*!
      Invert camera intrinsics on the image point to get normalized coordinates.

      This reverses the effect of camera intrinsics on a point which is in image
      coordinates to get normalized camera coordinates.
  */
  void InvertIntrinsics(double image_x, double image_y,
                        double *normalized_x, double *normalized_y) const;

  /*!
      Distort an image using the current camera instrinsics

      The distorted image is computed in \a dst using samples from \a src.
      both buffers should be \a width x \a height x \a channels sized.

      \note This is the reference implementation using floating point images.
  */
  void Distort(const float* src, float* dst,
               int width, int height, int channels);
  /*!
      Distort an image using the current camera instrinsics

      The distorted image is computed in \a dst using samples from \a src.
      both buffers should be \a width x \a height x \a channels sized.

      \note This version is much faster.
  */
  void Distort(const unsigned char* src, unsigned char* dst,
               int width, int height, int channels);
  /*!
      Undistort an image using the current camera instrinsics

      The undistorted image is computed in \a dst using samples from \a src.
      both buffers should be \a width x \a height x \a channels sized.

      \note This is the reference implementation using floating point images.
  */
  void Undistort(const float* src, float* dst,
                 int width, int height, int channels);
  /*!
      Undistort an image using the current camera instrinsics

      The undistorted image is computed in \a dst using samples from \a src.
      both buffers should be \a width x \a height x \a channels sized.

      \note This version is much faster.
  */
  void Undistort(const unsigned char* src, unsigned char* dst,
                 int width, int height, int channels);

 private:
  template<typename WarpFunction> void ComputeLookupGrid(Offset* grid, int width, int height);

  // The traditional intrinsics matrix from x = K[R|t]X.
  Mat3 K_;

  // This is the size of the image. This is necessary to, for example, handle
  // the case of processing a scaled image.
  int image_width_;
  int image_height_;

  // OpenCV's distortion model with third order polynomial radial distortion
  // terms and second order tangential distortion. The distortion is applied to
  // the normalized coordinates before the focal length, which makes them
  // independent of image size.
  double k1_, k2_, k3_, p1_, p2_;

  Offset* distort_;
  Offset* undistort_;
};

}  // namespace libmv

#endif  // LIBMV_SIMPLE_PIPELINE_CAMERA_INTRINSICS_H_
