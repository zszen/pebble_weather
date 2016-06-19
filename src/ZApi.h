#pragma once
#include <pebble.h>

static BitmapLayer* ZAPI_ImageCreate(uint32_t resourceId, GRect rect, bool isTransparent){
// 	APP_LOG(APP_LOG_LEVEL_WARNING, "image %ld",resourceId);
	GBitmap* bitmap = gbitmap_create_with_resource(resourceId);
	BitmapLayer* layer = bitmap_layer_create(rect);
	if(isTransparent){
		bitmap_layer_set_compositing_mode(layer, GCompOpSet);
	}
	bitmap_layer_set_bitmap(layer, bitmap);
	return layer;
}

static void ZAPI_ImageChange(BitmapLayer* bitmapLayer, uint32_t resourceId){
// 	APP_LOG(APP_LOG_LEVEL_WARNING, "image %ld",resourceId);
	GBitmap* bitmap = gbitmap_create_with_resource(resourceId);
	bitmap_layer_set_bitmap(bitmapLayer, bitmap);
}