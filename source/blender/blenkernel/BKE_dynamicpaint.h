/**
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Contributor(s): Miika Hämäläinen
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef BKE_DYNAMIC_PAINT_H_
#define BKE_DYNAMIC_PAINT_H_

struct bContext;
struct wmOperator;

/* Actual surface point	*/
typedef struct PaintSurfaceData {
	void *format_data; /* special data for each surface "format" */
	void *type_data; /* data used by specific surface type */
	struct PaintAdjData *adj_data; /* adjacency data for current surface */

	struct PaintBakeData *bData; /* temporary per step data used for frame calculation */
	int total_points;

} PaintSurfaceData;

/* Paint type surface point	*/
typedef struct PaintPoint {

	/* Wet paint is handled at effect layer only
	*  and mixed to surface when drying */
	float e_color[3];
	float e_alpha;
	float wetness;
	short state;
	float color[3];
	float alpha;
} PaintPoint;

/* heigh field waves	*/
typedef struct PaintWavePoint {		

	float height;
	float velocity;
	short state;
} PaintWavePoint;

struct DerivedMesh *dynamicPaint_Modifier_do(struct DynamicPaintModifierData *pmd, struct Scene *scene, struct Object *ob, struct DerivedMesh *dm);
void dynamicPaint_Modifier_free (struct DynamicPaintModifierData *pmd);
void dynamicPaint_Modifier_copy(struct DynamicPaintModifierData *pmd, struct DynamicPaintModifierData *tsmd);

int dynamicPaint_createType(struct DynamicPaintModifierData *pmd, int type, struct Scene *scene);
struct DynamicPaintSurface *dynamicPaint_createNewSurface(struct DynamicPaintCanvasSettings *canvas, struct Scene *scene);
void dynamicPaint_clearSurface(struct DynamicPaintSurface *surface);
int  dynamicPaint_resetSurface(struct DynamicPaintSurface *surface);
void dynamicPaint_freeSurface(struct DynamicPaintSurface *surface);
void dynamicPaint_freeCanvas(struct DynamicPaintModifierData *pmd);
void dynamicPaint_freeBrush(struct DynamicPaintModifierData *pmd);
void dynamicPaint_freeSurfaceData(struct DynamicPaintSurface *surface);

void dynamicPaint_cacheUpdateFrames(struct DynamicPaintSurface *surface);
int  dynamicPaint_surfaceHasColorPreview(struct DynamicPaintSurface *surface);
int dynamicPaint_outputLayerExists(struct DynamicPaintSurface *surface, struct Object *ob, int output);
void dynamicPaintSurface_updateType(struct DynamicPaintSurface *surface);
void dynamicPaintSurface_setUniqueName(struct DynamicPaintSurface *surface, const char *basename);
void dynamicPaint_resetPreview(struct DynamicPaintCanvasSettings *canvas);
struct DynamicPaintSurface *get_activeSurface(struct DynamicPaintCanvasSettings *canvas);

/* image sequence baking */
int dynamicPaint_createUVSurface(struct DynamicPaintSurface *surface);
int dynamicPaint_calculateFrame(struct DynamicPaintSurface *surface, struct Scene *scene, struct Object *cObject, int frame);
void dynamicPaint_outputSurfaceImage(struct DynamicPaintSurface *surface, char* filename, short output_layer);

/* PaintPoint state */
#define DPAINT_PAINT_NONE -1
#define DPAINT_PAINT_DRY 0
#define DPAINT_PAINT_WET 1
#define DPAINT_PAINT_NEW 2

/* PaintWavePoint state */
#define DPAINT_WAVE_NONE 0
#define DPAINT_WAVE_OBSTACLE 1
#define DPAINT_WAVE_REFLECT_ONLY 2

#endif /* BKE_DYNAMIC_PAINT_H_ */
