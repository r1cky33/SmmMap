#include <Library/PeCoffLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmCpu.h>

// Includes for File interaction
#include <Protocol/SimpleFileSystem.h>

#include "logging/logging.h"
#include "mmap/mmap.h"
#include "payload.h"

static EFI_SMM_BASE2_PROTOCOL *SmmBase2;
static EFI_SMM_SYSTEM_TABLE2 *GSmst2;
static EFI_SMM_CPU_PROTOCOL *Cpu = NULL;

EFI_HANDLE handle;
BOOLEAN mapped = FALSE; 

typedef EFI_STATUS (EFIAPI *UEFI_IMAGE_ENTRY)(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
);


EFI_STATUS EFIAPI SmiHandler(EFI_HANDLE dispatch, CONST VOID *context, VOID *buffer, UINTN *size) {

    if (mapped) {
        return EFI_SUCCESS;
    }

    GSmst2->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL, (VOID **)&Cpu);

    LOG_INFO("[INFO] Trying to map payload \r\n");

    EFI_STATUS Status;
    VOID *EntryPoint = NULL;

    Status = LoadAndRelocateSmmImage(GSmst2, (char*)payload, payload_size, &EntryPoint);

    if (EFI_ERROR(Status)) {

        LOG_ERROR("[ERR] Failed to load embedded SMM image: %r\n", Status);
        return EFI_OUT_OF_RESOURCES;
    }

    LOG_INFO("[INFO] Successfully mapped image... calling entrypoint: 0x%x \r\n", EntryPoint);

    UEFI_IMAGE_ENTRY EntryFunc = (UEFI_IMAGE_ENTRY)(UINTN)EntryPoint;
    Status = EntryFunc(NULL, gST);

    if (!EFI_ERROR(Status)) {
        mapped = TRUE;
        GSmst2->SmiHandlerUnRegister(handle);
    }

    LOG_INFO("[INFO] Image Entry returned: %r\n", Status);

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE image, IN EFI_SYSTEM_TABLE *table) {

    LOG_INFO("[HELLO] SmmMap Initializing... \r\n");

    gRT = table->RuntimeServices;
    gBS = table->BootServices;
    gST = table;

    if (EFI_ERROR(gBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, 0,
                                      (void **)&SmmBase2))) {
        LOG_ERROR("[ERR] Couldn't localte SmmBase2Protocol \r\n");
        return EFI_SUCCESS;
    }

    if (EFI_ERROR(SmmBase2->GetSmstLocation(SmmBase2, &GSmst2))) {
        LOG_ERROR("[ERR] Failed to find SMST \r\n");
        return EFI_SUCCESS;
    }

    GSmst2->SmiHandlerRegister(&SmiHandler, NULL, &handle);

    LOG_INFO("[INFO] Handler registered \r\n");

    return EFI_SUCCESS;
}
