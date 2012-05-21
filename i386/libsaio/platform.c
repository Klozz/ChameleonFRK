/*
 *  platform_env.c
 * 
 * Copyright 2012 Cadet-petit Armel <armelcadetpetit@gmail.com>. All rights reserved.
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "cpu.h"

#ifndef DEBUG_PLATFORM
#define DEBUG_PLATFORM 0
#endif

#if DEBUG_PLATFORM
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

#ifndef DEBUG_PLATFORM_CPU
#define DEBUG_PLATFORM_CPU 0
#endif

static char *gboardproduct = NULL;
static char *gPlatformName = NULL;
static char *gRootDevice = NULL;

void SetgRootDevice(const char * str)
{
    gRootDevice = (char*)str;
}
void Setgboardproduct(const char * str)
{
    gboardproduct = (char*)str;
}
void SetgPlatformName(const char * str)
{
    gPlatformName = (char*)str;
}

char * GetgPlatformName(void)
{
    return gPlatformName ;
}
char * Getgboardproduct(void)
{
    return gboardproduct;
}
char * GetgRootDevice(void)
{
    return gRootDevice;
}

#ifdef rootpath
static char gRootPath[ROOT_PATH_LEN];
void SetgRootPath(const char * str)
{
    bzero(gRootPath,sizeof(gRootPath));
    memcpy(gRootPath,str, sizeof(gRootPath));
}
char * GetgRootPath(void)
{
    return gRootPath ;
}
#endif

typedef enum envtype {
    kEnvPtr = 0,
    kEnvValue = 1
} envtype;

struct env_struct {
    unsigned long long value;                    
    char *name; 
    void * ptr;
    //int lock;
    enum envtype Type;
    UT_hash_handle hh;         /* makes this structure hashable */
};

static int CopyVarPtr (struct env_struct *var, void* ptr, size_t size);
static struct env_struct *find_env(const char *name);
static void _re_set_env_copy(struct env_struct *var , void* ptr,size_t size);
struct env_struct *platform_env = NULL;


static int CopyVarPtr (struct env_struct *var, void* ptr, size_t size)
{
    var->ptr = malloc(size);
    if (!var->ptr) {
        return 0;
    }
    memcpy(var->ptr, ptr, size);
    return 1;
}

static struct env_struct *find_env(const char *name) {
    struct env_struct *var;
    
	if (setjmp(uterror) == -1) {
#if DEBUG_PLATFORM
		printf("find_env: Unable to find environement variable\n"); 
        getc();
#endif
		return NULL;
	} else {
		HASH_FIND_STR( platform_env, name, var );  
	}
    return var;
}

static void _re_set_env_copy(struct env_struct *var , void* ptr,size_t size) {
    
	if (var->Type == kEnvPtr) {
		return ;
	} 
    
    if (var->ptr) {
        free(var->ptr);
        var->ptr = NULL;
    }
    
    CopyVarPtr(var, ptr,  size);
	
	return;    
}

void re_set_env_copy(const char *name , void* ptr,size_t size) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var|| (var->Type == kEnvPtr)) {
		printf("re_set_env_copy: Unable to find environement variable %s\n",name);
#if DEBUG_PLATFORM
        getc();
#endif
		return ;
	} 
    
    _re_set_env_copy(var , ptr, size);
	
	return;    
}

static void _set_env(const char *name, unsigned long long value,  enum envtype Type, void* ptr, size_t size ) {
    struct env_struct *var;
    
    var = (struct env_struct*)malloc(sizeof(struct env_struct));
    if (!var) {
        return;
    }
    if (Type == kEnvPtr) {
        if (!CopyVarPtr( var,  ptr, size)) return;
    } 
    else if (Type == kEnvValue) 
        var->value = value;
    else
        return;
    
    var->Type = Type;    
    
    var->name = newString(name);
    if (!var->name) {        
        free(var);
        return;
    }
	
	if (setjmp(uterror) == -1) {
		printf("_set_env: Unable to set environement variable"); // don't try to acces to the string 'name', 
		//'cause we just returned from the longjump, stack as already changed state.
#if DEBUG_PLATFORM
		getc();
#endif
		return;
	} else {
        HASH_ADD_KEYPTR( hh, platform_env, name, strlen(var->name), var );
	}
}

/* Warning: set_env will create a new variable each time it will be called, 
 * if you want to set again an existing variable, please use safe_set_env or re_set_env .
 * NOTE: If you set several times the "same variable" by using this function,
 * the HASH_COUNT will grow up, 
 * but hopefully find_env will return the last variable that you have set with the same name
 * ex: set_env("test",10);
 *     set_env("test",20);
 *
 *    HASH_COUNT will be equal to 2, get_env("test") will return 20
 *
 *    safe_set_env("test",10);
 *    safe_set_env("test",20);
 * 
 *    HASH_COUNT will be equal to 1, get_env("test") will return 20
 *
 *    set_env("test",10);
 *    re_set_env("test",20);
 *
 *    HASH_COUNT will be equal to 1, get_env("test") will return 20
 *
 */
void set_env(const char *name, unsigned long long value ) {
    _set_env(name, value, kEnvValue,0,0);
}

void set_env_copy(const char *name, void * ptr, size_t size ) {
    _set_env(name, 0, kEnvPtr,ptr,size);
}

unsigned long long get_env_var(const char *name) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var) {
#if DEBUG_PLATFORM
		printf("get_env_var: Unable to find environement variable %s\n",name);
        getc();
#endif
		return 0;
	}
    
    if (var->Type != kEnvValue) {
        printf("get_env_var: Variable %s is not a value\n",name);
#if DEBUG_PLATFORM
        getc();
#endif
        return 0;
    }
	
	return var->value;
    
}

unsigned long long get_env(const char *name) {	
	
	return get_env_var(name);;
    
}

void * get_env_ptr(const char *name) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var) {
#if DEBUG_PLATFORM
		printf("get_env_ptr: Unable to get environement ptr variable %s\n",name);
        getc();
#endif
		return 0;
	}
    
    if (var->Type != kEnvPtr) {
        printf("get_env_ptr: Variable %s is not a ptr\n",name);
#if DEBUG_PLATFORM
        getc();
#endif
        return 0;
    }
	
	return var->ptr;
    
}

/* If no specified variable exist, safe_set_env will create one, else it only modify the value */
static void _safe_set_env(const char *name, unsigned long long value,  enum envtype Type, void* ptr, size_t size )
{
	struct env_struct *var;
    
	var = find_env(name);
    
    if (!var) {
        if (Type == kEnvPtr) {
            _set_env(name, 0, kEnvPtr,ptr,size);
        } 
        else if (Type == kEnvValue) {
            _set_env(name, value, kEnvValue,0,0);
        }
    } 
    else if (var->Type != Type) {
        return;        
	}     
    else {
        if (Type == kEnvValue) 
            var->value = value;
        else if (Type == kEnvPtr)
            _re_set_env_copy(var,ptr,size);            
    }
	
	return;    
}

void safe_set_env_copy(const char *name , void * ptr, size_t size ) {
    
    _safe_set_env(name, 0, kEnvPtr,ptr,size);
	
	return;    
}

void safe_set_env(const char *name , unsigned long long value) {
    
    _safe_set_env(name, value, kEnvValue,0,0);
	
	return;    
}

void re_set_env(const char *name , unsigned long long value) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var || (var->Type == kEnvValue)) {
		printf("re_set_env: Unable to reset environement value variable %s\n",name);
#if DEBUG_PLATFORM
        getc();
#endif
		return ;
	} 
    
    var->value = value;
	
	return;    
}

static void delete_env(struct env_struct *var) {
	
	if (setjmp(uterror) == -1) {
		printf("delete_env: Unable to delete environement variable\n");
#if DEBUG_PLATFORM
        getc();	
#endif
		return;
	} else {
		HASH_DEL( platform_env, var);
        if (var->name) free(var->name);            
        free(var);
        
	}
}

void unset_env(const char *name) {
    struct env_struct *var;
    
    if ((var = find_env(name))) {
        delete_env(var);
    }      
}

void free_platform_env(void) {
    struct env_struct *current_var, *tmp; 
    
	if (setjmp(uterror) == -1) {
		printf("free_platform_env: Unable to delete all environement variables\n"); 
#if DEBUG_PLATFORM
		getc();
#endif
		return;
	} else {
		HASH_ITER(hh, platform_env, current_var, tmp) {    
			HASH_DEL(platform_env,current_var);
            if (current_var->name) free(current_var->name);
			free(current_var);           
		}
	}     
}

#if DEBUG_PLATFORM
void debug_platform_env(void)
{
    struct env_struct *current_var;
    for(current_var=platform_env;current_var;current_var=(struct env_struct*)(current_var->hh.next)) 
    {
#if DEBUG_PLATFORM  >= 2
        if (current_var->Type == kEnvValue)
            printf(" Name = %s | Type = VALUE | Value = 0x%04x\n",current_var->name,(uint32_t)current_var->value);
        else if (current_var->Type == kEnvPtr )
            printf(" Name = %s | Type = PTR(Copy) | Value = 0x%x\n",current_var->name,(uint32_t)current_var->ptr);
#else
        
        if (current_var->Type == kEnvValue)
            printf("%s: 0x%04x\n",current_var->name,(uint32_t)current_var->value);
        else if (current_var->Type == kEnvPtr )
            printf("%s(ptr): 0x%x\n",current_var->name,(uint32_t)current_var->ptr);
#endif
        
    }
    getc();
}
#endif

void showError(void)
{
    struct env_struct *current_var;
    for(current_var=platform_env;current_var;current_var=(struct env_struct*)(current_var->hh.next)) 
    {
		if (strcmp(current_var->name, envConsoleErr) == 0) {
			if (current_var->Type == kEnvPtr) {
				printf("stderr: %s \n",(char*)(uint32_t)current_var->ptr);
			}
		}        
    }
}

/** 
 Scan platform hardware information, called by the main entry point (common_boot() ) 
 _before_ bootConfig xml parsing settings are loaded
 */
void scan_platform(void)
{	
	build_pci_dt();
	scan_cpu();
    
#if DEBUG_PLATFORM_CPU
    printf("CPU: %s\n", (char*)get_env_ptr(envBrandString));
    if (get_env(envVendor) == CPUID_VENDOR_AMD)
        printf("CPU: Vendor/Model/ExtModel: 0x%x/0x%x/0x%x\n", (uint32_t)get_env(envVendor), (uint32_t)get_env(envModel), (uint32_t)get_env(envExtModel));
	printf("CPU: Family/ExtFamily:      0x%x/0x%x\n", (uint32_t)get_env(envFamily), (uint32_t)get_env(envExtFamily));
    if (get_env(envVendor) == CPUID_VENDOR_AMD) {
        printf("CPU (AMD): TSCFreq:               %dMHz\n", (uint32_t)(get_env(envTSCFreq) / 1000000));
        printf("CPU (AMD): FSBFreq:               %dMHz\n", (uint32_t)(get_env(envFSBFreq) / 1000000));
        printf("CPU (AMD): CPUFreq:               %dMHz\n", (uint32_t)(get_env(envCPUFreq) / 1000000));
        printf("CPU (AMD): MaxCoef/CurrCoef:      0x%x/0x%x\n", (uint32_t)get_env(envMaxCoef), (uint32_t)get_env(envCurrCoef));
        printf("CPU (AMD): MaxDiv/CurrDiv:        0x%x/0x%x\n", (uint32_t)get_env(envMaxDiv), (uint32_t)get_env(envCurrDiv));
    } else if (get_env(envVendor) == CPUID_VENDOR_INTEL){
        printf("CPU: TSCFreq:               %dMHz\n", (uint32_t)(get_env(envTSCFreq) / 1000000));
        printf("CPU: FSBFreq:               %dMHz\n", (uint32_t)(get_env(envFSBFreq) / 1000000));
        printf("CPU: CPUFreq:               %dMHz\n", (uint32_t)(get_env(envCPUFreq) / 1000000));
        printf("CPU: MaxCoef/CurrCoef:      0x%x/0x%x\n", (uint32_t)(get_env(envMaxCoef)), (uint32_t)(get_env(envCurrCoef)));
        printf("CPU: MaxDiv/CurrDiv:        0x%x/0x%x\n", (uint32_t)(get_env(envMaxDiv)), (uint32_t)(get_env(envCurrDiv)));
    }			
	
	printf("CPU: NoCores/NoThreads:     %d/%d\n", (uint32_t)(get_env(envNoCores)), (uint32_t)(get_env(envNoThreads)));
	printf("CPU: Features:              0x%08x\n", (uint32_t)(get_env(envFeatures)));
    printf("CPU: ExtFeatures:           0x%08x\n", (uint32_t)(get_env(envExtFeatures)));
#ifndef AMD_SUPPORT
    printf("CPU: MicrocodeVersion:      %d\n", (uint32_t)(get_env(envMicrocodeVersion)));
#endif
    pause();
#endif
}