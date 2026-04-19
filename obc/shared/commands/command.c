#include "command.h"
#include "obc_errors.h"
#include "obc_gs_command_id.h"
#include "obc_logging.h"
#ifdef NO_FREERTOS
#include <stddef.h>
#endif

extern const cmd_info_t cmdsConfig[];

#define CMDS_CONFIG_SIZE NUM_CMD_CALLBACKS

// Used to track whether a safety-critical command is currently being executed
// This is inefficient space-wise, but simplifies the code. We can optimize later if needed.
static bool cmdProgressTracker[NUM_CMD_CALLBACKS] = {false};

obc_error_code_t verifyCommand(cmd_msg_t *cmd, cmd_info_t *currCmdInfo) {
  if (cmd == NULL || currCmdInfo == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  if (cmd->id >= CMDS_CONFIG_SIZE) {
    return OBC_ERR_CODE_UNSUPPORTED_CMD;
  }

  *currCmdInfo = cmdsConfig[cmd->id];
  // Check if the ID has a callback
  if (currCmdInfo->callback == NULL) {
    return OBC_ERR_CODE_UNSUPPORTED_CMD;
  }

  // Check if the command is allowed to be executed
  if (currCmdInfo->policy == CMD_POLICY_RND && OBC_ACTIVE_POLICY == CMD_POLICY_PROD) {
    return OBC_ERR_CODE_CMD_NOT_ALLOWED;
  }

  // Check if the command is safety-critical
  if (currCmdInfo->opts & CMD_TYPE_CRITICAL) {
    // TODO: Make this persistent across resets
    if (!cmdProgressTracker[cmd->id]) {
      // Begin the two-step process of executing a safety-critical command
      // TODO: Create arm cmd that sets the progress tracker to true and fully
      // implement the two-step process to execute the safety-critical command
      LOG_DEBUG("Process started to execute safety-critical command");
      return OBC_ERR_CODE_UNSUPPORTED_CMD;
    }

    // Reset the progress tracker
    cmdProgressTracker[cmd->id] = false;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t processNonTimeTaggedCommand(cmd_msg_t *cmd, cmd_info_t *currCmdInfo, uint8_t *responseData,
                                             uint8_t *responseDataLen) {
  if (cmd == NULL || currCmdInfo == NULL || responseData == NULL || responseDataLen == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  // If the command is not time-tagged, execute it immediately
  obc_error_code_t errCode;
  if (cmd->isTimeTagged) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  // TODO: Handle safety-critical command failures
  RETURN_IF_ERROR_CODE(currCmdInfo->callback(cmd, responseData, responseDataLen));
  return OBC_ERR_CODE_SUCCESS;
}
