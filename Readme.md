# Zypper

### World's most powerful command line package manager 

Zypper is a command line package manager which makes use of libzypp. Zypper provides functions like repository access, dependency solving, package installation, etc.

YaST2 and RPM MetaData package repositories are supported. Zypper repositories are similar to the ones used in YaST, which also makes use of libzypp. Zypper can also handle repository extensions like patches, patterns, and products.

## Usage: 

* Adding a repo:
   
   ` $ sudo zypper addrepo [repo url]`
   
* Getting information about a package:

   `$ sudo zypper info [package name]`

* Renaming a repo:

   `$ sudo zypper namerepo [repo name] [new repo name]`
   
* Removing a repo: 

   `$ sudo zypper removerepo [repo name]`
   
* Refreshing repos:

   `$ sudo zypper refresh`
   
* Showing the repos:
  
  `$ zypper repos`
  
 * Installing a package:
   
   `$ sudo zypper install [package name]`
  
* Removing a package:

   `$ sudo zypper remove [package name]`
   
* Upgrading your distribution:

   `$ sudo zypper dist-upgrade`
