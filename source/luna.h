#define LUNA_DIR							"/config/luna/"
#define LUNA_DUMP_DIR						LUNA_DIR "dump/"
#define LUNA_TEMPLATE_DIR					LUNA_DIR "enctemplate/"

#define TID									0x01006F8002326000

#define MAJOR_VERSION						0
#define MINOR_VERSION						6
#define REVISION_VERSION					14
#define STRINGIFY(x)						#x
#define TOSTRING(x)							STRINGIFY(x)
#define STRING_VERSION						"" TOSTRING(MAJOR_VERSION) "." TOSTRING(MINOR_VERSION) "." TOSTRING(REVISION_VERSION) ""

//print extra debug log
#define DEBUG								0
//disable game and template checks
#define DEBUG_UI							0
//draw debug statistics
#define DEBUG_OV							0
//print debugging information to console (nxlink)
#define DEBUG_PRINTF						0