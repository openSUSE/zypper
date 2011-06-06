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
# - Update version according to your changes,
#   but based on 'LAST RELEASED:' below. I.e
#   there's no need to increase VERSION_PATCH
#   if it already differs from 'LAST RELEASED:'.
#
# - MOST IMPORTANT:
#   Before you submitt to autobuild, remember the
#   new version in 'LAST RELEASED:', and add a
#   note in the changes file.
#
SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "6")
SET(VERSION_PATCH "10")

# LAST RELEASED: 1.6.10
#=======
