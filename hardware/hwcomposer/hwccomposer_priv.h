#ifndef __HWCOMPOSER_PRIV_H__
#define __HWCOMPOSER_PRIV_H__

#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hwcomposer.h>

#include <EGL/egl.h>

enum
{
    HWC_STATUS_HAVE_FRAME       = 1,
    HWC_STATUS_COMPOSITED       = 2,
    HWC_STATUS_OPENED           = 4,
    HWC_STATUS_HAVE_VIDEO       = 8,

    HWC_STATUS_HAVE_FRAME_MASK  = 0xfffffffe,
    HWC_STATUS_OPENED_MASK      = 0xfffffffb,
    HWC_STATUS_HAVE_VIDEO_MASK  = 0xfffffff7,
};

typedef enum e_hwc_layer_cmd {
    /* rotation of the source image in degrees (0 to 359) */
    HWC_LAYER_ROTATION_DEG          = 1,
    /* enable or disable dithering */
    HWC_LAYER_DITHER                = 3,
    /* transformation applied (this is a superset of COPYBIT_ROTATION_DEG) */
    HWC_LAYER_SETINITPARA = 4,
    /* set videoplayer init overlay parameter */
    HWC_LAYER_SETVIDEOPARA = 5,
    /* set videoplayer play frame overlay parameter*/
    HWC_LAYER_SETFRAMEPARA = 6,
    /* get videoplayer play frame overlay parameter*/
    HWC_LAYER_GETCURFRAMEPARA = 7,
    /* query video blank interrupt*/
    HWC_LAYER_QUERYVBI = 8,
    /* set overlay screen id*/
    HWC_LAYER_SETMODE = 9,

    HWC_LAYER_SHOW = 0xa,
    HWC_LAYER_RELEASE = 0xb,
    HWC_LAYER_SET3DMODE = 0xc,
    HWC_LAYER_SETFORMAT = 0xd,
    HWC_LAYER_VPPON = 0xe,
    HWC_LAYER_VPPGETON = 0xf,
    HWC_LAYER_SETLUMASHARP = 0x10,
    HWC_LAYER_GETLUMASHARP = 0x11,
    HWC_LAYER_SETCHROMASHARP = 0x12,
    HWC_LAYER_GETCHROMASHARP = 0x13,
    HWC_LAYER_SETWHITEEXTEN = 0x14,
    HWC_LAYER_GETWHITEEXTEN = 0x15,
    HWC_LAYER_SETBLACKEXTEN = 0x16,
    HWC_LAYER_GETBLACKEXTEN = 0x17,
    HWC_LAYER_SET_3D_PARALLAX = 0x18,
    HWC_LAYER_SET_SCREEN_PARA = 0x19,


    HWC_LAYER_SETTOP                = 0x1a,
    HWC_LAYER_SETBOTTOM                = 0x1b,
} e_hwc_layer_cmd_t;


typedef enum e_hwc_mode
{
    HWC_MODE_SCREEN0                = 0,
    HWC_MODE_SCREEN1                = 1,
    HWC_MODE_SCREEN0_TO_SCREEN1     = 2,
    HWC_MODE_SCREEN0_AND_SCREEN1    = 3,
    HWC_MODE_SCREEN0_BE             = 4,
    HWC_MODE_SCREEN0_GPU            = 5,
} e_hwc_mode_t;

typedef enum e_hwc_format 
{
    HWC_FORMAT_MINVALUE     = 0x50,
    HWC_FORMAT_RGBA_8888    = 0x51,
    HWC_FORMAT_RGB_565      = 0x52,
    HWC_FORMAT_BGRA_8888    = 0x53,
    HWC_FORMAT_YCbYCr_422_I = 0x54,
    HWC_FORMAT_CbYCrY_422_I = 0x55,
    HWC_FORMAT_MBYUV420                = 0x56,
    HWC_FORMAT_MBYUV422                = 0x57,
    HWC_FORMAT_YUV420PLANAR        = 0x58,
    HWC_FORMAT_DEFAULT      = 0x99,    // The actual color format is determined
    HWC_FORMAT_MAXVALUE     = 0x100
} e_hwc_format_t;

typedef enum e_hwc_3d_src_mode 
{
    HWC_3D_SRC_MODE_TB = 0x0,//top bottom
    HWC_3D_SRC_MODE_FP = 0x1,//frame packing
    HWC_3D_SRC_MODE_SSF = 0x2,//side by side full
    HWC_3D_SRC_MODE_SSH = 0x3,//side by side half
    HWC_3D_SRC_MODE_LI = 0x4,//line interleaved


    HWC_3D_SRC_MODE_NORMAL = 0xFF//2d
} e_hwc_3d_src_mode_t;

typedef enum e_hwc_3d_out_mode{
    HWC_3D_OUT_MODE_2D                             = 0x0,//left picture
    HWC_3D_OUT_MODE_HDMI_3D_1080P24_FP         = 0x1,
    HWC_3D_OUT_MODE_ANAGLAGH                 = 0x2,//·ЦЙ«
    HWC_3D_OUT_MODE_ORIGINAL                 = 0x3,//original pixture


    HWC_3D_OUT_MODE_LI                  = 0x4,//line interleaved
    HWC_3D_OUT_MODE_CI_1                = 0x5,//column interlaved 1
    HWC_3D_OUT_MODE_CI_2                = 0x6,//column interlaved 2
    HWC_3D_OUT_MODE_CI_3                = 0x7,//column interlaved 3
    HWC_3D_OUT_MODE_CI_4                = 0x8,//column interlaved 4


    HWC_3D_OUT_MODE_HDMI_3D_720P50_FP   = 0x9,
    HWC_3D_OUT_MODE_HDMI_3D_720P60_FP   = 0xa
} e_hwc_3d_out_mode_t;

typedef enum tag_RepeatField
{
    REPEAT_FIELD_NONE,          //means no field should be repeated


    REPEAT_FIELD_TOP,           //means the top field should be repeated
    REPEAT_FIELD_BOTTOM,        //means the bottom field should be repeated


    REPEAT_FIELD_
} repeatfield_t;

typedef struct tag_HWCLayerInitPara
{
    uint32_t                w;
    uint32_t                h;
    uint32_t                format;
    uint32_t                screenid;
} layerinitpara_t;

typedef struct tag_VideoInfo
{
    unsigned short              width;          //the stored picture width for luma because of mapping
    unsigned short              height;         //the stored picture height for luma because of mapping
    unsigned short              frame_rate;     //the source picture frame rate
    unsigned short              eAspectRatio;   //the source picture aspect ratio
    unsigned short              color_format;   //the source picture color format
} videoinfo_t;

typedef struct tag_Video3DInfo
{
    unsigned int width;
    unsigned int height;
    e_hwc_format_t format;
    e_hwc_3d_src_mode_t src_mode;
    e_hwc_3d_out_mode_t display_mode;
} video3Dinfo_t;

typedef struct tag_PanScanInfo
{
    unsigned long               uNumberOfOffset;
    signed short                HorizontalOffsets[3];
} panscaninfo_t;

typedef struct tag_VdrvRect
{
    signed short                uStartX;    // Horizontal start point.
    signed short                uStartY;    // Vertical start point.
    signed short                uWidth;     // Horizontal size.
    signed short                uHeight;    // Vertical size.
} vdrvrect_t;

typedef struct screen_para
{
    unsigned int width[2];//screen total width
    unsigned int height[2];//screen total height
    unsigned int valid_width[2];//screen width that can be seen
    unsigned int valid_height[2];//screen height that can be seen
    unsigned int app_width[2];//the width that app use
    unsigned int app_height[2];//the height that app use
} screen_para_t;

typedef struct tag_LIBHWCLAYERPARA
{
    unsigned long   number;

    repeatfield_t               eRepeatField;       // only check it when frame rate is 24FPS and interlace output
    videoinfo_t                 pVideoInfo;         // a pointer to structure stored video information
    panscaninfo_t               pPanScanInfo;
    vdrvrect_t                  src_rect;           // source valid size
    vdrvrect_t                  dst_rect;           // source display size
    unsigned char               top_index;          // frame buffer index containing the top field
    unsigned long   top_y;              // the address of frame buffer, which contains top field luminance
    unsigned long   top_c;              // the address of frame buffer, which contains top field chrominance
    unsigned long   bottom_y;           // the address of frame buffer, which contains bottom field luminance
    unsigned long   bottom_c;           // the address of frame buffer, which contains bottom field chrominance

    //the following is just for future
    unsigned char               bottom_index;       // frame buffer index containing the bottom field

    //time stamp of the frame
    unsigned long               uPts;               // time stamp of the frame (ms?)
    unsigned char        first_frame_flg;
    signed char     bProgressiveSrc;    // Indicating the source is progressive or not
    signed char     bTopFieldFirst;     // VPO should check this flag when bProgressiveSrc is FALSE
    unsigned long   flag_addr;          //dit maf flag address
    unsigned long   flag_stride;        //dit maf flag line stride
    unsigned char   maf_valid;
    unsigned char   pre_frame_valid;
} libhwclayerpara_t;

typedef struct hwc_context_t 
{
    hwc_composer_device_1_t 	device;
    hwc_procs_t 			*procs;
    int						dispfd;
    int                     mFD_fb[2];
    e_hwc_mode_t            mode;
    screen_para_t           screen_para;
    bool		    b_video_in_valid_area;

    uint32_t                ui_layerhdl[2];
    uint32_t                video_layerhdl[2];
    uint32_t                status[2];
    uint32_t		        w;
    uint32_t		        h;
    uint32_t		        format;
    bool		    cur_3denable;
    e_hwc_3d_src_mode_t     cur_3d_src;
    e_hwc_3d_out_mode_t     cur_3d_out;
    uint32_t                cur_3d_w;
    uint32_t                cur_3d_h;
    e_hwc_format_t          cur_3d_format;
    hwc_rect_t              rect_in;
    hwc_rect_t              rect_out;
    hwc_rect_t              rect_out_active[2];
    __disp_tv_mode_t        org_hdmi_mode;
    __disp_rect_t           org_scn_win;
    libhwclayerpara_t       cur_frame_para;
    bool                    layer_para_set;
    bool                    vsync_enabled;
    pthread_t               vsync_thread;
}sun4i_hwc_context_t;

extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
                           const struct timespec *request,
                           struct timespec *remain);

#endif

