#ifndef CONSTANTS_H // Include guarding
#define CONSTANTS_H
// Allowed commands
#define N_COMMANDS 4 // Number of allowed commands
#define COMMAND_MAX 100 // Maximum length of a command
const char* COMMANDS[N_COMMANDS] = { // Array of command strings
"config", 
"init", 
"add",
"reset",

};
// User info
#define USERNAME_MAX 30
#define EMAIL_MAX 40


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
INVALID_OPTION[] = "Invalid option\n",
INVALID_COMMAND_FOR_ALIAS[] = "Invalid command used for alias\n",
ALREADY_IN_REPOSITORY[] = "There is already a repository in current directory or parent directories\n",
NOT_REPO[] = "Not a lit repository\n",
// messages
REPO_INIT_MESSAGE[] = "Initialized empty Git repository in %s\n",
FILEN_NOT_FOUND[] = "Could not locate file %s\n",
STAGE_SUCCESS[] = "File(s) staged successfully\n",
FILE_IS_STAGED_COLOR[] = "\033[0;32mStaged\033[0m\n",
FILE_ISNT_STAGED_COLOR[] = "\033[0;33mNot staged\033[0m\n",
FILE_IS_UNTRACKED_COLOR[] = "\033[0;31mUntracked\033[0m\n",
RESET_SUCCESS[] = "File(s) unstaged successfully\n"
;

// Paths
#define PATH_MAX 280
#define LITDIR_NAME ".lit"
#define CONFIG_PATH "C:\\Users\\Asus\\Desktop\\CE\\Mabani\\Project"
#define CONFIG_MAX 200

// Commits
#define COMMIT_MESSAGE_MAX 72
#define MAX_TRACKED_FILES 200
#define ROOT_ID 0x00
#define STAGE_ID 0x01


// Branch
#define BRANCH_NAME_MAX 40

// File
#define MAX_LINE_LENGTH 300

// Usage text
const char USAGE[] = "usage: lit <command> [<args>]\n";
#endif // CONSTANTS_H