
#ifndef ENUMS_H
#define ENUMS_H

enum class TextureType { Diffuse = 1, Specular, Normal };

enum class EffectType {
  NoEffect = 0,
  Inversion,
  Grayscale,
  Sharpen,
  Blur,
  Edge
};
#endif
