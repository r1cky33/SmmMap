#include "mmap.h"
#include "../logging/logging.h"
#include "../memory/memory.h"

#include <Library/PeCoffLib.h>

EFI_STATUS LoadAndRelocateSmmImage(IN EFI_SMM_SYSTEM_TABLE2 *GSmst2, IN VOID *ImageBuffer, IN UINTN ImageSize, OUT VOID **EntryPoint) {
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
