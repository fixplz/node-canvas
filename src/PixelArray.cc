
//
// PixelArray.cc
//
// Copyright (c) 2010 LearnBoost <tj@learnboost.com>
//

#include "PixelArray.h"
#include <stdlib.h>
#include <string.h>

Persistent<FunctionTemplate> PixelArray::constructor;

/*
 * Initialize PixelArray.
 */

void
PixelArray::Initialize(Handle<Object> target) {
  NanScope();

  // Constructor
  Local<FunctionTemplate> ctor = NanNew<FunctionTemplate>(PixelArray::New);
  NanAssignPersistent(constructor, ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(NanNew("CanvasPixelArray"));

  // Prototype
  Local<ObjectTemplate> proto = ctor->InstanceTemplate();
  proto->SetAccessor(NanNew("length"), GetLength);
  target->Set(NanNew("CanvasPixelArray"), ctor->GetFunction());
}

/*
 * Initialize a new PixelArray.
 */

NAN_METHOD(PixelArray::New) {
  NanScope();
  PixelArray *arr;
  Local<Object> obj = args[0]->ToObject();

  switch (args.Length()) {
    // width, height
    case 2:
      arr = new PixelArray(
          args[0]->Int32Value()
        , args[1]->Int32Value());
      break;
    // canvas, x, y, width, height
    case 5: {
      if (NanHasInstance(Canvas::constructor, obj)) {
        Canvas *canvas = ObjectWrap::Unwrap<Canvas>(obj);
        arr = new PixelArray(
            canvas
          , args[1]->Int32Value()
          , args[2]->Int32Value()
          , args[3]->Int32Value()
          , args[4]->Int32Value());
      }
      else if(NanHasInstance(Image::constructor, obj)) {
        Image *image = ObjectWrap::Unwrap<Image>(obj);
        arr = new PixelArray(
            image
          , args[1]->Int32Value()
          , args[2]->Int32Value()
          , args[3]->Int32Value()
          , args[4]->Int32Value());
      }
      else
        return NanThrowTypeError("Canvas or Image expected");

      }

      break;
    default:
      return NanThrowTypeError("invalid arguments");
  }

  // Let v8 handle accessors (and clamping)
  args.This()->SetIndexedPropertiesToPixelData(
      arr->data()
    , arr->length());

  arr->Wrap(args.This());
  NanReturnValue(args.This());
}

/*
 * Get length.
 */

NAN_GETTER(PixelArray::GetLength) {
  NanScope();
  NanReturnValue(NanNew<Number>(args.This()->GetIndexedPropertiesPixelDataLength()));
}

/*
 * Initialize a new PixelArray copying data
 * from the surface using the given rect.
 */

PixelArray::PixelArray(Canvas *canvas, int sx, int sy, int width, int height):
  _width(width), _height(height) {

  getPixels(sx, sy, width, height, true, canvas->data(), canvas->stride(), canvas->width, canvas->height);
}

PixelArray::PixelArray(Image *image, int sx, int sy, int width, int height):
  _width(width), _height(height) {

  getPixels(sx, sy, width, height, false, image->data(), image->stride(), image->width, image->height);
}

void
PixelArray::getPixels(
  int sx, int sy, int width, int height, bool unmultiply,
  uint8_t *src, int srcStride,int srcWidth, int srcHeight) {
  // Alloc space for our new data
  uint8_t *dst = alloc();

  int dstStride = stride();

  if (sx < 0) width += sx, sx = 0;
  if (sy < 0) height += sy, sy = 0;
  if (sx + width > srcWidth) width = srcWidth - sx;
  if (sy + height > srcHeight) height = srcHeight - sy;
  if (width <= 0 || height <= 0) return;

  // Normalize data (argb -> rgba)
  for (int y = 0; y < height; ++y) {
    uint32_t *row = (uint32_t *)(src + srcStride * (y + sy));
    for (int x = 0; x < width; ++x) {
      int bx = x * 4;
      uint32_t *pixel = row + x + sx;
      uint8_t a = *pixel >> 24;
      uint8_t r = *pixel >> 16;
      uint8_t g = *pixel >> 8;
      uint8_t b = *pixel;
      dst[bx + 3] = a;
      if(unmultiply) {
        float alpha = (float) a / 255;
        dst[bx + 0] = (int)((float) r / alpha);
        dst[bx + 1] = (int)((float) g / alpha);
        dst[bx + 2] = (int)((float) b / alpha);
      }
      else {
        dst[bx + 0] = r;
        dst[bx + 1] = g;
        dst[bx + 2] = b;
      }
    }
    dst += dstStride;
  }
}

/*
 * Initialize an empty PixelArray with the given dimensions.
 */

PixelArray::PixelArray(int width, int height):
  _width(width), _height(height) {
  alloc();
}

/*
 * Allocate / zero data buffer. Hint mem adjustment.
 */

uint8_t *
PixelArray::alloc() {
  int len = length();
  _data = (uint8_t *) calloc(1, len);
  NanAdjustExternalMemory(len);
  return _data;
}

/*
 * Hint mem adjustment.
 */

PixelArray::~PixelArray() {
  NanAdjustExternalMemory(-length());
  free(_data);
}
