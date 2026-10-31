/* RenderWare image resampling: each pixel of the resampled image is the average of the source
 * rectangle it covers, weighing the partly covered pixels at its edges. Positions are 16.16 fixed
 * point. The original file name is unknown. */
#include "rw/rwcore.h"

/* The average color of row y (16.16) from x0 to x1 (16.16), weighing the edge pixels by how much
 * of them the span covers */
void fn_8048A178(const RwImage *image, RwFixed x0, RwFixed x1, RwFixed y, RwRGBAReal *result) {
    RwReal width = (RwReal)(x1 - x0) * (1.0f / 65536.0f);
    RwInt32 x = x0 >> 16; /* the first pixel, then each pixel covered completely */
    RwInt32 endX = x1 >> 16;
    const RwRGBA *pixel =
        (const RwRGBA *)(image->cpPixels + (image->stride * (y >> 16) + x * sizeof(RwRGBA)));

    if (x == endX) {
        RwRGBARealFromRwRGBA(result, pixel);
        RwRGBARealScale(result, result, width);
    } else {
        RwFixed nextX = (x + 1) << 16;
        RwRGBAReal color;
        RwReal weight;

        /* The first pixel, covered from x0 */
        RwRGBARealFromRwRGBA(result, pixel);
        weight = (RwReal)(nextX - x0) * (1.0f / 65536.0f);
        RwRGBARealScale(result, result, weight);
        pixel++;

        /* The pixels covered completely */
        for (x = nextX >> 16; x != endX; x++) {
            RwRGBARealFromRwRGBA(&color, pixel);
            RwRGBARealAdd(result, result, &color);
            nextX += 1 << 16;
            pixel++;
        }

        /* The last pixel, covered up to x1 */
        RwRGBARealFromRwRGBA(&color, pixel);
        weight = (RwReal)(x1 - nextX) * (1.0f / 65536.0f);
        RwRGBARealScale(&color, &color, weight);
        RwRGBARealAdd(result, result, &color);
    }
    {
        /* The sum covers the span's width of pixels */
        RwReal recipWidth = 1.0f / width;

        RwRGBARealScale(result, result, recipWidth);
    }
}

/* The average color of the rectangle from x0 to x1 and y0 to y1 (16.16), weighing the edge rows by
 * how much of them it covers */
void fn_8048A50C(const RwImage *image, RwFixed x0, RwFixed x1, RwFixed y0, RwFixed y1,
                 RwRGBAReal *result) {
    RwReal height = (RwReal)(y1 - y0) * (1.0f / 65536.0f);
    RwInt32 row = y0 >> 16; /* the first row, then each row covered completely */
    RwInt32 endY = y1 >> 16;

    if (row == endY) {
        fn_8048A178(image, x0, x1, y0, result);
        RwRGBARealScale(result, result, height);
    } else {
        RwRGBAReal rowColor;
        RwReal weight;
        RwFixed nextY = (row + 1) << 16;

        /* The first row, covered from y0 */
        fn_8048A178(image, x0, x1, y0, result);
        weight = (RwReal)(nextY - y0) * (1.0f / 65536.0f);
        RwRGBARealScale(result, result, weight);

        /* The rows covered completely */
        for (row = nextY >> 16; row != endY; row++) {
            fn_8048A178(image, x0, x1, nextY, &rowColor);
            nextY += 1 << 16;
            RwRGBARealAdd(result, &rowColor, result);
        }

        /* The last row, covered up to y1 */
        fn_8048A178(image, x0, x1, nextY, &rowColor);
        weight = (RwReal)(y1 - nextY) * (1.0f / 65536.0f);
        RwRGBARealScale(&rowColor, &rowColor, weight);
        RwRGBARealAdd(result, result, &rowColor);
    }
    {
        /* The sum covers the rectangle's height of rows */
        RwReal recipHeight = 1.0f / height;

        RwRGBARealScale(result, result, recipHeight);
    }
}

/* Resamples a 32-bit image into destImage, each pixel the average of the source rectangle it
 * covers */
RwImage *fn_8048A790(RwImage *destImage, const RwImage *sourceImage) {
    RwInt32 column;
    RwInt32 row;
    RwFixed x;
    RwFixed xStep;
    RwFixed y;
    RwFixed yStep;
    RwInt32 destWidth;
    RwInt32 destHeight;
    RwInt32 sourceWidth;
    RwInt32 sourceHeight;

    destWidth = destImage->width;
    destHeight = destImage->height;
    sourceWidth = sourceImage->width;
    sourceHeight = sourceImage->height;
    destImage->flags |= sourceImage->flags & rwIMAGEGAMMACORRECTED;

    xStep = (RwFixed)(65536.0f * ((RwReal)sourceWidth / (RwReal)destWidth));
    yStep = (RwFixed)(65536.0f * ((RwReal)sourceHeight / (RwReal)destHeight));

    y = 0;
    for (row = 0; row < destHeight; row++) {
        RwRGBA *const destRow = (RwRGBA *)(destImage->cpPixels + destImage->stride * row);

        x = 0;
        for (column = 0; column < destWidth; column++) {
            RwRGBAReal color;

            fn_8048A50C(sourceImage, x, x + xStep - 1, y, y + yStep - 1, &color);
            RwRGBAFromRwRGBAReal(&destRow[column], &color);
            x += xStep;
        }
        y += yStep;
    }
    return destImage;
}

/* Creates a 32-bit resampled copy of an image (a 4 or 8-bit image goes through a 32-bit copy) */
RwImage *fn_8048A96C(const RwImage *sourceImage, RwInt32 width, RwInt32 height) {
    RwImage *resampled = fn_80487D08(width, height, 32);

    if (resampled == NULL) {
        return NULL;
    }
    if (fn_80487E18(resampled) == NULL) {
        fn_80487DB0(resampled);
        return NULL;
    }

    if (sourceImage->depth != 32) {
        /* A 4 or 8-bit image is resampled from a 32-bit copy */
        RwImage *source32 = fn_80487D08(sourceImage->width, sourceImage->height, 32);

        if (source32 == NULL) {
            fn_80487F50(resampled);
            fn_80487DB0(resampled);
            return NULL;
        }
        if (fn_80487E18(source32) == NULL) {
            fn_80487DB0(source32);
            fn_80487F50(resampled);
            fn_80487DB0(resampled);
            return NULL;
        }
        fn_804893B8(source32, sourceImage);
        if (fn_8048A790(resampled, source32) == NULL) {
            fn_80487F50(source32);
            fn_80487DB0(source32);
            fn_80487F50(resampled);
            fn_80487DB0(resampled);
            return NULL;
        }
        fn_80487F50(source32);
        fn_80487DB0(source32);
    } else if (fn_8048A790(resampled, sourceImage) == NULL) {
        fn_80487F50(resampled);
        fn_80487DB0(resampled);
        return NULL;
    }
    return resampled;
}
