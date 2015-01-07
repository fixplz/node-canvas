
/*!
 * Canvas - Image
 * Copyright (c) 2010 LearnBoost <tj@learnboost.com>
 * MIT Licensed
 */

/**
 * Module dependencies.
 */

var Canvas = require('./bindings')
  , Image = Canvas.Image
  , ImageData = Canvas.ImageData
  , PixelArray = Canvas.CanvasPixelArray;

/**
 * Src setter.
 *
 *  - convert data uri to `Buffer`
 *
 * @param {String|Buffer} val filename, buffer, data uri
 * @api public
 */

Image.prototype.__defineSetter__('src', function(val){
  if ('string' == typeof val && 0 == val.indexOf('data:')) {
    val = val.slice(val.indexOf(',') + 1);
    this.source = new Buffer(val, 'base64');
  } else {
    this.source = val;
  }
});

/**
 * Src getter.
 * 
 * TODO: return buffer
 * 
 * @api public
 */

Image.prototype.__defineGetter__('src', function(){
  return this.source;
});

/**
 * Inspect image.
 *
 * TODO: indicate that the .src was a buffer, data uri etc
 *
 * @return {String}
 * @api public
 */

Image.prototype.inspect = function(){
  return '[Image'
    + (this.complete ? ':' + this.width + 'x' + this.height : '')
    + (this.src ? ' ' + this.src : '')
    + (this.complete ? ' complete' : '')
    + ']';
};

/**
 * Get `ImageData` with the given rect.
 *
 * @param {Number} x
 * @param {Number} y
 * @param {Number} width
 * @param {Number} height
 * @return {ImageData}
 * @api public
 */

Image.prototype.getImageData = function(x, y, width, height){
  var arr = new PixelArray(this, x, y, width, height);
  return new ImageData(arr);
};
