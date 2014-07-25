# ==========
# Versioning
# ==========
#
# VERSION_MAJOR	Denotes major milestones.
#
# VERSION_MINOR	Denotes feature implementations.
#
# VERSION_PATCH	Minor changes and fixes.
#
#
# - The package VERSION will be VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH.
#
# - Update the version information only immediately before a public release
#   of your software. More frequent updates are unnecessary.
#
# - If the source code has changed at all since the last update,
#   then increment VERSION_PATCH.
#

#=======
# - MOST IMPORTANT:
#   - Before you submitt to git:
#     - Remember the new version in 'LAST RELEASED:'
#     - State the new version in the changes file by adding a line
#       "- version MAJOR.MINOR.PATCH"
#     - Commit changes and version files together in a separate
#       commit using -m 'changes MAJOR.MINOR.PATCH'
#     - Tag the above commit with 'MAJOR.MINOR.PATCH' using
#       -m "tagging MAJOR.MINOR.PATCH".
#
# - Consider calling ./mkChangelog to assist you.
#   See './mkChangelog -h' for help.
#
SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "11")
SET(VERSION_PATCH "11")

# LAST RELEASED: 1.11.11
#=======
