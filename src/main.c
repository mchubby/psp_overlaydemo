#include <pspkernel.h>
#include <pspdisplay.h>
#include <string.h>
#include "cmlibmenu.h"

#define PLUGIN_NAME "overlaydemo"
#define PLUGIN_MAJ 0
#define PLUGIN_MIN 2
PSP_MODULE_INFO(PLUGIN_NAME, PSP_MODULE_KERNEL, PLUGIN_MAJ, PLUGIN_MIN);
PSP_HEAP_SIZE_KB(-1);

static char ms_libmenu_path[] = { "ms0:/seplugins/lib/cmlibmenu.prx"};
static char ef_libmenu_path[] = { "ef0:/seplugins/lib/cmlibmenu.prx"};

int (*sceDisplaySetFrameBuf_Org)(void *, int, int, int);
libm_draw_info dinfo;
libm_vram_info vinfo;

SceUID LoadStartModule(char *module) {
	SceUID mod = sceKernelLoadModule(module, 0, NULL);
	if (mod < 0)
		return mod;

	return sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
}

void init_modules() {
    while(1) {
        if(sceKernelFindModuleByName("sceKernelLibrary")){
            // prxlibmenu loading Check
            if( sceKernelFindModuleByName("cmlibMenu") == NULL ) {
                LoadStartModule(ms_libmenu_path);
                if( sceKernelFindModuleByName("cmlibMenu") == NULL ){
                    // PSP Go Built-in Memory Support
                    LoadStartModule(ef_libmenu_path);
                }
            }
            break;
        }
        sceKernelDelayThread(1000);
    }
}

////////////////////////////////////////////////////////////////////////////////
int sceDisplaySetFrameBuf_Patched(void *topaddr, int bufferwidth, int pixelformat, int sync) {

    /* set unbuffered framebuffer access */
    u32 ptr = (u32)topaddr;
    ptr |= ((ptr & 0x80000000) ? 0xA0000000 : 0x40000000);

    // avoid flickering most of the time with uncached buffer access
    libmInitBuffers_ForHook(LIBM_DRAW_BLEND, (void*)ptr, bufferwidth, pixelformat, sync, &dinfo);

    libmFrame(5,55, 95,155, 0xff00FF00, &dinfo);
    libmPrintXY(0,256,0xff00FFFF,0x5cFFFFFF, "Overlay demo text", &dinfo);
    libmFillRect(10,50, 100,150, 0xffFF0000, &dinfo);
#if 1
    sync = PSP_DISPLAY_SETBUF_IMMEDIATE;
#endif
    return sceDisplaySetFrameBuf_Org ? sceDisplaySetFrameBuf_Org(topaddr, bufferwidth, pixelformat, sync) : 0;
}

int thread_start(SceSize args, void *argp) {
    init_modules();

    // load 0x00-0x7F + SJIS range
    libmLoadFont(LIBM_FONT_CG);
    libmLoadFont(LIBM_FONT_SJIS);

    memset(&dinfo, 0, sizeof(dinfo));    memset(&vinfo, 0, sizeof(vinfo));
    dinfo.vinfo = &vinfo;

    // This needs to be called at least once
    libmInitBuffers(0x00000000, 0, &dinfo);

    sceDisplaySetFrameBuf_Org = libmHookDisplayHandler(sceDisplaySetFrameBuf_Patched);

    sceKernelSleepThread();
    return 0;
}

int module_start(SceSize args, void *argp) {
    SceUID thid = sceKernelCreateThread("overlaydemo_thread", thread_start, 0x22, 0x2000, 0, NULL);
    if(thid >= 0) {
        sceKernelStartThread(thid, args, argp);
    }
    return 0;
}

int module_stop(SceSize args, void *argp) {
    return 0;
}
