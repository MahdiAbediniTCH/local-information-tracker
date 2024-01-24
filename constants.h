// Allowed commands
#define N_COMMANDS 2 // Number of allowed commands
#define COMMAND_MAX 60 // Maximum length of a command
const char* COMMANDS[N_COMMANDS] = { // Array of command strings
"config", 
"init", 
};

// Alias
#define MAX_ALIASES 40

// Command handling related strings
const char 
// options
GLOBAL_OPTION[] = "-global", 
// errors
FEW_ARGUMENTS[] = "Too few arguments\n",
NO_REPO[] = "Not a repository\n",
INVALID_CONFIG[] = "Invalid configuration\n",
INVALID_USAGE[] = "Invalid usage\n",
INVALID_COMMAND_FOR_ALIAS[] = "Invalid command used for alias\n"
;

// Paths
#define PATH_MAX 260
#define LITDIR_NAME ".lit"
#define CONFIG_PATH "C:\\Users\\Asus\\Desktop\\CE\\Mabani\\Project"
#define CONFIG_MAX 200

