#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>
#include <unistd.h>

#define VKAPI_PTR

typedef enum VkResult
{
    VK_SUCCESS = 0
} VkResult;

// Provided by VK_VERSION_1_0
typedef void(VKAPI_PTR *PFN_vkVoidFunction)(void);

// Provided by VK_VERSION_1_0
#define VK_DEFINE_HANDLE(object) typedef struct object##_T *object;

// Provided by VK_VERSION_1_0
VK_DEFINE_HANDLE(VkInstance)

// Provided by VK_VERSION_1_0
typedef PFN_vkVoidFunction (*vkGetInstanceProcAddr_PFN)(
    VkInstance instance, const char *pName);

// Provided by VK_VERSION_1_0
#define VK_MAX_EXTENSION_NAME_SIZE 256U
typedef struct VkExtensionProperties
{
    char extensionName[VK_MAX_EXTENSION_NAME_SIZE];
    uint32_t specVersion;
} VkExtensionProperties;

typedef VkResult (*vkEnumerateInstanceExtensionProperties_PFN)(
    const char *pLayerName,
    uint32_t *pPropertyCount,
    VkExtensionProperties *pProperties);

typedef struct
{
    void *handle;
} VkLibrary;

void vklib_free(VkLibrary *lib)
{
    if (lib->handle != NULL) {
        dlclose(lib->handle);
        lib->handle = NULL;
    }

    memset(lib, 0, sizeof(VkLibrary));
}

bool vklib_init(VkLibrary *lib, const char *so_file_name)
{
    memset(lib, 0, sizeof(VkLibrary));

    void *handle = dlopen(so_file_name, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
        const char *err = dlerror();
        printf("Load \"%s\" error: (%s)\n", so_file_name, err);
        vklib_free(lib);
        return false;
    }
    lib->handle = handle;
    return true;
}

PFN_vkVoidFunction vklib_proc(VkLibrary *lib, const char *fn_name)
{
    PFN_vkVoidFunction fn = dlsym(lib->handle, fn_name);
    const char *status = dlerror();
    if (status != NULL) {
        printf("load %s error: %s\n", fn_name, status);
        return NULL;
    }
    return fn;
}

bool do_load_iteration(const char *lib_path)
{
    VkLibrary icd = {0};
    if (!vklib_init(&icd, lib_path)) {
        vklib_free(&icd);
        return false;
    }

    typedef vkEnumerateInstanceExtensionProperties_PFN GetPropsPFN;
    GetPropsPFN vkEnumerateInstanceExtensionProperties =
        (GetPropsPFN)vklib_proc(&icd, "vkEnumerateInstanceExtensionProperties");
    if (vkEnumerateInstanceExtensionProperties == NULL) {
        printf("Unable to load %s\n", "vkEnumerateInstanceExtensionProperties");
        vklib_free(&icd);
        return false;
    }

    uint32_t nb_exts = 0;
    VkResult res = vkEnumerateInstanceExtensionProperties(NULL, &nb_exts, NULL);
    if (res != VK_SUCCESS) {
        printf(
            "sizecall for vkEnumerateInstanceExtensionProperties failed"
            "with %d\n",
            res);
        vklib_free(&icd);
        return false;
    }

    vklib_free(&icd);
    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("argv[1] should contain path to vulkan library\n");
        return EXIT_FAILURE;
    }

    uint32_t nb_iterations = 1;
    if (argc >= 3) {
        int nb_iteration_arg = atoi(argv[2]);
        if (nb_iterations > 0) {
            nb_iterations = nb_iteration_arg;
        }
    } else {
        printf("NOTE: arg[3] can be set to number of iteration\n");
    }

    printf(
        "Running test %d %s\n",
        nb_iterations,
        nb_iterations > 1 ? "times" : "time");

    for (size_t iteration = 0; iteration != nb_iterations; ++iteration) {
        printf("Iteration %" PRIu64 "\n", iteration + 1);
        bool status = do_load_iteration(argv[1]);
        if (!status) {
            printf("Failed iteration\n");
            break;
        }
    }

    return EXIT_SUCCESS;
}
