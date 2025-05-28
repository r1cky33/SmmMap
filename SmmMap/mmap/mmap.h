#pragma once

#include <Pi/PiSmmCis.h>

EFI_STATUS LoadAndRelocateSmmImage(IN EFI_SMM_SYSTEM_TABLE2 *GSmst2, IN VOID *ImageBuffer, IN UINTN ImageSize, OUT VOID **EntryPoint);
