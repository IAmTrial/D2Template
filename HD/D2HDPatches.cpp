#include "D2HDPatches.h"
#include "../DLLmain.h"
#include "../D2Ptrs.h"

void __stdcall ResizeWindow(int mode, int* width, int* height);
int __stdcall GetNewResolutionId();
int __stdcall GetNewResolutionOnGameStart();
int __stdcall SetupGlideRenderResolution();

void __declspec(naked) HD::ResizeWindow_Interception() {
    __asm {
        PUSH [ESP + 0x10]
        PUSH [ESP + 0x10]
        PUSH [ESP + 0x10]
        CALL [ResizeWindow]
        RET
    }
}

void __stdcall ResizeWindow(int mode, int* width, int* height) {
    switch (mode) {
    case 0:
        *width = 640;
        *height = 480;
        break;

    case 1:
        *width = 800;
        *height = 600;
        break;

    case 2:
        *width = 800;
        *height = 600;
        break;

    case 3:
        *width = 1344;
        *height = 700;
        break;

    case 4:
        *width = 1068;
        *height = 600;
        break;

    default:
        *width = 640;
        *height = 480;
        break;
    }
}

void __declspec(naked) HD::ResizeRenderResolution_Interception() {
    __asm {
        SUB ESP, 0x8
        LEA ESI, [ESP+0x4]
        LEA EDX, [ESP]
        PUSH ESI
        PUSH EDX
        PUSH EAX
        CALL[ResizeWindow]
        MOV ESI, [ESP]
        MOV EDX, [ESP+0x4]
        ADD ESP, 0x8
        RET
    }
}

void HD::ResizeForgroundRenderWidth_Interception() {
    int mode, temp;
    __asm MOV mode, ESI

    ResizeWindow(mode, D2GDI_ForegroundRenderWidth, &temp);

    __asm MOV EAX, 0x1
}

void HD::ResizeGameLogicResolution_Interception() {
    __asm PUSHAD

    int mode;
    __asm MOV mode, ESI

    ResizeWindow(mode, D2CLIENT_ScreenSizeX, D2CLIENT_ScreenSizeY);
    *D2CLIENT_InventoryArrangeMode = (mode == 2) ? 1 : 0;

    __asm POPAD
}

void __declspec(naked) HD::SetResolutionModeId_Interception() {
    __asm {
        CALL[GetNewResolutionId]
        MOV ESI, EAX
        RET
    }
}

void __declspec(naked) HD::SetResolutionModeOnGameStart_Interception() {
    __asm {
        PUSH EAX
        PUSH ECX
        CALL[GetNewResolutionOnGameStart]
        MOV ESI, EAX
        POP ECX
        POP EAX
        RET
    }
}

int __stdcall GetNewResolutionId() {
    int mode = D2GFX_GetResolutionMode();

    if (mode >= NUMBER_OF_CUSTOM_RESOLUTIONS) {
        mode = 0;
    } else if (mode == 0) {
        mode = 2;
    } else {
        mode++;
    }

    return mode;
}

int __stdcall GetNewResolutionOnGameStart() {
    int mode = *D2CLIENT_CurrentRegistryResolutionMode;

    if (mode == 1) {
        return 2;
    } else {
        return mode;
    }
}

void __declspec(naked) HD::SetRegistryResolutionModeId_Interception() {
    __asm {
        PUSHAD

        PUSH ECX
        CALL [GetNewResolutionId]
        POP ECX
        MOV [ECX + 0x124], EAX

        POPAD
        RET
    }
}

void __declspec(naked) HD::Replace640_ResizeD2DWindow_Interception() {
    __asm {
        mov ecx, D2DDRAW_GameWindowSizeY
            mov dword ptr ds : [ecx], RESOLUTION_640_TO_HD_HEIGHT;
        mov ecx, RESOLUTION_640_TO_HD_WIDTH;
        ret
    }
}

void __declspec(naked) HD::Replace640_ResizeD2D_Interception1() {
    __asm {
        mov edi, RESOLUTION_640_TO_HD_WIDTH
            mov esi, RESOLUTION_640_TO_HD_HEIGHT
            ret
    }
}

void __declspec(naked) HD::SetupGlideRenderResolution_Interception() {
    __asm {
        PUSH EBX
        PUSH EDI
        CALL [SetupGlideRenderResolution]
        MOV ECX, EAX
        POP EDI
        POP EBX
        MOV ESI, 0
        MOV EDX, 0; Set EDX to 0
        RET
    }
}

int __stdcall SetupGlideRenderResolution() {
    int newResolutionMode, glideVideoMode;
    __asm MOV newResolutionMode, ESI

    ResizeWindow(newResolutionMode, D2GLIDE_ScreenSizeX, D2GLIDE_ScreenSizeY);

    switch (newResolutionMode) {
    case 0:
        glideVideoMode = 7;
        break;

    case 1:
        glideVideoMode = 8;
        break;

    default:
        glideVideoMode = (glideVideoMode - 2) + 8;
        break;
    }

    return glideVideoMode;
}

void __stdcall HD::SetupGlideWindowSize() {
    __asm PUSHAD

    int newGlideVideoMode;
    __asm MOV newGlideVideoMode, EAX

    int resolutionMode = (newGlideVideoMode == 7) ? 0 : ((newGlideVideoMode - 8) + 2);
    ResizeWindow(resolutionMode, *GLIDE3X_GameWindowSizeX, *GLIDE3X_GameWindowSizeY);

    __asm POPAD
}

// Repositions panels in the correct location, independent of resolution.
void __stdcall HD::RepositionPanels() {
    *D2CLIENT_PanelOffsetX = (*D2CLIENT_ScreenSizeX / 2) - 320;
    *D2CLIENT_PanelOffsetY = (*D2CLIENT_ScreenSizeY - 480) / -2;
}

// This function is used to prevent running unwanted 640 code.
int __stdcall HD::GetResolutionMode_Patch() {
    return (*D2CLIENT_ScreenSizeX >= 800) ? 2 : 0;
}
