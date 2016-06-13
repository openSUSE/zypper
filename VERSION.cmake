# ==================================================
# Versioning
# ==========
#
# MAJOR	Major number for this branch.
#
# MINOR	The most recent interface number this
# 		library implements.
#
# COMPATMINOR	The latest binary compatible minor number
# 		this library implements.
#
# PATCH	The implementation number of the current interface.
#
#
# - The package VERSION will be MAJOR.MINOR.PATCH.
#
# - Libtool's -version-info will be derived from MAJOR, MINOR, PATCH
#   and COMPATMINOR (see configure.ac).
#
# - Changing MAJOR always breaks binary compatibility.
#
# - Changing MINOR doesn't break binary compatibility by default.
#   Only if COMPATMINOR is changed as well.
#
#
# 1) After branching from TRUNK increment TRUNKs MAJOR and
#    start with version `MAJOR.0.0' and also set COMPATMINOR to 0.
#
# 2) Update the version information only immediately before a public release
#    of your software. More frequent updates are unnecessary, and only guarantee
#    that the current interface number gets larger faster.
#
# 3) If the library source code has changed at all since the last update,
#    then increment PATCH.
#
# 4) If any interfaces have been added, removed, or changed since the last
#    update, increment MINOR, and set PATCH to 0.
#
# 5) If any interfaces have been added since the last public release, then
#    leave COMPATMINOR unchanged. (binary compatible change)
#
# 6) If any interfaces have been removed since the last public release, then
#    set COMPATMINOR to MINOR. (binary incompatible change)
# ==================================================

#=======
# - MOST IMPORTANT:
#   - Before you submitt to git:
#     - Remember the new version in 'LAST RELEASED:'
#     - State the new version in the changes file by adding a line
#       "- version MAJOR.MINOR.PATCH (COMPATMINOR)"
#     - Commit changes and version files together in a separate
#       commit using -m 'changes MAJOR.MINOR.PATCH (COMPATMINOR)'
#     - Tag the above commit with 'MAJOR.MINOR.PATCH' using
#       -m "tagging MAJOR.MINOR.PATCH".
#
# - Consider calling ./mkChangelog to assist you.
#   See './mkChangelog -h' for help.
#
SET(LIBZYPP_MAJOR "16")
SET(LIBZYPP_COMPATMINOR "0")
SET(LIBZYPP_MINOR "0")
SET(LIBZYPP_PATCH "3")
#
# LAST RELEASED: 16.0.3 (0)
# (The number in parenthesis is LIBZYPP_COMPATMINOR)
#=======
