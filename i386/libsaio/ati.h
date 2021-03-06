//
//  ati.h
//  Chameleon
//
//  Created by Chris Morton on 1/30/13.
//
//

#ifndef Chameleon_ati_h
#define Chameleon_ati_h

#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "device_inject.h"
#include "ati_reg.h"

/* DEFINES */

#define Reg32(reg)		(*(volatile uint32_t *)(card->mmio + reg))
#define RegRead32(reg)		(Reg32(reg))
#define RegWrite32(reg, value)	(Reg32(reg) = value)

#define OFFSET_TO_GET_ATOMBIOS_STRINGS_START 0x6e
#define DATVAL(x)			{kPtr, sizeof(x), (uint8_t *)x}
#define STRVAL(x)			{kStr, sizeof(x), (uint8_t *)x}
#define BYTVAL(x)			{kCst, 1, (uint8_t *)x}
#define WRDVAL(x)			{kCst, 2, (uint8_t *)x}
#define DWRVAL(x)			{kCst, 4, (uint8_t *)x}
#define QWRVAL(x)			{kCst, 8, (uint8_t *)x}
#define NULVAL				{kNul, 0, (uint8_t *)NULL}


/*Typedefs ENUMS*/
typedef enum {
	kNul,
	kStr,
	kPtr,
	kCst
} type_t;

typedef enum {
	CHIP_FAMILY_UNKNOW,
	/* Old */
	CHIP_FAMILY_R420,
	CHIP_FAMILY_R423,
	CHIP_FAMILY_RV410,
	CHIP_FAMILY_RV515,
	CHIP_FAMILY_R520,
	CHIP_FAMILY_RV530,
	CHIP_FAMILY_RV560,
	CHIP_FAMILY_RV570,
	CHIP_FAMILY_R580,
	/* IGP */
	CHIP_FAMILY_RS600,
	CHIP_FAMILY_RS690,
	CHIP_FAMILY_RS740,
	CHIP_FAMILY_RS780,
	CHIP_FAMILY_RS880,
	/* R600 */
	CHIP_FAMILY_R600,
	CHIP_FAMILY_RV610,
	CHIP_FAMILY_RV620,
	CHIP_FAMILY_RV630,
	CHIP_FAMILY_RV635,
	CHIP_FAMILY_RV670,
	/* R700 */
	CHIP_FAMILY_RV710,
	CHIP_FAMILY_RV730,
	CHIP_FAMILY_RV740,
	CHIP_FAMILY_RV770,
	CHIP_FAMILY_RV772,
	CHIP_FAMILY_RV790,
	/* Evergreen */
	CHIP_FAMILY_CEDAR,
	CHIP_FAMILY_CYPRESS,
	CHIP_FAMILY_HEMLOCK,
	CHIP_FAMILY_JUNIPER,
	CHIP_FAMILY_REDWOOD,
	/* Northern Islands */
	CHIP_FAMILY_BARTS,
	CHIP_FAMILY_CAICOS,
	CHIP_FAMILY_CAYMAN,
	CHIP_FAMILY_TURKS,
	/* Southern Islands */
	CHIP_FAMILY_PALM,
	CHIP_FAMILY_SUMO,
	CHIP_FAMILY_SUMO2,
	CHIP_FAMILY_ARUBA,
	CHIP_FAMILY_TAHITI,
	CHIP_FAMILY_PITCAIRN,
	CHIP_FAMILY_VERDE,
	CHIP_FAMILY_OLAND,
	CHIP_FAMILY_HAINAN,
	CHIP_FAMILY_BONAIRE,
	CHIP_FAMILY_KAVERI,
	CHIP_FAMILY_KABINI,
	CHIP_FAMILY_HAWAII,
	/* ... */
	CHIP_FAMILY_MULLINS,
	CHIP_FAMILY_TOPAS,
	CHIP_FAMILY_AMETHYST,
	CHIP_FAMILY_TONGA,
	CHIP_FAMILY_LAST
} ati_chip_family_t;

//card to #ports
typedef struct {
	const char		*name;
	uint8_t			ports;
} card_config_t;

typedef enum {
	kNull,
	/* OLDController */
	kWormy,
	kAlopias,
	kCaretta,
	kKakapo,
	kKipunji,
	kPeregrine,
	kRaven,
	kSphyrna,
	/* AMD2400Controller */
	kIago,
	/* AMD2600Controller */
	kHypoprion,
	kLamna,
	/* AMD3800Controller */
	kMegalodon,
	kTriakis,
	/* AMD4600Controller */
	kFlicker,
	kGliff,
	kShrike,
	/* AMD4800Controller */
	kCardinal,
	kMotmot,
	kQuail,
	/* AMD5000Controller */
	kDouc,
	kLangur,
	kUakari,
	kZonalis,
	kAlouatta,
	kHoolock,
	kVervet,
	kBaboon,
	kEulemur,
	kGalago,
	kColobus,
	kMangabey,
	kNomascus,
	kOrangutan,
	/* AMD6000Controller */
	kPithecia,
	kBulrushes,
	kCattail,
	kHydrilla,
	kDuckweed,
	kFanwort,
	kElodea,
	kKudzu,
	kGibba,
	kLotus,
	kIpomoea,
	kMuskgrass,
	kJuncus,
	kOsmunda,
	kPondweed,
	kSpikerush,
	kTypha,
	/* AMD7000Controller */
	kNamako,
	kAji,
	kBuri,
	kChutoro,
	kDashimaki,
	kEbi,
	kGari,
	kFutomaki,
	kHamachi,
	kOPM,
	kIkura,
	kIkuraS,
	kJunsai,
	kKani,
	kKaniS,
	kDashimakiS,
	kMaguro,
	kMaguroS,
	/* AMD8000Controller */
	kBaladi,
	/* AMD9000Controller */
	kExmoor,
	kBasset,
	kGreyhound,
	kLabrador,
	kCfgEnd
} config_name_t;

//radeon card (includes the AtiConfig)
typedef struct {
	uint16_t		device_id;
	uint32_t		subsys_id;
	ati_chip_family_t	chip_family;
	const char		*model_name;
	config_name_t		cfg_name;
} radeon_card_info_t;

typedef struct {
	struct DevPropDevice	*device;
	radeon_card_info_t	*info;
	pci_dt_t		*pci_dev;
	uint8_t			*fb;
	uint8_t			*mmio;
	uint8_t			*io;
	uint8_t			*rom;
	uint64_t		rom_size;
	uint64_t		vram_size;
	const char		*cfg_name;
	uint8_t			ports;
	uint32_t		flags;
	bool			posted;
} card_t;


/* Flags */
#define MKFLAG(n)	(1 << n)
#define FLAGTRUE	MKFLAG(0)
#define EVERGREEN	MKFLAG(1)
#define FLAGMOBILE	MKFLAG(2)
#define FLAGOLD		MKFLAG(3)
#define FLAGNOTFAKE	MKFLAG(4)

/* Typedefs STRUCTS */
typedef struct {
	type_t			type;
	uint32_t		size;
	uint8_t			*data;
} value_t;

// dev_tree  representation
typedef struct {
	uint32_t	flags;
	bool		all_ports;
	char		*name;
	bool		(*get_value)(value_t *val);
	value_t		default_val;
} AtiDevProp;

/* functions */
bool get_bootdisplay_val(value_t *val);
bool get_vrammemory_val(value_t *val);
bool get_name_val(value_t *val);
bool get_nameparent_val(value_t *val);
bool get_model_val(value_t *val);
bool get_conntype_val(value_t *val);
bool get_vrammemsize_val(value_t *val);
bool get_binimage_val(value_t *val);
bool get_romrevision_val(value_t *val);
bool get_deviceid_val(value_t *val);
bool get_mclk_val(value_t *val);
bool get_sclk_val(value_t *val);
bool get_refclk_val(value_t *val);
bool get_platforminfo_val(value_t *val);
bool get_vramtotalsize_val(value_t *val);
bool get_hdmiaudio(value_t * val);

#endif
