/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <sys/ioctl.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hwcomposer.h>

#include <EGL/egl.h>

#include <pthread.h>
#include <utils/Timers.h>
#include <sys/resource.h>
#include <signal.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <linux/netlink.h>
#include <poll.h>
#include <hardware_legacy/uevent.h>
#include <cutils/properties.h>
#include <system/thread_defs.h>

#include "drv_display_sun4i.h"
#include "gralloc_priv.h"

#define GET_UMP_PHYSICAL_ADDRESS 0x4702

/*****************************************************************************/

struct hwc_context_t {
    hwc_composer_device_1_t device;

    /* our private state goes below here */
	hwc_procs_t* m_callback;

	int m_fb_fd;
	int m_disp_fd;
	uint32_t ui_layerhdl;
	uint32_t video_layerhdl;
	bool video_opened;
	bool have_frame;
	private_handle_t * previous_handle;
	uint32_t count;

	bool vsync_enabled;
	pthread_t vsync_thread;
};

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static struct hw_module_methods_t hwc_module_methods = {
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: HWC_HARDWARE_MODULE_ID,
        name: "Sample hwcomposer module",
        author: "The Android Open Source Project",
        methods: &hwc_module_methods,
    }
};

/*****************************************************************************/

static void dump_layer(hwc_layer_1_t const* l) {
    ALOGD("\ttype=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%d,%d,%d,%d}, {%d,%d,%d,%d}",
            l->compositionType, l->flags, l->handle, l->transform, l->blending,
            l->sourceCrop.left,
            l->sourceCrop.top,
            l->sourceCrop.right,
            l->sourceCrop.bottom,
            l->displayFrame.left,
            l->displayFrame.top,
            l->displayFrame.right,
            l->displayFrame.bottom);
}

static int hwc_video_open(struct hwc_context_t *ctx)
{
	unsigned long args[4] = {0};

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_OPEN, args);

	//args[0]                         = 0;
	//args[1]                         = ctx->video_layerhdl;
	//ioctl(ctx->m_disp_fd, DISP_CMD_VIDEO_START, args);

	//ctx->have_frame = false;

	return 0;
}

static int hwc_video_close(struct hwc_context_t *ctx)
{
	unsigned long args[4] = {0};

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_CLOSE, args);

	//args[0]                         = 0;
	//args[1]                         = ctx->video_layerhdl;
	//ioctl(ctx->m_disp_fd, DISP_CMD_VIDEO_STOP, args);

	ctx->previous_handle = NULL;

	ctx->have_frame = false;
	ctx->count = 0;

	return 0;
}

#define _ALIGN( value, base ) (((value) + ((base) - 1)) & ~((base) - 1))

static int hwc_video_set_fb(struct hwc_context_t *ctx, hwc_layer_1_t *layer)
{
	unsigned long args[4] = {0};
	int ret;

	__disp_fb_t fb;

	private_handle_t * priv_handle = (private_handle_t*) layer->handle;

	if (priv_handle == ctx->previous_handle)
		return 0;

	memset(&fb, 0, sizeof(fb));

#if 0
	if (priv_handle->ump_id > 0)
	{
		unsigned long paddr = 0;
		args[0] = (unsigned long) priv_handle->ump_id;
		args[1] = (unsigned long) &paddr;
		ioctl(ctx->m_fb_fd, GET_UMP_PHYSICAL_ADDRESS, (unsigned long) args);
#endif
		fb.addr[0] = priv_handle->paddr;
		fb.addr[2] = priv_handle->paddr + priv_handle->width * priv_handle->height;
		fb.addr[1] = fb.addr[2] + (_ALIGN(priv_handle->width /2 , 16) * priv_handle->height) / 2;
#if 0
	}
#endif

	//ALOGE("size (%d x %d)", priv_handle->width, priv_handle->height);
	fb.size.width = priv_handle->width;
	fb.size.height = priv_handle->height;

	//fb.size.width = layer->sourceCrop.right - layer->sourceCrop.left;
	//fb.size.height = layer->sourceCrop.bottom - layer->sourceCrop.top;
	fb.format = DISP_FORMAT_YUV420;
	fb.seq = DISP_SEQ_P3210;
	fb.mode = DISP_MOD_NON_MB_PLANAR;
	fb.cs_mode = DISP_BT709;

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	args[2] = (unsigned long) (&fb);
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_SET_FB, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	ctx->previous_handle = priv_handle;

	return 0;
}

static int hwc_video_set_frame_para(struct hwc_context_t *ctx, hwc_layer_1_t *layer)
{
	__disp_video_fb_t video_info;
#if 0
	__disp_fb_t fb;
	__disp_rect_t buf_rect;
	__disp_rect_t win_rect;

	buf_rect.x = layer->sourceCrop.left;
    buf_rect.y = layer->sourceCrop.top;
	buf_rect.width = layer->sourceCrop.right - layer->sourceCrop.left;
	buf_rect.height = layer->sourceCrop.bottom - layer->sourceCrop.top;

	win_rect.x = layer->displayFrame.left;
    win_rect.y = layer->displayFrame.top;
	win_rect.width = layer->displayFrame.right - layer->displayFrame.left;
	win_rect.height = layer->displayFrame.bottom - layer->displayFrame.top;
#endif

	unsigned long args[4] = {0};
	int ret;

	private_handle_t * priv_handle = (private_handle_t*) layer->handle;

#if 0
	ALOGE("%s: u = %x v =%x", __func__, handle->picture.y, handle->picture.u);

	video_info.interlace         = (handle->picture.is_progressive ? 0 : 1);
	video_info.top_field_first   = handle->picture.top_field_first;
	video_info.addr[0]           = (u32) handle->picture.y;
	video_info.addr[1]           = (u32) handle->picture.u;
	video_info.addr[2]           = 0;
	video_info.addr_right[0]     = 0;
	video_info.addr_right[1]     = 0;
	video_info.addr_right[2]     = 0;
	video_info.id                = 0; //overlaypara->number;
	video_info.maf_valid         = handle->picture.maf_valid;
	video_info.pre_frame_valid   = handle->picture.pre_frame_valid;
	video_info.flag_addr         = handle->picture.flag_addr;
	video_info.flag_stride       = handle->picture.flag_stride;
#endif

	//memset(&fb, 0, sizeof(fb));

	if (priv_handle->ump_id > 0)
	{
		unsigned long paddr = 0;
		args[0] = (unsigned long) priv_handle->ump_id;
		args[1] = (unsigned long) &paddr;
		ioctl(ctx->m_fb_fd, GET_UMP_PHYSICAL_ADDRESS, (unsigned long) args);

#if 0
		fb.addr[0] = paddr;
		fb.addr[2] = paddr + priv_handle->uoffset;
		fb.addr[1] = fb.addr[2] + priv_handle->voffset;
#endif

		video_info.addr[0] = paddr;
		video_info.addr[2] = paddr + priv_handle->uoffset;
		video_info.addr[1] = video_info.addr[2] + priv_handle->voffset;
	}

#if 0
	fb.size.width = buf_rect.width;
	fb.size.height = buf_rect.height;
	fb.format = DISP_FORMAT_YUV420;
	fb.seq = DISP_SEQ_P3210;
	fb.mode = DISP_MOD_NON_MB_PLANAR;

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	args[2] = (unsigned long) (&fb);
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_SET_FB, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	args[2] = (unsigned long) (&buf_rect);
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_SET_SRC_WINDOW, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	args[2] = (unsigned long) (&win_rect);
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_SET_SCN_WINDOW, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	ctx->have_frame = true;
#endif

#if 0
	video_info.interlace         = (priv_handle->is_progressive ? 0 : 1);
	video_info.top_field_first   = priv_handle->top_field_first;
#endif
	video_info.interlace         = 1;
	video_info.top_field_first   = 0;

	video_info.addr_right[0]     = 0;
	video_info.addr_right[1]     = 0;
	video_info.addr_right[2]     = 0;
	video_info.id                = 0; //overlaypara->number;

#if 0
	video_info.maf_valid         = priv_handle->maf_valid;
	video_info.pre_frame_valid   = priv_handle->pre_frame_valid;
	video_info.flag_addr         = priv_handle->flag_addr;
	video_info.flag_stride       = priv_handle->flag_stride;
#endif

	if (!ctx->have_frame)
	{
		__disp_layer_info_t         layer_info;

		args[0]                 = 0;
		args[1]                 = ctx->video_layerhdl;
		args[2]                 = (unsigned long) (&layer_info);
		args[3]                 = 0;
		ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_GET_PARA, args);
		if(ret < 0)
		{
			ALOGD("####DISP_CMD_LAYER_GET_PARA fail in %s, screen_idx:%d, hdl:%d\n",__FUNCTION__, 0, ctx->video_layerhdl);
		}

		layer_info.src_win.x = layer->sourceCrop.left;
		layer_info.src_win.y = layer->sourceCrop.top;
		layer_info.src_win.width = layer->sourceCrop.right - layer->sourceCrop.left;
		layer_info.src_win.height = layer->sourceCrop.bottom - layer->sourceCrop.top;

		layer_info.scn_win.x = layer->displayFrame.left;
		layer_info.scn_win.y = layer->displayFrame.top;
		layer_info.scn_win.width = layer->displayFrame.right - layer->displayFrame.left;
		layer_info.scn_win.height = layer->displayFrame.bottom - layer->displayFrame.top;

		layer_info.fb.addr[0]   = video_info.addr[0];
		layer_info.fb.addr[1]   = video_info.addr[1];
		layer_info.fb.addr[2]   = video_info.addr[2];
		layer_info.fb.trd_right_addr[0] = video_info.addr_right[0];
		layer_info.fb.trd_right_addr[1] = video_info.addr_right[1];
		layer_info.fb.trd_right_addr[2] = video_info.addr_right[2];

		layer_info.fb.size.width = layer_info.src_win.width;
		layer_info.fb.size.height = layer_info.src_win.height;

#if 0
		ALOGD("src_win.x = %d src_win.y = %d src_win.width = %d src_win.height = %d", layer_info.src_win.x, layer_info.src_win.y, layer_info.src_win.width, layer_info.src_win.height);

		ALOGD("scn_win.x = %d scn_win.y = %d scn_win.width = %d scn_win.height = %d", layer_info.scn_win.x, layer_info.scn_win.y, layer_info.scn_win.width, layer_info.scn_win.height);

		ALOGD("layer_info.fb.size.width = %d layer_info.fb.size.height = %d", layer_info.fb.size.width, layer_info.fb.size.height);
#endif

		args[0]                 = 0;
		args[1]                 = ctx->video_layerhdl;
		args[2]                 = (unsigned long) (&layer_info);
		args[3]                 = 0;
		ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_SET_PARA, args);

		ctx->have_frame = true;
	}

	args[0]                 = 0;
	args[1]                 = ctx->video_layerhdl;
	args[2]                 = (unsigned long) (&video_info);
	args[3]                 = 0;
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_VIDEO_SET_FB, args);

	return 0;
}

static int sunxi_layer_set_colorkey(struct hwc_context_t *ctx, uint32_t color)
{
	int ret;
	unsigned long args[4] = {0};

	__disp_colorkey_t colorkey;
	__disp_color_t disp_color;

	disp_color.alpha = (color >> 24) & 0xFF;
	disp_color.red   = (color >> 16) & 0xFF;
	disp_color.green = (color >> 8)  & 0xFF;
	disp_color.blue  = (color >> 0)  & 0xFF;

	colorkey.ck_min = disp_color;
	colorkey.ck_max = disp_color;
	colorkey.red_match_rule   = 2;
	colorkey.green_match_rule = 2;
	colorkey.blue_match_rule  = 2;

	args[0] = 0;
	args[1] = (unsigned int) (&colorkey);
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_SET_COLORKEY, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	//args[0] = 0;
	//args[1] = ctx->ui_layerhdl;
	//ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_BOTTOM, args);
	//ALOGE_IF(ret != 0, "error %d", __LINE__);

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_BOTTOM, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_CK_ON, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	args[0] = 0;
	args[1] = ctx->ui_layerhdl;
	//ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_CK_ON, args);
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_CK_OFF, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	args[0] = 0;
	args[1] = ctx->ui_layerhdl;
	//ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_ALPHA_ON, args);
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_ALPHA_OFF, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	return 0;
}

static int sunxi_layer_disable_colorkey(struct hwc_context_t *ctx)
{
	int ret;
	unsigned long args[4] = {0};

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_CK_OFF, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	args[0] = 0;
	args[1] = ctx->video_layerhdl;
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_BOTTOM, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	args[0] = 0;
	args[1] = ctx->ui_layerhdl;
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_BOTTOM, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	return 0;
}

static int hwc_video_request(struct hwc_context_t * ctx, int w, int h);
static int hwc_video_release(struct hwc_context_t * ctx);

static int hwc_prepare(hwc_composer_device_1_t *dev,
		size_t numDisplays, hwc_display_contents_1_t** displays)
{
	bool haveOverlay = false;

    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

	int width = 0;
	int height = 0;

	for (size_t i=0 ; i < displays[0]->numHwLayers ; i++)
	{
		//dump_layer(&list->hwLayers[i]);
#if 0
		if (displays[0]->hwLayers[i].handle)
		{
			private_handle_t * priv_handle = (private_handle_t*) displays[0]->hwLayers[i].handle;

			if (priv_handle->format == HAL_PIXEL_FORMAT_YV12)
			{
				displays[0]->hwLayers[i].compositionType = HWC_OVERLAY;
				//if (!ctx->video_opened)
				//	sunxi_layer_set_colorkey(ctx, 0x081018);
				// hwc_video_set_frame_para(ctx, &displays[0]->hwLayers[i]);
				if (ctx->video_layerhdl == 0)
					hwc_video_request(ctx, priv_handle->width, priv_handle->height);
				haveOverlay = true;
			}
			else
			{
				displays[0]->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
			}
		}
		else
#endif
			displays[0]->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
	}

	if (!haveOverlay)
	{
		if (ctx->video_layerhdl)
			hwc_video_release(ctx);
	}

    return 0;
}

static int hwc_set(hwc_composer_device_1_t *dev,
		        size_t numDisplays, hwc_display_contents_1_t** displays)
{
    //for (size_t i=0 ; i<list->numHwLayers ; i++) {
    //    dump_layer(&list->hwLayers[i]);
    //}

	bool haveOverlay = false;

    EGLBoolean sucess = eglSwapBuffers((EGLDisplay) displays[0]->dpy,
			(EGLSurface) displays[0]->sur);
    if (!sucess) {
        return HWC_EGL_ERROR;
    }

	struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

	for (size_t i=0 ; i < displays[0]->numHwLayers ; i++)
	{
		//dump_layer(&list->hwLayers[i]);
		if (displays[0]->hwLayers[i].compositionType == HWC_OVERLAY)
		{
			hwc_video_set_frame_para(ctx, &displays[0]->hwLayers[i]);
			haveOverlay = true;
		}
	}

	if (haveOverlay)
	{
		if (!ctx->video_opened)
		{
			hwc_video_open(ctx);
			ctx->video_opened = true;
		}
	}
	else
	{
		if (ctx->video_opened)
		{
			hwc_video_close(ctx);
			ctx->video_opened = false;
		}
	}

    return 0;
}

static int hwc_device_close(struct hw_device_t *dev)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    if (ctx) {
        free(ctx);
    }
    return 0;
}

static int hwc_video_request(struct hwc_context_t * ctx, int w, int h)
{
	int ret;
	unsigned long args[4] = {0};
	__disp_layer_info_t layer_info;
	__disp_colorkey_t ck;

	ioctl(ctx->m_fb_fd, FBIOGET_LAYER_HDL_0, &ctx->ui_layerhdl);

	args[0] = 0;
	ctx->video_layerhdl = (uint32_t) ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_REQUEST, args);

	if (ctx->video_layerhdl == 0)
		return -1;

	memset(&layer_info, 0, sizeof(__disp_layer_info_t));

	layer_info.fb.cs_mode = DISP_BT709;
	layer_info.fb.format = DISP_FORMAT_YUV420;
	layer_info.fb.mode = DISP_MOD_NON_MB_PLANAR;
	layer_info.fb.seq = DISP_SEQ_P3210;

	layer_info.fb.br_swap         = 0;
	layer_info.fb.addr[0]         = 0;
	layer_info.fb.addr[1]         = 0;
	layer_info.fb.addr[2]         = 0;
	layer_info.fb.size.width      = w;
	layer_info.fb.size.height     = h;
	layer_info.mode               = DISP_LAYER_WORK_MODE_SCALER;
	layer_info.alpha_en           = 1;
	layer_info.alpha_val          = 0xff;
	layer_info.pipe               = 1;
	layer_info.src_win.x          = 0;
	layer_info.src_win.y          = 0;
	layer_info.src_win.width      = 1;
	layer_info.src_win.height     = 1;
	layer_info.scn_win.x          = 0;
	layer_info.scn_win.y          = 0;
	layer_info.scn_win.width      = 1;
	layer_info.scn_win.height     = 1;

	args[0]                         = 0;
	args[1]                         = ctx->video_layerhdl;
	args[2]                         = (unsigned long) (&layer_info);
	args[3]                         = 0;
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_SET_PARA, args);
	ALOGE_IF(ret != 0, "error %d", __LINE__);

	sunxi_layer_set_colorkey(ctx, 0x081018);

	args[0]                         = 0;
	args[1]                         = ctx->video_layerhdl;
	ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_BOTTOM, args);

	ck.ck_min.alpha                 = 0x00;
	ck.ck_min.red                   = 0x08; //0x01;
	ck.ck_min.green                 = 0x10; //0x03;
	ck.ck_min.blue                  = 0x18; //0x05;
	ck.ck_max.alpha                 = 0x00;
	ck.ck_max.red                   = 0x08; //0x01;
	ck.ck_max.green                 = 0x10; //0x03;
	ck.ck_max.blue                  = 0x18; //0x05;
	ck.red_match_rule               = 2;
	ck.green_match_rule             = 2;
	ck.blue_match_rule              = 2;
	args[0]                         = 0;
	args[1]                         = (unsigned long) &ck;
	ioctl(ctx->m_disp_fd, DISP_CMD_SET_COLORKEY, (void*) args);

	args[0]                         = 0;
	args[1]                         = ctx->ui_layerhdl;
	ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_CK_OFF, (void*) args);

	args[0]                         = 0;
	args[1]                         = ctx->ui_layerhdl;
	args[2]                         = (unsigned long) (&layer_info);
	args[3]                         = 0;
	ret = ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_GET_PARA, args);
	if (ret < 0)
	{
		ALOGE("DISP_CMD_LAYER_GET_PARA fail");
	}

	args[0]                     	= 0;
	args[1]                     	= ctx->video_layerhdl;
	ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_CK_ON, (void*) args);

	args[0]                         = 0;
	args[1]                         = ctx->ui_layerhdl;
	ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_ALPHA_OFF, args);

	args[0]                         = 0;
	args[1]                         = ctx->video_layerhdl;
	ioctl(ctx->m_disp_fd, DISP_CMD_VIDEO_START, args);

	return 0;
}

static int hwc_video_release(struct hwc_context_t * ctx)
{
	int ret;
	unsigned long args[4] = {0};

	args[0]                         = 0;
    args[1]                         = ctx->video_layerhdl;
    ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_CLOSE, args);

	args[0]                         = 0;
	args[1]                         = ctx->video_layerhdl;
	ioctl(ctx->m_disp_fd, DISP_CMD_VIDEO_STOP, args);

	usleep(20 * 1000);

	args[0]                         = 0;
	args[1]                         = ctx->video_layerhdl;
	ioctl(ctx->m_disp_fd, DISP_CMD_LAYER_RELEASE, args);

	ctx->video_layerhdl = 0;

	return 0;
}

extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
		const struct timespec *request,
		struct timespec *remain);

//#define DEBUG_HWC_VSYNC_TIMING 1

static void *hwc_vsync_thread(void *data)
{
#if 0
#define HWC_VSYNC_THREAD_NAME "hwcVsyncThread"

	const char* vsync_timestamp_fb0 = "/sys/class/disp/disp/attr/vsync0_event";

	hwc_context_t *ctx = (hwc_context_t *) data;

	char thread_name[64] = HWC_VSYNC_THREAD_NAME;
	prctl(PR_SET_NAME, (unsigned long) &thread_name, 0, 0, 0);
	setpriority(PRIO_PROCESS, 0, HAL_PRIORITY_URGENT_DISPLAY +
			ANDROID_PRIORITY_MORE_FAVORABLE);

	const int MAX_DATA = 64;
	static char vdata[MAX_DATA];

	uint64_t cur_timestamp=0;
	ssize_t len = -1;
	int fd_timestamp = -1;
	int ret = 0;
	bool fb1_vsync = false;
	bool fakevsync = false;
	bool logvsync = false;

	char property[PROPERTY_VALUE_MAX];
	if(property_get("debug.hwc.fakevsync", property, NULL) > 0) {
		if(atoi(property) == 1)
			fakevsync = true;
	}

	if(property_get("debug.hwc.logvsync", property, 0) > 0) {
		if(atoi(property) == 1)
			logvsync = true;
	}

    /* Currently read vsync timestamp from drivers
       e.g. VSYNC=41800875994
       */
    fd_timestamp = open(vsync_timestamp_fb0, O_RDONLY);
    if (fd_timestamp < 0) {
        // Make sure fb device is opened before starting this thread so this
        // never happens.
        ALOGE ("FATAL:%s:not able to open file:%s, %s",  __FUNCTION__,
               vsync_timestamp_fb0,
               strerror(errno));
        fakevsync = true;
    }

    do {
        if (!fakevsync) {
            len = pread(fd_timestamp, vdata, MAX_DATA, 0);
            if (len < 0) {
                // If the read was just interrupted - it is not a fatal error
                // In either case, just continue.
                if (errno != EAGAIN &&
                    errno != EINTR  &&
                    errno != EBUSY) {
                    ALOGE ("FATAL:%s:not able to read file:%s, %s",
                           __FUNCTION__,
                           vsync_timestamp_fb0, strerror(errno));
                }
                continue;
            }
            // extract timestamp
            const char *str = vdata;
            if (!strncmp(str, "VSYNC=", strlen("VSYNC="))) {
                cur_timestamp = strtoull(str + strlen("VSYNC="), NULL, 0);
            }
        } else {
            usleep(16666);
            cur_timestamp = systemTime();
        }
        // send timestamp to HAL
        if(ctx->vsync_enabled) {
            ALOGD_IF (logvsync, "%s: timestamp %llu sent to HWC for %s",
                      __FUNCTION__, cur_timestamp, "fb0");
            ctx->m_callback->vsync(ctx->m_callback, 0, cur_timestamp);
        }

    } while (true);

    if(fd_timestamp >= 0)
        close (fd_timestamp);

    return NULL;

#endif

#if 0
	struct sockaddr_nl snl;
	const int buffersize = 16*1024;
	int retval;
	int hotplug_sock;

	setpriority(PRIO_PROCESS, 0, HAL_PRIORITY_URGENT_DISPLAY);

	hwc_context_t *ctx = (hwc_context_t *) data;

	memset(&snl, 0x0, sizeof(snl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 0xffffffff;

	hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (hotplug_sock == -1) {
		ALOGE("####socket is failed in %s error:%d %s\n", __FUNCTION__, errno, strerror(errno));
		return NULL;
	}

	setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));

	retval = bind(hotplug_sock, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));

	if (retval < 0) {
		ALOGE("####bind is failed in %s error:%d %s\n", __FUNCTION__, errno, strerror(errno));
		close(hotplug_sock);
		return NULL;
	}

	while(1)
	{
		char buf[4096*2] = {0};
		struct pollfd fds;
		int err;

		fds.fd = hotplug_sock;
		fds.events = POLLIN;
		fds.revents = 0;

		err = poll(&fds, 1, 256);

		if(err > 0 && fds.events == POLLIN)
		{
			int count = recv(hotplug_sock, &buf, sizeof(buf),0);
			if(count > 0)
			{
				int IsVsync = !strcmp(buf, "change@/devices/platform/disp");

				if (IsVsync && ctx->vsync_enabled)
				{
					uint64_t timestamp = 0;
					unsigned int display_id = -1;
					const char *s = buf;

					s += strlen(s) + 1;
					while(s)
					{
						if (!strncmp(s, "VSYNC0=", strlen("VSYNC0=")))
						{
							timestamp = strtoull(s + strlen("VSYNC0="), NULL, 0);
							ALOGV("#### %s display0 timestamp:%lld", s,timestamp);
							display_id = 0;
						}
						else if (!strncmp(s, "VSYNC1=", strlen("VSYNC1=")))
						{
							timestamp = strtoull(s + strlen("VSYNC1="), NULL, 0);
							ALOGV("#### %s display1 timestamp:%lld", s,timestamp);
							display_id = 1;
						}

						s += strlen(s) + 1;
						if(s - buf >= count)
						{
							break;
						}
					}

					if(display_id == 0)//only handle display 0 vsync event now
					{
#ifdef DEBUG_HWC_VSYNC_TIMING
						static nsecs_t last_time_ns;
						nsecs_t cur_time_ns;

						cur_time_ns  = systemTime(SYSTEM_TIME_MONOTONIC);
						ctx->m_callback->vsync(ctx->m_callback, 0, timestamp);
						ALOGE("Vsync %llu, %llu\n", cur_time_ns - last_time_ns,
								cur_time_ns - timestamp);
						last_time_ns = cur_time_ns;
#else
						ctx->m_callback->vsync(ctx->m_callback, 0, timestamp);
#endif
					}
				}
			}
		}
	}
#endif

#if 1
#define HZ 30
	struct timespec spec;
	int err = 0;
	nsecs_t period, now, next_vsync, sleep, next_fake_vsync = 0;
	hwc_context_t *ctx = (hwc_context_t *) data;

	setpriority(PRIO_PROCESS, 0, HAL_PRIORITY_URGENT_DISPLAY);

	while (true) {
		period = 1000000000/HZ;
		next_vsync = next_fake_vsync;
		sleep = next_vsync - now;
		if (sleep < 0) {
			// we missed, find where the next vsync should be
			sleep = (period - ((now - next_vsync) % period));
			next_vsync = now + sleep;
		}
		next_fake_vsync = next_vsync + period;

		spec.tv_sec  = next_vsync / 1000000000;
		spec.tv_nsec = next_vsync % 1000000000;

		do {
			err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
		} while (err < 0 && errno == EINTR);

		if (ctx->vsync_enabled)
			ctx->m_callback->vsync(ctx->m_callback, 0, next_vsync);
	}
#endif

	return NULL;
}

static int hwc_blank(hwc_composer_device_1* a, int b, int c)
{
	/* STUB */
	return 0;
}

static int hwc_eventControl(hwc_composer_device_1* dev, int dpy, int event,
		int enabled)
{
	unsigned long args[4] = {0};

	struct hwc_context_t* ctx = (struct hwc_context_t *) dev;

	switch (event) {
		case HWC_EVENT_VSYNC:
			args[0] = 0;
			args[1] = !!enabled;
			ioctl(ctx->m_disp_fd, DISP_CMD_VSYNC_EVENT_EN, (unsigned long) args);
			ctx->vsync_enabled = !!enabled;
			return 0;
	}
	return -EINVAL;
}

static void hwc_registerProcs(struct hwc_composer_device_1* dev,
		hwc_procs_t const* procs)
{
	struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
	ctx->m_callback = const_cast<hwc_procs_t *>(procs);
}


/*****************************************************************************/

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    int status = -EINVAL;
    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        struct hwc_context_t *dev;
        dev = (hwc_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = HWC_DEVICE_API_VERSION_1_0;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = hwc_device_close;

        dev->device.prepare = hwc_prepare;
        dev->device.set = hwc_set;
		dev->device.blank = hwc_blank;
		dev->device.eventControl = hwc_eventControl;
		dev->device.registerProcs = hwc_registerProcs;

		dev->m_disp_fd = open("/dev/disp", O_RDWR);
		if (dev->m_disp_fd < 0)
		{
			ALOGE("Failed to open disp device, ret: %d, errno: %d", dev->m_disp_fd, errno);
			return -1;
		}

		dev->m_fb_fd = open("/dev/graphics/fb0", O_RDWR);
		if (dev->m_fb_fd < 0)
		{
			ALOGE("Failed to open fb0 device, ret: %d, errno: %d", dev->m_fb_fd, errno);
			close(dev->m_disp_fd);
			return -1;
		}

        *device = &dev->device.common;
        status = 0;

		status = pthread_create(&dev->vsync_thread, NULL, hwc_vsync_thread, dev);
		if (status) {
			ALOGE("%s::pthread_create() failed : %s", __func__, strerror(status));
			status = -status;
		}
    }
    return status;
}
