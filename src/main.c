#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspumd.h>
#include <string.h>
#include "sysctrldefs.h"
#include "cmlibmenu.h"

#define PLUGIN_NAME "overlaydemo"
#define PLUGIN_MAJ 0
#define PLUGIN_MIN 4
PSP_MODULE_INFO(PLUGIN_NAME, PSP_MODULE_KERNEL, PLUGIN_MAJ, PLUGIN_MIN);
PSP_HEAP_SIZE_KB(8);

typedef int (*StartModuleHandler)(SceModule2 *);
StartModuleHandler sctrlHENSetStartModuleHandler(StartModuleHandler handler);
static StartModuleHandler previousHandler = 0;

static char ms_libmenu_path[] = { "ms0:/seplugins/lib/cmlibmenu.prx"};
static char ef_libmenu_path[] = { "ef0:/seplugins/lib/cmlibmenu.prx"};

int (*sceDisplaySetFrameBuf_Org)(void *, int, int, int);
libm_draw_info dinfo;
libm_vram_info vinfo;

char gameid[16];
int plugin_enabled;
int configoption_latehook;
int configoption_flushcacheinhook;
int configoption_delayexectime;
int remember_pixelformat = -1;

SceUID LoadStartModule(char *module) {
	SceUID mod = sceKernelLoadModule(module, 0, NULL);
	if (mod < 0)
		return mod;

	return sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
}

void init_modules() {
    // prxlibmenu loading Check
    if( sceKernelFindModuleByName("cmlibMenu") == NULL ) {
        LoadStartModule(ms_libmenu_path);
        if( sceKernelFindModuleByName("cmlibMenu") == NULL ){
            // PSP Go Built-in Memory Support
            LoadStartModule(ef_libmenu_path);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
int sceDisplaySetFrameBuf_Patched(void *topaddr, int bufferwidth, int pixelformat, int sync) {
    if (plugin_enabled) {

        /* set unbuffered framebuffer access */
        u32 ptr = (u32)topaddr;
        ptr |= ((ptr & 0x80000000) ? 0xA0000000 : 0x40000000);

        // avoid flickering most of the time with uncached buffer access
        libmInitBuffers_ForHook(LIBM_DRAW_BLEND, (void*)ptr, bufferwidth, pixelformat, sync, &dinfo);

        libmFrame(5,55, 95,155, 0xff00FF00, &dinfo);
        libmPrintXY(0,256,0xff00FFFF,0x5cFFFFFF, "Overlay demo text", &dinfo);
        libmFillRect(10,50, 100,150, 0xffFF0000, &dinfo);
    }
    return sceDisplaySetFrameBuf_Org ? sceDisplaySetFrameBuf_Org(topaddr, bufferwidth, pixelformat, sync) : 0;
}



static void read_gameid_from_umd() {
    if (sceUmdGetDriveStat() != PSP_UMD_READY) {
        if (sceUmdWaitDriveStatWithTimer(PSP_UMD_PRESENT, 500000) < 0
                || sceUmdActivate(1, "disc0:") < 0
                || sceUmdWaitDriveStat(PSP_UMD_READY) < 0) {
            gameid[0] = 0;
            return;
        }
    }
    SceUID fd = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777);
    if (fd >= 0) {
        sceIoRead(fd, gameid, 10);
        sceIoClose(fd);
        if (gameid[4] == '-') {
            memmove(gameid + 4, gameid + 5, 5);
        }
        gameid[9] = 0;
    } else {
        gameid[0] = 0;
    }
}

static int get_delay_exec_time_from_gameid() {
    static const char *delays_id[] = {
        "NPJH90076", // Senjou no Valkyria II JP Demo
        "NPUH90071", // Valkyria Chronicles II US Demo
        "NPJH50145", // Senjou no Valkyria II JP
        "ULUS10515", // Valkyria Chronicles II US
        "ULES01417", // Valkyria Chronicles II EU

        "NPJH90295", // Dies Irae ~Amantes Amentes~ JP Demo
        "ULJM06107", // Dies Irae ~Amantes Amentes~ JP Disc1
        "ULJM06108", // Dies Irae ~Amantes Amentes~ JP Disc2

    };
    if (gameid[0] == 0) { return 0; }
    int i;
    int n = (int)(sizeof(delays_id) / sizeof(0[delays_id]));
    for(i = 0; i < n; ++i)
       if(strcmp(gameid, delays_id[i]) == 0)
           return 2000000;

    return 0;
}

static SceUID moduleLoadedTrafficLight = 0;
int modding = 0;

int overlaydemo_module_loader(SceModule2* m) {
    int ret = previousHandler != 0 ? previousHandler(m) : 0;
    if (m != 0) {
#if 0
        SceUID file = sceIoOpen("ms0:/overlaydemo.txt", PSP_O_APPEND | PSP_O_CREAT | PSP_O_WRONLY, 0777);
        if(file >= 0) {
            sceIoWrite(file, m->modname, strlen(m->modname) + 1);
            sceIoClose(file);
        }
#endif
        if (m->text_addr & 0x80000000
                || (m->modname[0] == 's' && m->modname[1] == 'c' && m->modname[2] == 'e')
                || (m->modname[0] == 'o' && m->modname[1] == 'p' && m->modname[2] == 'n')) {
            /* Skipping kernel and SCE modules */
        } else {
            if (!modding) {
                modding = 1;
                /* greenlit */
                sceKernelSignalSema(moduleLoadedTrafficLight, 1);
            }
        }
    }
    return ret;
}




int thread_start(SceSize args, void *argp) {
    read_gameid_from_umd();
    plugin_enabled = 1;
    configoption_latehook = 1;
    configoption_flushcacheinhook = 1;
    configoption_delayexectime = get_delay_exec_time_from_gameid();

    if((moduleLoadedTrafficLight = sceKernelCreateSema("overlaydemo_semawake", 0, 0, 1, NULL)) < 0) {
        return 1;
    }
#if 0
    SceUID file = sceIoOpen("ms0:/overlaydemo.txt", PSP_O_TRUNC | PSP_O_CREAT | PSP_O_WRONLY, 0777);
    if(file >= 0) sceIoClose(file);
#endif
    previousHandler = sctrlHENSetStartModuleHandler(&overlaydemo_module_loader);
    /* block until user module is properly loaded */
    sceKernelWaitSema(moduleLoadedTrafficLight, 1, NULL);

    /* load own needed libraries */
    init_modules();
    sceDisplayWaitVblankStart();
    while(1) {
        void *frameaddr;
        int bufferwidth = 0, pixelformat;
        if(sceDisplayGetFrameBuf(&frameaddr, &bufferwidth, &pixelformat, PSP_DISPLAY_SETBUF_NEXTFRAME) >= 0 && frameaddr && bufferwidth > 0) {
            break;
        }
        sceKernelDelayThread(1000);
    }

    // load 0x00-0x7F + SJIS range
    libmLoadFont(LIBM_FONT_CG);
    libmLoadFont(LIBM_FONT_SJIS);

    memset(&dinfo, 0, sizeof(dinfo));    memset(&vinfo, 0, sizeof(vinfo));
    dinfo.vinfo = &vinfo;

    // This needs to be called at least once
    libmInitBuffers(0x00000000, 0, &dinfo);

    if (configoption_delayexectime > 0) {
        sceKernelDelayThread(configoption_delayexectime);
        sceKernelDelayThread(configoption_delayexectime);
        sceKernelDelayThread(configoption_delayexectime);
        sceKernelDelayThread(configoption_delayexectime);
    }
    sceDisplaySetFrameBuf_Org = libmHookDisplayHandler(sceDisplaySetFrameBuf_Patched);

    while (	1 ) {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(&pad, 1);
        if ((pad.Buttons & PSP_CTRL_NOTE) != 0) {
            plugin_enabled = 1 - plugin_enabled;
        }
        sceKernelDelayThread(10000);
    }

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
