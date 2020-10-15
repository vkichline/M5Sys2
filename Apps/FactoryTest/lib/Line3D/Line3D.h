#pragma once

#include <Arduino.h>
#include "utility/Sprite.h"

typedef struct Point_3d_t {
  double x;
  double y;
  double z;
} Point_3d_t;

typedef struct Line_3d_t {
  Point_3d_t start_point;
  Point_3d_t end_point;
} Line_3d_t;

typedef struct Point_2d_t {
  double x;
  double y;
} Point_2d_t;

class Line3D {
  public:
    Line3D(/* args */);
    ~Line3D();

    void setZeroOffset(int offsetX, int offsetY) { _zeroXoffset = offsetX;  _zeroYOffset = offsetY;}
    bool point3Dto2D(Point_3d_t *source, Point_2d_t *point);
    bool point2DToDisPoint(Point_2d_t *point, uint8_t *x, uint8_t *y);
    bool printLine3D(TFT_eSprite *display, Line_3d_t *line, uint32_t color);
    void RotatePoint(Point_3d_t *point, double x, double y, double z);
    void RotatePoint(Point_3d_t *point, Point_3d_t *point_new, double x, double y, double z);

  private:
    double r_rand    = PI / 180;
    double r_alpha   = 19.47 * PI / 180;
    double r_gamma   = 20.7 * PI / 180;
    double sin_alpha = sin(19.47 * PI / 180);
    double cos_alpha = cos(19.47 * PI / 180);
    double sin_gamma = sin(20.7 * PI / 180);
    double cos_gamma = cos(20.7 * PI / 180);
    int _zeroXoffset = 0;
    int _zeroYOffset = 0;
};
