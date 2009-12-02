#ifndef PROMPT_H_
#define PROMPT_H_

/**
 * Defines identifiers of zypper's user prompts. Used in machine-readable output
 * for easy and unique identification of the prompts by a machine.
 *
 * <table>
 * <tr>
 * <td>Id</td>
 * <td>#</td>
 * <td>Prompt</td>
 * <td>Description</td>
 * <td>answers</td>
 * </tr>
 * <tr>
 * <td>PROMPT_YN_INST_REMOVE_CONTINUE</td>
 * <td>0</td>
 * <td>Proceed with the installation/removal/update? Displayed right after the install summary.</td>
 * <td>y/n or stdlib localized answers (rpmatch)</td>
 * </tr>
 * </table>
 * \todo add the description of the rest
 */
typedef enum
{
  PROMPT_YN_INST_REMOVE_CONTINUE         =  0,
  PROMPT_DEP_RESOLVE                     =  1,
  PROMPT_AUTH_USERNAME                   =  2,
  PROMPT_AUTH_PASSWORD                   =  3,
  PROMPT_ARI_RPM_DOWNLOAD_PROBLEM        =  4,
  PROMPT_ARI_REPO_PROBLEM                =  5,
  PROMPT_ARI_RPM_REMOVE_PROBLEM          =  6,
  PROMPT_ARI_RPM_INSTALL_PROBLEM         =  7,
  PROMPT_ARI_MEDIA_PROBLEM               =  8,
  PROMPT_YN_MEDIA_CHANGE                 =  9,
  PROMPT_YN_LICENSE_AGREE                = 10,
  PROMPT_YN_GPG_UNSIGNED_FILE_ACCEPT     = 11,
//  PROMPT_YN_GPG_KEY_IMPORT_TRUSTED       = 12,
  PROMPT_YN_GPG_UNKNOWN_KEY_ACCEPT       = 13,
  PROMPT_YN_GPG_KEY_TRUST                = 14,
  PROMPT_YN_GPG_CHECK_FAILED_IGNORE      = 15,
  PROMPT_GPG_NO_DIGEST_ACCEPT            = 16,
  PROMPT_GPG_UNKNOWN_DIGEST_ACCEPT       = 17,
  PROMPT_GPG_WRONG_DIGEST_ACCEPT         = 18,
  PROMPT_YN_REMOVE_LOCK                  = 19,
  PROMPT_PATCH_MESSAGE_CONTINUE          = 20,
  PROMPT_ARI_PATCH_SCRIPT_PROBLEM        = 21,
  PROMPT_MEDIA_EJECT                     = 22
} PromptId;

#endif /*PROMPT_H_*/
