#ifndef CONSTANTS_H // Include guarding
#define CONSTANTS_H
// Allowed commands
#define N_COMMANDS 17 // Number of allowed commands
#define COMMAND_MAX 100 // Maximum length of a command
const char* COMMANDS[N_COMMANDS] = { // Array of command strings
"config", 
"init", 
"add",
"reset",
"status", 
"commit",
"set",
"replace",
"remove",
"log",
"branch",
"checkout",
"revert",
"merge",
"diff",
"tag",
"grep"
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
NOT_REPO[] = "Not a Lit repository\n",
MSG_TOO_LONG[] = "Message is too long\n",
EXPECTED_MSG[] = "Expected a commit message\n",
EXPECTED_SHORTCUT[] = "Expected a commit message shortcut\n",
NOTHING_TO_COMMIT[] = "There is nothing to be committed (try lit add <file>)\n",
OPTION_REQUIRED[] = "An option is required for this command\n",
INVALID_TIME_FORMAT[] = "Invalid time format. Try using YYYY/MM/DD HH:mm\n",
NO_MESSAGES_FOUND[] = "No commit messages were found matching the given word\n",
BRANCH_EXISTS[] = "Branch already exists\n",
CHECKOUT_FAILURE[] = "Could not locate identifier\n",
CHECKOUT_UNCHANGED_ERR[] = "There are uncommitted changes in working tree\n",
COMMIT_HEAD_DETACHED[] = "Cannot commit in detached HEAD mode\n",
BRANCH_HEAD_DETACHED[] = "Cannot create a new branch in detached HEAD mode\n",
REVERT_HEAD_DETACHED[] = "Cannot create revert in detached HEAD mode\n",
CHECKOUT_CANCELLED[] = "Checkout cancelled.\n",
MERGE_CANCELLED[] = "Merge cancelled.\n",
REVERT_INVALID_ID[] = "Could not revert to the specified commit identifier\n",
CONFLICT_FOUND[] = "Conflict detected: Merge cancelled\n",
BRANCH_NOT_FOUND[] = "Could not locate the specified branch(es)\n",
INVALID_LINE_NUMBERS[] = "Invalid line numbers\n",
INVALID_COMMIT_IDS[] = "Invalid commit ID\n",
TAG_NOT_FOUND[] = "Tag name not found\n",
TAG_EXISTS[] = "Tag name already exists. Use -f to overwrite\n",
MISSING_ARGUMENTS[] = "Missing arguments\n",

SHORTCUT_NOT_FOUND[] = "\033[0;31mCouldn't locate shortcut key \"%s\"\033[0m\n",
SHORTCUT_ALREADY_EXISTS[] = "\033[0;31mShortcut key \"%s\" already exists\033[0m\n",

// messages
REPO_INIT_MESSAGE[] = "Initialized empty Lit repository in %s\n",
FILEN_NOT_FOUND[] = "Could not locate file %s\n",
FILES_NOT_FOUND[] = "Could not locate file(s)\n",
STAGE_SUCCESS[] = "File(s) staged successfully\n",
FILE_IS_STAGED_COLOR[] = "\033[0;32mStaged\033[0m\n",
FILE_ISNT_STAGED_COLOR[] = "\033[0;33mNot staged\033[0m\n",
FILE_IS_UNTRACKED_COLOR[] = "\033[0;31mUntracked\033[0m\n",
FILE_IS_UNCHANGED_COLOR[] = "\033[0;35mUnchanged\033[0m\n",
RESET_SUCCESS[] = "File(s) unstaged successfully\n",
RED_MINUS[] = "\033[1;31m-\033[0m",
GREED_PLUS[] = "\033[1;32m+\033[0m",
MODIFIED_COLOR[] = "\033[0;36mM\033[0m\n",
ADDED_COLOR[] = "\033[0;32mA\033[0m\n",
DELETED_COLOR[] = "\033[0;31mD\033[0m\n",
COMMIT_SUCCESS[] = "Changes committed successfully \nCommit id: \033[0;36m%x\033[0m \nTime created: \033[0;34m%s\033[0m \nMessage: \033[0;33m%s\033[0m\n",
ON_BRANCH[] = "On branch \033[1;32m%s\033[0m\n",
BRANCH_PRINT_FORMAT[] = "\033[1;32m%s\033[0m",
SHORTCUT_ADD_SUCCESS[] = "Shortcut \"%s\" added successfully\n",
SHORTCUT_REPLACE_SUCCESS[] = "Shortcut \"%s\" replaced successfully\n",
SHORTCUT_REMOVE_SUCCESS[] = "Shortcut \"%s\" removed successfully\n",
BRANCH_CREATE_SUCCESS[] = "Created new branch \"%s\" on HEAD(%x)\n",
DETACHED_WARNING[] = "\033[0;33mWarning: HEAD is now detached\033[0m\n",
CHECKOUT_SUCCESS[] = "Switched to %s\n",
CHECKOUT_HEAD_WARNING[] = "\033[0;33mThis action will erase all changes in the working tree. Do you want to continue?(Y/n): \033[0m",
REVERT_SUCCESS[] = "Successfully reverted to %s\n",
REVERT_SUCCESS_N[] = "Successfully reverted working tree to %s (No commit)\n",
REVERT_IS_SAME[] = "Destination data is the same as HEAD. Nothing committed.\n",
MERGE_SUCCESS[] = "Merged two branches \033[1;32m%s\033[0m and \033[1;32m%s\033[0m\n",
TAG_SUCCESS[] = "Tag \'%s\' created successfully\n",
TAG_SUCCESS_OW[] = "Tag \'%s\' overwritten successfully\n",
NO_MATCH[] = "No match was found for \'%s\'\n",

LOG_FORMAT[] = "Commit \033[0;36m%x\033[0m \nBranch: \033[1;32m%s\033[0m \nAuthor: %s <%s> \nDate: \033[0;34m%s\033[0mNumber of changes: %d \nMessage: \033[0;33m%s\033[0m\n\n",
HEAD_MARKING[] = "Commit \033[0;36m%x\033[0m [\033[3;1;36mHEAD\033[0m]\nBranch: \033[1;32m%s\033[0m \nAuthor: %s <%s> \nDate: \033[0;34m%s\033[0mNumber of changes: %d \nMessage: \033[0;33m%s\033[0m\n\n"
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
#define SHORTCUT_MAX 40

// Branch
#define BRANCH_NAME_MAX 40

// File
#define MAX_LINE_LENGTH 300

// Tag
#define TAG_NAME_MAX 60

// Usage text
const char USAGE[] = "Usage: lit <command> [<args>]\n";
#endif // CONSTANTS_H