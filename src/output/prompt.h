#ifndef PROMPT_H_
  #ifndef PR_ENUML
    #define PR_ENUML(nam, val) nam = val
  #else
    #define PR_MAN
  #endif
  #ifndef PR_ENUM
    #define PR_ENUM(nam, val) PR_ENUML(nam, val),
  #endif
#else
  #ifndef PR_ENUML
    #define PR_ENUML(nam, val)
    #define PR_ENUM(nam, val)
  #else
    #define PR_MAN
    #ifndef PR_ENUM
      #define PR_ENUM(nam, val) PR_ENUML(nam, val),
    #endif
  #endif
#endif

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
#ifndef PROMPT_H_
#ifndef PR_MAN
typedef enum
{
#endif
#endif
  PR_ENUM(PROMPT_YN_INST_REMOVE_CONTINUE, 0)
  PR_ENUM(PROMPT_DEP_RESOLVE, 1)
  PR_ENUM(PROMPT_AUTH_USERNAME, 2)
  PR_ENUM(PROMPT_AUTH_PASSWORD, 3)
  PR_ENUM(PROMPT_ARI_RPM_DOWNLOAD_PROBLEM, 4)
  PR_ENUM(PROMPT_ARI_REPO_PROBLEM, 5)
  PR_ENUM(PROMPT_ARI_RPM_REMOVE_PROBLEM, 6)
  PR_ENUM(PROMPT_ARI_RPM_INSTALL_PROBLEM, 7)
  PR_ENUM(PROMPT_ARI_MEDIA_PROBLEM, 8)
  PR_ENUM(PROMPT_YN_MEDIA_CHANGE, 9)
  PR_ENUM(PROMPT_YN_LICENSE_AGREE, 10)
  PR_ENUM(PROMPT_YN_GPG_UNSIGNED_FILE_ACCEPT, 11)
//PR_ENUM(PROMPT_YN_GPG_KEY_IMPORT_TRUSTED, 12)
  PR_ENUM(PROMPT_YN_GPG_UNKNOWN_KEY_ACCEPT, 13)
  PR_ENUM(PROMPT_YN_GPG_KEY_TRUST, 14)
  PR_ENUM(PROMPT_YN_GPG_CHECK_FAILED_IGNORE, 15)
  PR_ENUM(PROMPT_GPG_NO_DIGEST_ACCEPT, 16)
  PR_ENUM(PROMPT_GPG_UNKNOWN_DIGEST_ACCEPT, 17)
  PR_ENUM(PROMPT_GPG_WRONG_DIGEST_ACCEPT, 18)
  PR_ENUM(PROMPT_YN_REMOVE_LOCK, 19)
  PR_ENUM(PROMPT_PATCH_MESSAGE_CONTINUE, 20)
  PR_ENUM(PROMPT_ARI_PATCH_SCRIPT_PROBLEM, 21)
  PR_ENUM(PROMPT_MEDIA_EJECT, 22)
  PR_ENUM(PROMPT_PACKAGEKIT_QUIT, 23)
  PR_ENUM(PROMPT_YN_CONTINUE_ON_FILECONFLICT, 24)

#ifndef PROMPT_H_
#define PROMPT_H_
#ifndef PR_MAN
} PromptId;

#endif
#endif /*PROMPT_H_*/

#undef PR_ENUM
#undef PR_ENUML
#undef PR_MAN
