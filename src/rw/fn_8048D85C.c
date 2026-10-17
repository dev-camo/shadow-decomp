/* RenderWare palette quantization: colors are added to a tree of cells by the high bits of their
 * components, keeping weighted sums of the rest; the cells are then split into boxes of least
 * squared error, one per palette color, and images are matched to the palette through the tree.
 * The original file name is unknown. */
#include "rw/rwcore.h"

/* The memory hint ID of the quantizer's tree nodes (inferred) */
#define rwID_PALQUANTNODE 0x411

/* The weights of red and blue in color distances. The blue weight is written with eight digits:
 * the compiler rounds the seven-digit 0.1942078f to a float two units lower than the original. */
#define PALQUANTREDWEIGHT 0.5093697f
#define PALQUANTBLUEWEIGHT 0.19420783f

/* The number of bit levels of the tree */
RwInt32 lbl_805EDF20 = 6;

/* Each component value's bits, spread out to every fourth bit so that four components interleave
 * into a tree index */
RwUInt32 lbl_805E2D58[256];

/* The tree index of a color: its components' high bits interleaved, red highest (inferred
 * helper) */
static inline RwUInt32 PalQuantIndex(RwRGBA *color, RwInt32 shift) {
    return (lbl_805E2D58[color->red >> shift] << 3) | (lbl_805E2D58[color->green >> shift] << 2) |
           (lbl_805E2D58[color->blue >> shift] << 1) | lbl_805E2D58[color->alpha >> shift];
}

/* Turns weighted red and blue back into plain components */
void fn_8048D85C(RwRGBAReal *color) {
    color->red /= PALQUANTREDWEIGHT;
    color->blue /= PALQUANTBLUEWEIGHT;
}

void fn_8048D880(rwPalQuantLeafNode *leaf) {
    leaf->palIndex = 0;
    leaf->weight = 0.0f;
    leaf->sums.red = 0.0f;
    leaf->sums.green = 0.0f;
    leaf->sums.blue = 0.0f;
    leaf->sums.alpha = 0.0f;
    leaf->error = 0.0f;
}

/* Turns a leaf's sums of the colors' low bits into sums of whole colors: the error loses its mean
 * term, and the cell's corner color is added for every weighted color */
void fn_8048D8A8(rwPalQuantLeafNode *leaf, RwRGBA *corner) {
    RwRGBAReal color;

    leaf->error -= (leaf->sums.red * leaf->sums.red + leaf->sums.green * leaf->sums.green +
                    leaf->sums.blue * leaf->sums.blue + leaf->sums.alpha * leaf->sums.alpha) /
                   leaf->weight;
    if (leaf->error < 0.0f) {
        leaf->error = 0.0f;
    }
    color.red = (1.0f / 255.0f) * (RwReal)corner->red;
    color.green = (1.0f / 255.0f) * (RwReal)corner->green;
    color.blue = (1.0f / 255.0f) * (RwReal)corner->blue;
    color.alpha = (1.0f / 255.0f) * (RwReal)corner->alpha;
    color.red *= PALQUANTREDWEIGHT;
    color.blue *= PALQUANTBLUEWEIGHT;
    RwRGBARealScale(&color, &color, leaf->weight);
    leaf->sums.red += color.red;
    leaf->sums.green += color.green;
    leaf->sums.blue += color.blue;
    leaf->sums.alpha += color.alpha;
}

/* Merges two leaves' statistics; the error gains the cost of merging their means */
void fn_8048D9C8(rwPalQuantLeafNode *result, rwPalQuantLeafNode *leaf1, rwPalQuantLeafNode *leaf2) {
    result->error = leaf1->error + leaf2->error;
    if (leaf1->weight > 0.0f && leaf2->weight > 0.0f) {
        RwReal recip1 = 1.0f / leaf1->weight;
        RwReal recip2 = 1.0f / leaf2->weight;
        RwRGBAReal mean1;
        RwRGBAReal mean2;

        RwRGBARealScale(&mean1, &leaf1->sums, recip1);
        RwRGBARealScale(&mean2, &leaf2->sums, recip2);
        mean1.red -= mean2.red;
        mean1.green -= mean2.green;
        mean1.blue -= mean2.blue;
        mean1.alpha -= mean2.alpha;
        result->error += (mean1.red * mean1.red + mean1.green * mean1.green +
                          mean1.blue * mean1.blue + mean1.alpha * mean1.alpha) /
                         (recip1 + recip2);
    }
    RwRGBARealAdd(&result->sums, &leaf1->sums, &leaf2->sums);
    result->weight = leaf1->weight + leaf2->weight;
}

/* A leaf's mean color, as bytes */
void fn_8048DAC8(RwRGBA *color, rwPalQuantLeafNode *leaf) {
    RwRGBAReal mean;
    RwInt32 red;
    RwInt32 green;
    RwInt32 blue;
    RwInt32 alpha;

    RwRGBARealScale(&mean, &leaf->sums, 1.0f / leaf->weight);
    fn_8048D85C(&mean);

    red = (RwInt32)(255.99f * mean.red);
    green = (RwInt32)(255.99f * mean.green);
    blue = (RwInt32)(255.99f * mean.blue);
    alpha = (RwInt32)(255.99f * mean.alpha);
    color->red = 255;
    if (red < 255) {
        color->red = red;
    }
    color->green = 255;
    if (green < 255) {
        color->green = green;
    }
    color->blue = 255;
    if (blue < 255) {
        color->blue = blue;
    }
    color->alpha = 255;
    if (alpha < 255) {
        color->alpha = alpha;
    }
}

void fn_8048DBA4(rwPalQuantOctNode **children) {
    RwInt32 i;

    for (i = 0; i < 16; i++) {
        children[i] = NULL;
    }
}

/* The leaf of a tree index, creating the nodes on the way */
rwPalQuantOctNode *fn_8048DBEC(RwPalQuant *palQuant, rwPalQuantOctNode *node, RwUInt32 index,
                               RwInt32 level) {
    if (level == 0) {
        return node;
    }
    if (node->children[index & 0xF] == NULL) {
        rwPalQuantOctNode *child = (rwPalQuantOctNode *)RwFreeListAlloc(
            palQuant->nodeFreeList, rwMEMHINTDUR_EVENT | rwID_PALQUANTNODE);

        node->children[index & 0xF] = child;
        fn_8048DBA4(child->children);
        if (level == 1) {
            fn_8048D880(&child->leaf);
        }
    }
    return fn_8048DBEC(palQuant, node->children[index & 0xF], index >> 4, level - 1);
}

/* Adds a weighted color's low bits to its leaf (inferred helper) */
static inline void PalQuantAddColor(rwPalQuantLeafNode *leaf, RwRGBA *color, RwReal weight) {
    RwUInt8 mask;
    RwRGBAReal low;

    mask = (RwUInt8)((1 << (8 - lbl_805EDF20)) - 1);
    low.red = (1.0f / 255.0f) * (RwReal)(RwUInt8)(color->red & mask);
    low.green = (1.0f / 255.0f) * (RwReal)(RwUInt8)(color->green & mask);
    low.blue = (1.0f / 255.0f) * (RwReal)(RwUInt8)(color->blue & mask);
    low.alpha = (1.0f / 255.0f) * (RwReal)(RwUInt8)(color->alpha & mask);
    low.red *= PALQUANTREDWEIGHT;
    low.blue *= PALQUANTBLUEWEIGHT;
    leaf->weight += weight;
    leaf->error += weight * (low.red * low.red + low.green * low.green + low.blue * low.blue +
                             low.alpha * low.alpha);
    RwRGBARealScale(&low, &low, weight);
    RwRGBARealAdd(&leaf->sums, &leaf->sums, &low);
}

/* Adds an image's colors to the quantizer, each with the weight */
void fn_8048DEB0(RwPalQuant *palQuant, RwImage *image, RwReal weight) {
    RwInt32 columns;
    RwInt32 stride;
    RwUInt8 *row;
    RwInt32 rows;
    RwRGBA *palette;
    RwUInt8 *pixel;
    RwRGBA *color;
    rwPalQuantOctNode *leaf;
    RwInt32 level;

    stride = image->stride;
    row = image->cpPixels;
    palette = image->palette;
    rows = image->height;
    switch (image->depth) {
    case 4:
    case 8:
        while (rows-- != 0) {
            columns = image->width;
            pixel = row;
            while (columns-- != 0) {
                color = &palette[*pixel];
                level = lbl_805EDF20;
                leaf =
                    fn_8048DBEC(palQuant, palQuant->root, PalQuantIndex(color, 8 - level), level);
                PalQuantAddColor(&leaf->leaf, color, weight);
                pixel++;
            }
            row += stride;
        }
        break;
    case 32:
        while (rows-- != 0) {
            columns = image->width;
            color = (RwRGBA *)row;
            while (columns-- != 0) {
                level = lbl_805EDF20;
                leaf =
                    fn_8048DBEC(palQuant, palQuant->root, PalQuantIndex(color, 8 - level), level);
                PalQuantAddColor(&leaf->leaf, color, weight);
                color++;
            }
            row += stride;
        }
        break;
    }
}

/* Gives the leaves of a cell (corner, 1 << level wide) that are in a box a palette index */
void fn_8048E300(rwPalQuantOctNode *node, RwInt32 *corner, RwInt32 level, rwPalQuantBox *box,
                 RwInt32 index) {
    if (node != NULL) {
        RwInt32 d0 = (1 << level) + corner[0] - box->lower[0];
        RwInt32 d1 = (1 << level) + corner[1] - box->lower[1];
        RwInt32 d2 = (1 << level) + corner[2] - box->lower[2];
        RwInt32 d3 = (1 << level) + corner[3] - box->lower[3];

        /* Nothing to do unless the cell overlaps the box */
        if (d0 <= 0 || d1 <= 0 || d2 <= 0 || d3 <= 0) {
            return;
        }
        d0 = corner[0] - box->upper[0];
        d1 = corner[1] - box->upper[1];
        d2 = corner[2] - box->upper[2];
        d3 = corner[3] - box->upper[3];
        if (d0 >= 0 || d1 >= 0 || d2 >= 0 || d3 >= 0) {
            return;
        }

        if (level == 0) {
            node->leaf.palIndex = index;
        } else {
            RwInt32 i;

            for (i = 0; i < 16; i++) {
                RwInt32 subCorner[4];

                subCorner[0] = corner[0] + (((i >> 3) & 1) << (level - 1));
                subCorner[1] = corner[1] + (((i >> 2) & 1) << (level - 1));
                subCorner[2] = corner[2] + (((i >> 1) & 1) << (level - 1));
                subCorner[3] = corner[3] + ((i & 1) << (level - 1));
                fn_8048E300(node->children[i], subCorner, level - 1, box, index);
            }
        }
    }
}

/* Adds the statistics of the leaves of a cell (corner, 1 << level wide) that are in a box */
void fn_8048E47C(rwPalQuantOctNode *node, RwInt32 *corner, RwInt32 level, rwPalQuantBox *box,
                 rwPalQuantLeafNode *result) {
    if (node != NULL) {
        RwInt32 d0 = (1 << level) + corner[0] - box->lower[0];
        RwInt32 d1 = (1 << level) + corner[1] - box->lower[1];
        RwInt32 d2 = (1 << level) + corner[2] - box->lower[2];
        RwInt32 d3 = (1 << level) + corner[3] - box->lower[3];

        /* Nothing to add unless the cell overlaps the box */
        if (d0 <= 0 || d1 <= 0 || d2 <= 0 || d3 <= 0) {
            return;
        }
        d0 = corner[0] - box->upper[0];
        d1 = corner[1] - box->upper[1];
        d2 = corner[2] - box->upper[2];
        d3 = corner[3] - box->upper[3];
        if (d0 >= 0 || d1 >= 0 || d2 >= 0 || d3 >= 0) {
            return;
        }

        d0 = corner[0] - box->lower[0];
        d1 = corner[1] - box->lower[1];
        d2 = corner[2] - box->lower[2];
        d3 = corner[3] - box->lower[3];
        if (d0 >= 0 && d1 >= 0 && d2 >= 0 && d3 >= 0) {
            d0 = box->upper[0] - ((1 << level) + corner[0]);
            d1 = box->upper[1] - ((1 << level) + corner[1]);
            d2 = box->upper[2] - ((1 << level) + corner[2]);
            d3 = box->upper[3] - ((1 << level) + corner[3]);
            if (d0 >= 0 && d1 >= 0 && d2 >= 0 && d3 >= 0) {
                /* The cell is inside the box */
                fn_8048D9C8(result, result, &node->leaf);
                return;
            }
        }
        if (level > 0) {
            RwInt32 i;

            for (i = 0; i < 16; i++) {
                RwInt32 subCorner[4];

                subCorner[0] = corner[0] + (((i >> 3) & 1) << (level - 1));
                subCorner[1] = corner[1] + (((i >> 2) & 1) << (level - 1));
                subCorner[2] = corner[2] + (((i >> 1) & 1) << (level - 1));
                subCorner[3] = corner[3] + ((i & 1) << (level - 1));
                fn_8048E47C(node->children[i], subCorner, level - 1, box, result);
            }
        }
    }
}

/* The statistics of a set (whole) without a subset (part) (inferred helper) */
static inline void PalQuantSubtract(rwPalQuantLeafNode *result, rwPalQuantLeafNode *whole,
                                    rwPalQuantLeafNode *part) {
    result->weight = whole->weight - part->weight;
    result->sums.red = whole->sums.red - part->sums.red;
    result->sums.green = whole->sums.green - part->sums.green;
    result->sums.blue = whole->sums.blue - part->sums.blue;
    result->sums.alpha = whole->sums.alpha - part->sums.alpha;
    result->error = whole->error - part->error;
    if (result->weight > 0.0f && part->weight > 0.0f) {
        RwReal recip1 = 1.0f / part->weight;
        RwReal recip2 = 1.0f / result->weight;
        RwRGBAReal mean1;
        RwRGBAReal mean2;

        RwRGBARealScale(&mean1, &part->sums, recip1);
        RwRGBARealScale(&mean2, &result->sums, recip2);
        mean1.red -= mean2.red;
        mean1.green -= mean2.green;
        mean1.blue -= mean2.blue;
        mean1.alpha -= mean2.alpha;
        result->error -= (mean1.red * mean1.red + mean1.green * mean1.green +
                          mean1.blue * mean1.blue + mean1.alpha * mean1.alpha) /
                         (recip1 + recip2);
    }
}

/* Searches one axis of a box for the cut that leaves the least error in its two halves; returns
 * that error, and the cut through cut[axis], when it is less than the box's own */
RwReal fn_8048E748(rwPalQuantOctNode *root, rwPalQuantBox *box, RwInt32 axis, RwInt32 *cut,
                   rwPalQuantLeafNode *whole) {
    rwPalQuantBox half;
    rwPalQuantLeafNode inner;
    RwInt32 corner[4];
    RwReal best = whole->error;
    RwInt32 middle;
    RwInt32 lower;
    RwInt32 upper;

    half = *box;
    lower = box->lower[axis];
    upper = box->upper[axis];
    while (upper - lower > 1) {
        rwPalQuantLeafNode outer;

        middle = lower + ((upper - lower) >> 1);

        /* The statistics of the lower half... */
        half.upper[axis] = middle;
        corner[0] = 0;
        corner[1] = 0;
        corner[2] = 0;
        corner[3] = 0;
        fn_8048D880(&inner);
        fn_8048E47C(root, corner, lbl_805EDF20, &half, &inner);

        /* ...and of the upper half, the rest of the box */
        PalQuantSubtract(&outer, whole, &inner);

        if (inner.weight == 0.0f) {
            lower = middle;
        } else if (outer.weight == 0.0f) {
            upper = middle;
        } else {
            RwReal error = inner.error + outer.error;

            if (error < best) {
                best = error;
                cut[axis] = half.upper[axis];
            }
            if (inner.error < outer.error) {
                lower = middle;
            } else {
                upper = middle;
            }
        }
    }
    return best;
}

/* Cuts a box along the axis that leaves the least error, into it and a new box, when that error is
 * less than the box's own */
RwBool fn_8048E988(rwPalQuantOctNode *root, rwPalQuantLeafNode *whole, rwPalQuantBox *box,
                   rwPalQuantBox *newBox) {
    RwReal errors[4];
    RwInt32 cut[4];
    RwInt32 axis;
    RwInt32 i;

    errors[0] = fn_8048E748(root, box, 0, cut, whole);
    errors[1] = fn_8048E748(root, box, 1, cut, whole);
    errors[2] = fn_8048E748(root, box, 2, cut, whole);
    errors[3] = fn_8048E748(root, box, 3, cut, whole);

    axis = 0;
    for (i = 0; i < 4; i++) {
        if (errors[i] < errors[axis]) {
            axis = i;
        }
    }

    if (errors[axis] < whole->error) {
        RwUInt8 cutValue;

        *newBox = *box;
        cutValue = cut[axis];
        newBox->lower[axis] = cutValue;
        box->upper[axis] = cutValue;
        return TRUE;
    }
    return FALSE;
}

/* Turns the statistics of a cell (corner color, level) and its descendants into sums of whole
 * colors, merging them into each node */
rwPalQuantOctNode *fn_8048EB24(rwPalQuantOctNode *node, RwRGBA *corner, RwInt32 level) {
    RwInt32 childLevel = level - 1;
    rwPalQuantOctNode *result = NULL;

    if (node != NULL) {
        result = node;
        if (level > 0) {
            RwInt32 i;

            fn_8048D880(&node->leaf);
            for (i = 0; i < 16; i++) {
                RwInt32 shift = childLevel + (8 - lbl_805EDF20);
                RwRGBA subCorner;
                rwPalQuantOctNode *child;

                subCorner.red = corner->red + (((i >> 3) & 1) << shift);
                subCorner.green = corner->green + (((i >> 2) & 1) << shift);
                subCorner.blue = corner->blue + (((i >> 1) & 1) << shift);
                subCorner.alpha = corner->alpha + ((i & 1) << shift);
                child = fn_8048EB24(node->children[i], &subCorner, level - 1);
                if (child != NULL) {
                    fn_8048D9C8(&node->leaf, &node->leaf, &child->leaf);
                }
            }
        } else {
            fn_8048D8A8(&node->leaf, corner);
        }
    }
    return result;
}

RwInt32 fn_8048EEFC(rwPalQuantOctNode *node, RwInt32 level) {
    RwInt32 i;
    RwInt32 count = 0;

    if (node != NULL) {
        if (level > 0) {
            for (i = 0; i < 16; i++) {
                count += fn_8048EEFC(node->children[i], level - 1);
            }
        } else {
            count = 1;
        }
    }
    return count;
}

/* Gives each leaf the next palette index and its mean color; returns the next index */
RwInt32 fn_8048F1E4(rwPalQuantOctNode *node, RwRGBA *palette, RwInt32 index, RwInt32 level) {
    RwInt32 i;

    if (node != NULL) {
        if (level > 0) {
            for (i = 0; i < 16; i++) {
                index = fn_8048F1E4(node->children[i], palette, index, level - 1);
            }
        } else {
            fn_8048DAC8(&palette[index], &node->leaf);
            node->leaf.palIndex = index;
            index++;
        }
    }
    return index;
}

RwUInt8 fn_8048F4B4(rwPalQuantOctNode *node, RwUInt32 index, RwInt32 level) {
    if (level == 0) {
        return node->leaf.palIndex;
    }
    return fn_8048F4B4(node->children[index & 0xF], index >> 4, level - 1);
}

/* The statistics of the colors in a box (inferred helper) */
static inline void PalQuantBoxStats(rwPalQuantOctNode *root, rwPalQuantBox *box,
                                    rwPalQuantLeafNode *stats) {
    RwInt32 origin[4];

    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;
    origin[3] = 0;
    fn_8048D880(stats);
    fn_8048E47C(root, origin, lbl_805EDF20, box, stats);
}

/* Gives the leaves in a box a palette index (inferred helper) */
static inline void PalQuantSetBoxIndex(rwPalQuantOctNode *root, rwPalQuantBox *box, RwInt32 index) {
    RwInt32 origin[4];

    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;
    origin[3] = 0;
    fn_8048E300(root, origin, lbl_805EDF20, box, index);
}

/* Makes a palette of at most maxColors colors from the colors added; returns how many it has */
RwInt32 fn_8048F5D0(RwRGBA *palette, RwInt32 maxColors, RwPalQuant *palQuant) {
    RwRGBA corner = {0, 0, 0, 0};
    RwInt32 numLeaves;
    RwInt32 i;

    fn_8048EB24(palQuant->root, &corner, lbl_805EDF20);
    for (i = 0; i < maxColors; i++) {
        palette[i].red = 0;
        palette[i].green = 0;
        palette[i].blue = 0;
        palette[i].alpha = 0;
    }

    numLeaves = fn_8048EEFC(palQuant->root, lbl_805EDF20);
    if (numLeaves <= maxColors) {
        /* Every leaf has its own color */
        maxColors = numLeaves;
        fn_8048F1E4(palQuant->root, palette, 0, lbl_805EDF20);
    } else {
        RwInt32 numBoxes;

        /* One box holds every color... */
        palQuant->boxes[0].lower[0] = 0;
        palQuant->boxes[0].lower[1] = 0;
        palQuant->boxes[0].lower[2] = 0;
        palQuant->boxes[0].lower[3] = 0;
        palQuant->boxes[0].upper[0] = 1 << lbl_805EDF20;
        palQuant->boxes[0].upper[1] = 1 << lbl_805EDF20;
        palQuant->boxes[0].upper[2] = 1 << lbl_805EDF20;
        palQuant->boxes[0].upper[3] = 1 << lbl_805EDF20;
        PalQuantBoxStats(palQuant->root, &palQuant->boxes[0], &palQuant->boxStats[0]);
        palQuant->boxErrors[0] = palQuant->boxStats[0].error;

        /* ...and the box of most error is cut until there are enough */
        for (numBoxes = 1; numBoxes < maxColors; numBoxes++) {
            RwReal maxError = 0.0f;
            RwInt32 next = -1;

            for (i = 0; i < numBoxes; i++) {
                if (palQuant->boxErrors[i] > maxError) {
                    maxError = palQuant->boxErrors[i];
                    next = i;
                }
            }
            if (next == -1) {
                break;
            }

            if (fn_8048E988(palQuant->root, &palQuant->boxStats[next], &palQuant->boxes[next],
                            &palQuant->boxes[numBoxes])) {
                PalQuantBoxStats(palQuant->root, &palQuant->boxes[next], &palQuant->boxStats[next]);
                PalQuantBoxStats(palQuant->root, &palQuant->boxes[numBoxes],
                                 &palQuant->boxStats[numBoxes]);
                palQuant->boxErrors[next] = palQuant->boxStats[next].error;
                palQuant->boxErrors[numBoxes] = palQuant->boxStats[numBoxes].error;
            } else {
                /* The box cannot be cut usefully */
                palQuant->boxErrors[next] = 0.0f;
                numBoxes--;
            }
        }

        /* Each box's leaves take its index, and the palette its mean color */
        for (i = 0; i < maxColors; i++) {
            PalQuantSetBoxIndex(palQuant->root, &palQuant->boxes[i], i);
            fn_8048DAC8(&palette[i], &palQuant->boxStats[i]);
        }
    }
    return maxColors;
}

/* Writes an image's pixels as the palette indices of their colors: two to a byte (the first in the
 * low nibble) for packed 4-bit pixels */
void fn_8048FAB0(RwUInt8 *pixels, RwInt32 stride, RwInt32 depth, RwBool packed,
                 RwPalQuant *palQuant, RwImage *image) {
    RwRGBA *palette;
    RwInt32 sourceStride;
    RwRGBA *color;
    RwUInt8 *sourceRow;
    RwUInt32 width;
    RwUInt32 x;
    RwUInt32 rows;
    RwUInt8 index;
    RwUInt8 *destination;
    RwUInt8 *source;

    sourceStride = image->stride;
    sourceRow = image->cpPixels;
    if (depth == 4 && packed == TRUE) {
        switch (image->depth) {
        case 4:
        case 8:
            palette = image->palette;
            rows = image->height;
            while (rows-- != 0) {
                width = image->width;
                source = sourceRow;
                destination = pixels;
                for (x = 0; x < width; x++) {
                    color = &palette[*source++];
                    index = fn_8048F4B4(palQuant->root, PalQuantIndex(color, 8 - lbl_805EDF20),
                                        lbl_805EDF20);
                    if (x & 1) {
                        *destination &= 0x0F;
                        *destination |= (index & 0x0F) << 4;
                        destination++;
                    } else {
                        *destination &= 0xF0;
                        *destination |= index & 0x0F;
                    }
                }
                sourceRow += sourceStride;
                pixels += stride;
            }
            break;
        case 32:
            rows = image->height;
            while (rows-- != 0) {
                width = image->width;
                color = (RwRGBA *)sourceRow;
                destination = pixels;
                for (x = 0; x < width; x++) {
                    index = fn_8048F4B4(palQuant->root, PalQuantIndex(color++, 8 - lbl_805EDF20),
                                        lbl_805EDF20);
                    if (x & 1) {
                        *destination &= 0x0F;
                        *destination |= (index & 0x0F) << 4;
                        destination++;
                    } else {
                        *destination &= 0xF0;
                        *destination |= index & 0x0F;
                    }
                }
                sourceRow += sourceStride;
                pixels += stride;
            }
            break;
        }
    } else {
        switch (image->depth) {
        case 4:
        case 8:
            palette = image->palette;
            rows = image->height;
            while (rows-- != 0) {
                width = image->width;
                source = sourceRow;
                destination = pixels;
                while (width-- != 0) {
                    color = &palette[*source++];
                    *destination++ = fn_8048F4B4(
                        palQuant->root, PalQuantIndex(color, 8 - lbl_805EDF20), lbl_805EDF20);
                }
                sourceRow += sourceStride;
                pixels += stride;
            }
            break;
        case 32:
            rows = image->height;
            while (rows-- != 0) {
                width = image->width;
                color = (RwRGBA *)sourceRow;
                destination = pixels;
                while (width-- != 0) {
                    *destination++ = fn_8048F4B4(
                        palQuant->root, PalQuantIndex(color++, 8 - lbl_805EDF20), lbl_805EDF20);
                }
                sourceRow += sourceStride;
                pixels += stride;
            }
            break;
        }
    }
}

/* Initializes a quantizer: the bit spreading table, the free list of tree nodes and the root */
RwBool fn_8048FED8(RwPalQuant *palQuant) {
    RwInt32 levels;
    RwUInt32 value;
    RwUInt32 bit;
    RwUInt32 numValues;
    RwUInt32 bits;

    levels = lbl_805EDF20;
    numValues = 1 << levels;
    for (value = 0; value < numValues; value++) {
        bits = 0;
        for (bit = 0; bit < levels; bit++) {
            bits |= (value & (1 << bit)) ? (1 << ((levels - 1 - bit) * 4)) : 0;
        }
        lbl_805E2D58[value] = bits;
    }

    palQuant->nodeFreeList =
        fn_8047FBD8(sizeof(rwPalQuantOctNode), 1024, 4, rwMEMHINTDUR_EVENT | rwID_PALQUANTNODE);
    palQuant->root = (rwPalQuantOctNode *)RwFreeListAlloc(palQuant->nodeFreeList,
                                                          rwMEMHINTDUR_EVENT | rwID_PALQUANTNODE);
    fn_8048DBA4(palQuant->root->children);
    return TRUE;
}

/* Frees a node and its descendants */
void fn_804900D4(RwPalQuant *palQuant, rwPalQuantOctNode *node, RwInt32 level) {
    if (node != NULL) {
        if (level > 0) {
            RwInt32 i;

            for (i = 0; i < 16; i++) {
                fn_804900D4(palQuant, node->children[i], level - 1);
            }
        }
        RwFreeListFree(palQuant->nodeFreeList, node);
    }
}

void fn_804903F8(RwPalQuant *palQuant) {
    fn_804900D4(palQuant, palQuant->root, lbl_805EDF20);
    palQuant->root = NULL;
    fn_8047FCDC(palQuant->nodeFreeList);
}
