#include "logging.h"
#include "memory.h"
#include "payload.h"

#include <Uefi/UefiSpec.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <PiDxe.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SmmCpu.h>
#include <Uefi.h>
#include <Protocol/SmmBase2.h>
#include <Library/PeCoffLib.h>

static EFI_SMM_BASE2_PROTOCOL *SmmBase2;
static EFI_SMM_SYSTEM_TABLE2 *GSmst2;
static EFI_SMM_CPU_PROTOCOL *Cpu = NULL;

typedef EFI_STATUS (EFIAPI *UEFI_IMAGE_ENTRY)(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
);

EFI_STATUS
LoadAndRelocateSmmImage(IN VOID *ImageBuffer, IN UINTN ImageSize,
                        OUT VOID **EntryPoint) {
    EFI_STATUS Status;
    PE_COFF_LOADER_IMAGE_CONTEXT ImageContext;
    EFI_PHYSICAL_ADDRESS LoadedImage = 0;

    ZMemSet(&ImageContext, 0, sizeof(ImageContext));
    ImageContext.Handle = ImageBuffer;
    ImageContext.ImageRead = PeCoffLoaderImageReadFromMemory;

    LOG_INFO("[INFO] Get Image information \r\n");
 
    //
    // Get image info
    //
    Status = PeCoffLoaderGetImageInfo(&ImageContext);
    if (EFI_ERROR(Status)) {
        LOG_ERROR("[ERR] PeCoffLoaderGetImageInfo failed: %d \r\n", Status);
        return Status;
    }

    LOG_INFO("[INFO] Allocating Memory for manual mapping image \r\n");
    
    //
    // Allocate memory for the image in SMM (must be executable memory)
    //
    GSmst2->SmmAllocatePages(AllocateAnyPages, EfiRuntimeServicesData,
                             EFI_SIZE_TO_PAGES(ImageContext.ImageSize),
                             &LoadedImage);
    if (LoadedImage == 0) {
        LOG_ERROR("[ERR] Out of resources \r\n");
        return EFI_OUT_OF_RESOURCES;
    }

    LOG_INFO("[INFO] Image Entry returned: %d \r\n", Status);

    //
    // Load the image to memory
    //
    ImageContext.ImageAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)LoadedImage;
    ImageContext.DestinationAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)LoadedImage;

    Status = PeCoffLoaderLoadImage(&ImageContext);
    if (EFI_ERROR(Status)) {
        LOG_ERROR("[ERR] PeCoffLoaderLoadImage failed: %d \r\n", Status);
        GSmst2->SmmFreePages(LoadedImage,
                             EFI_SIZE_TO_PAGES(ImageContext.ImageSize));
        return Status;
    }

    LOG_INFO("[INFO] Grabbing relocation info \r\n");
 
    //
    // Manually get RelocationData (I don't know why the routine shouln't just grab it itself)
    //
    UINT8 *ImageBasePtr = (UINT8 *)(UINTN)ImageContext.ImageAddress;
    EFI_IMAGE_DOS_HEADER *DosHdr = (EFI_IMAGE_DOS_HEADER *)ImageBasePtr;
    EFI_IMAGE_NT_HEADERS64 *NtHdr =
        (EFI_IMAGE_NT_HEADERS64 *)(ImageBasePtr + DosHdr->e_lfanew);

    EFI_IMAGE_DATA_DIRECTORY relocDir =
        NtHdr->OptionalHeader
            .DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC];

    VOID *RelocationData = NULL;
    if (relocDir.Size != 0) {
        RelocationData = ImageBasePtr + relocDir.VirtualAddress;
    }

    LOG_INFO("[INFO] Calling RelocateForRuntime \r\n");
   
    //
    // Relocate for runtime/SMM
    //
    PeCoffLoaderRelocateImageForRuntime(
        ImageContext.ImageAddress, ImageContext.ImageAddress,
        ImageContext.ImageSize, RelocationData);
    if (EFI_ERROR(Status)) {
        LOG_ERROR("[ERR] PeCoffLoaderRelocateImageForRuntime failed %d \r\n",
                  Status);
        GSmst2->SmmFreePages(LoadedImage,
                             EFI_SIZE_TO_PAGES(ImageContext.ImageSize));
        return Status;
    }

    LOG_INFO("[INFO] Getting entrypoint \r\n");

    //
    // Return entry point
    //
    if (EntryPoint != NULL) {
        *EntryPoint = (VOID *)(UINTN)ImageContext.EntryPoint;
    }

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI SmiHandler(EFI_HANDLE dispatch, CONST VOID *context, VOID *buffer, UINTN *size) {

    GSmst2->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL, (VOID **)&Cpu);

    LOG_INFO("[INFO] Trying to map payload \r\n");

    EFI_STATUS Status;
    VOID *EntryPoint = NULL;

    Status = LoadAndRelocateSmmImage(payload, payload_size, &EntryPoint);

    if (EFI_ERROR(Status)) {

        LOG_ERROR("[ERR] Failed to load embedded SMM image: %r\n", Status);
        return EFI_OUT_OF_RESOURCES;
    }

    LOG_INFO("[INFO] Successfully mapped image... calling entrypoint: 0x%x \r\n", EntryPoint);

    UEFI_IMAGE_ENTRY EntryFunc = (UEFI_IMAGE_ENTRY)(UINTN)EntryPoint;
    Status = EntryFunc(NULL, gST);

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

    EFI_HANDLE handle;
    GSmst2->SmiHandlerRegister(&SmiHandler, NULL, &handle);

    LOG_INFO("[INFO] Handler registered \r\n");

    return EFI_SUCCESS;
}
