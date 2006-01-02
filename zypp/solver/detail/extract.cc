/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * extract.cc
 *
 * Copyright (C) 2000-2003 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 */

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

#include <zypp/solver/detail/extract.h>
#include <zypp/solver/detail/XmlParser.h>
#include <zypp/solver/detail/utils.h>

#include <zypp/solver/detail/debug.h>

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////
      
      using namespace std;
      
      
      int
      extract_packages_from_xml_node (XmlNodePtr node, ChannelPtr channel, CResItemFn callback, void *data)
      {
          PackagePtr package;
          int count = 0;
      
          if (getenv ("RC_SPEW_XML")) fprintf (stderr, "extract_packages_from_xml_node ()\n");
          // search the 'package' node
      
          while (node && !node->equals("package")) {
      	if (!node->isElement()) {
      	    node = node->next();
      	    continue;
      	}
      
      	node = node->children();
          }
      
          // extract the 'package' node, if found
      
          while (node) {
      	if (node->equals("package")) {
      	    package = new Package (node, channel);
      	    if (package) {
      		if (getenv ("RC_SPEW")) fprintf (stderr, "%s\n", package->asString(true).c_str());
      		bool ok = true;
      		if (callback)
      		    ok = callback (package, data);
      		if (! ok)
      		    return -1;
      		++count;
      	    }
      	}
      	node = node->next();
          }
      
          return count;
      }
      
      
      int 
      extract_packages_from_helix_buffer (const char *buf, size_t len, ChannelPtr channel, CResItemFn callback, void *data)
      {
          unsigned int count = 0;
          PackageList packages;
      
          if (getenv ("RC_SPEW_XML")) fprintf (stderr, "extract_packages_from_helix_buffer(%.32s...,%ld,...)\n", buf, (long)len);
      
          if (buf == NULL || len == 0)
      	return 0;
      
          XmlParser parser (channel);
          parser.parseChunk (buf, len);
          packages = parser.done ();
      
          if (packages.empty())
      	return 0;
      
          count = packages.size();
      
          if (getenv ("RC_SPEW_XML")) fprintf (stderr, "extract_packages_from_helix_buffer: parsed %d packages\n", count);
      
          if (callback != NULL) {
      	for (PackageList::iterator iter = packages.begin(); iter != packages.end(); iter++) {
      	    callback (*iter, data);
      	}
          }
        
          return count;
      }
      
      
      int
      extract_packages_from_helix_file (const string & filename, ChannelPtr channel, CResItemFn callback, void *data)
      {
          Buffer *buf;
          int count;
      
          if (filename.empty())
      	return -1;
      
          buf = buffer_map_file (filename);
          if (buf == NULL)
      	return -1;
      
          count = extract_packages_from_helix_buffer ((const char *)(buf->data), buf->size, channel, callback, data);
      
          buffer_unmap_file (buf);
      
          return count;
      }
      
      
      int
      extract_packages_from_undump_buffer (const char *buf, size_t len, ChannelAndSubscribedFn channel_callback, CResItemFn resItem_callback, MatchFn lock_callback, void *data)
      {
          xmlDoc *doc;
          XmlNodePtr dump_node;
          ChannelPtr system_channel = NULL;
          ChannelPtr current_channel = NULL;
          XmlNodePtr channel_node;
          int count = 0;
      
          doc = parse_xml_from_buffer (buf, len);
          if (doc == NULL)
      	return -1;
      
          dump_node = new XmlNode (xmlDocGetRootElement (doc));
          if (dump_node == NULL)
      	return -1;
      
          if (!dump_node->equals("world")) {
      	debug (DEBUG_LEVEL_WARNING, "Unrecognized top-level node for undump: '%s'", dump_node->name());
      	return -1;
          }
      
          channel_node = dump_node->children();
      
          while (channel_node != NULL) {
      
      	if (channel_node->equals("locks")) {
      	    XmlNodePtr lock_node = channel_node->children();
      
      	    while (lock_node) {
      		MatchPtr lock;
      
      		lock = new Match (lock_node);
      
      		if (lock_callback)
      		    lock_callback (lock, data);
      
      		lock_node = lock_node->next();
      	    }
      
      	} else if (channel_node->equals("system_packages")) {
      
      	    int subcount;
      
      	    if (!system_channel) {
      		system_channel = new Channel ("@system", "System Packages", "@system", "System Packages");
      		system_channel->setSystem (true);
      		system_channel->setHidden (true);
      	    }
      
      	    if (channel_callback) {
      		channel_callback (system_channel, false, data);
      	    }
      	    
      	    subcount = extract_packages_from_xml_node (channel_node, system_channel, resItem_callback, data);
      
      	    if (subcount < 0) {
      		/* Do something clever */
      		fprintf (stderr, "No packages found\n");
      		abort ();
      	    }
      	    
      	    count += subcount;
      
      	} else if (channel_node->equals("channel")) {
      
      	    int subscribed;
      	    current_channel = new Channel (channel_node, &subscribed, (World *)data);
      
      	    if (channel_callback) {
      		channel_callback (current_channel, subscribed != 0, data);
      	    }
      
      	    if (resItem_callback) {
      		int subcount;
      		subcount = extract_packages_from_xml_node (channel_node, current_channel, resItem_callback, data);
      		if (subcount < 0) {
      		    /* FIXME: do something clever */
      		    fprintf (stderr, "No packages found\n");
      		    abort ();
      		}
      		count += subcount;
      	    }
      	}
      
      	channel_node = channel_node->next();
          }
      
          xmlFreeDoc (doc);
      
          return count;
      }
      
      
      int
      extract_packages_from_undump_file (const string & filename, ChannelAndSubscribedFn channel_callback, CResItemFn resItem_callback, MatchFn lock_callback, void *data)
      {
          Buffer *buf;
          int count;
      
          if (filename.empty())
      	return -1;
      
          buf = buffer_map_file (filename);
          if (buf == NULL)
      	return -1;
      
          count = extract_packages_from_undump_buffer ((const char *)(buf->data), buf->size, channel_callback, resItem_callback, lock_callback, data);
      
          buffer_unmap_file (buf);
      
          return count;
      }
      
      #if 0
      /* ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** */
      
      static ResItemPtr
      fill_debian_package (const char *buf, const char *url_prefix, int *off)
      {
          const char *ibuf;
          RCPackageUpdate *up = NULL;
          ResItemPtr r;
          ResItemList requires, provides, conflicts, suggests, recommends;
      
          up = rc_package_update_new ();
      
          ibuf = buf;
          while (1) {
      	char *key;
      	GString *value = NULL;
      	const char *p;
      	int ind;
      
      	/* Linebreaks indicate the end of a package block. */
      	if (*ibuf == '\0' || *ibuf == '\n') break;
      
      	p = strchr (ibuf, ':');
      
      	/* Something bad happened, we're supposed to have a colon. */
      	if (!p) break;
      
      	/* Copy the name of the key and lowercase it */
      	key = g_ascii_strdown (ibuf, p - ibuf);
      	
      	/* Move past the colon and any spaces */
      	ibuf = p;
      	while (*ibuf && (*ibuf == ':' || *ibuf == ' ')) ibuf++;
      
      	ind = 0;
      	while ((p = strchr (ibuf, '\n'))) {
      	    if (!value)
      		value = g_string_new ("");
      
      	    g_string_append_len (value, ibuf, p - ibuf);
      	    ind += p - ibuf;
      
      	    ibuf = p;
      
      	    /* Move past the newline */
      	    ibuf++;
      
      	    /* Check to see if this is a continuation of the previous line */
      	    if (*ibuf == ' ') {
      		/* It is.  Move past the space */
      		ibuf++;
      
      		/*
      		 * This is a hack.  Description is special because it's
      		 * intended to be multiline and user-visible.  So if we're
      		 * dealing with description, add a newline.
      		 */
      
      		if (strncmp (key, "description",
      			     strlen ("description")) == 0) {
      		    g_string_append_c (value, '\n');
      
      		    /*
      		     * A period on a line by itself indicates that it
      		     * should be a blank line.  A newline will follow the
      		     * period, so we'll just skip over it.
      		     */
      		    if (*ibuf == '.')
      			ibuf++;
      		}
      	    }
      	    else {
      		/* It isn't.  Break out. */
      		break;
      	    }
      	}
      
      	if (!strncmp (key, "package", strlen ("package"))) {
      	    rc_resItem_spec_set_name (RC_RESOLVABLE_SPEC (pkg), value->str);
      	} else if (!strncmp (key, "installed-size",
      			     strlen ("installed-size"))) {
      	    up->installed_size = strtoul (value->str, NULL, 10) * 1024;
      	} else if (!strncmp (key, "size", strlen ("size"))) {
      	    up->package_size = strtoul(value->str, NULL, 10);
      	} else if (!strncmp (key, "description", strlen ("description"))) {
      	    char *newline;
      
      	    /*
      	     * We only want the first line for the summary, and all the
      	     * other lines for the description.
      	     */
      
      	    newline = strchr (value->str, '\n');
      	    if (!newline) {
      		pkg->summary = strdup (value->str);
      		pkg->description = g_strconcat (value->str, "\n", NULL);
      	    }
      	    else {
      		pkg->summary = g_strndup (value->str, newline - value->str);
      		pkg->description = g_strconcat (newline + 1, "\n", NULL);
      	    }
      	} else if (!strncmp (key, "version", strlen ("version"))) {
      	    RCResItemSpec *spec = RC_RESOLVABLE_SPEC (pkg);
      	    rc_version_parse (value->str, spec);
      	} else if (!strncmp (key, "section", strlen ("section"))) {
      	    pkg->section = rc_debman_section_to_package_section (value->str);
      	} else if (!strncmp (key, "depends", strlen ("depends"))) {
      	    requires = g_slist_concat (
      		requires,
      		rc_debman_fill_depends (value->str, false));
      	} else if (!strncmp (key, "recommends", strlen ("recommends"))) {
      	    recommends = g_slist_concat (
      		recommends,
      		rc_debman_fill_depends (value->str, false));
      	} else if (!strncmp (key, "suggests", strlen ("suggests"))) {
      	    suggests = g_slist_concat (
      		suggests,
      		rc_debman_fill_depends (value->str, false));
      	} else if (!strncmp (key, "pre-depends", strlen ("pre-depends"))) {
      	    requires = g_slist_concat (
      		requires,
      		rc_debman_fill_depends (value->str, true));
      	} else if (!strncmp (key, "conflicts", strlen ("conflicts"))) {
      	    conflicts = g_slist_concat (
      		conflicts,
      		rc_debman_fill_depends (value->str, false));
      	} else if (!strncmp (key, "provides", strlen ("provides"))) {
      	    provides = g_slist_concat (
      		provides,
      		rc_debman_fill_depends (value->str, false));
      	} else if (!strncmp (key, "filename", strlen ("filename"))) {
      	    /* Build a new update with just this version */
      	    if (url_prefix) {
      		up->package_url = g_strconcat (url_prefix, "/",
      					       value->str,
      					       NULL);
      	    } else {
      		up->package_url = strdup (value->str);
      	    }
      	} else if (!strncmp (key, "md5sum", strlen ("md5sum"))) {
      	    up->md5sum = strdup (value->str);
      	} else if (!strncmp (key, "architecture", strlen ("architecture"))) {
      	    rc_resItem_spec_set_arch (RC_RESOLVABLE_SPEC (pkg), rc_arch_from_string (value->str));
      	}
      
      	g_string_free (value, true);
          }
      
          up->importance = RC_IMPORTANCE_SUGGESTED;
          up->description = strdup ("Upstream Debian release");
          rc_resItem_spec_copy (rc_package_update_get_spec(up), RC_RESOLVABLE_SPEC (pkg));
          rc_package_add_update (pkg, up);
      
          r = RC_RESOLVABLE (pkg);
      
          /* Make sure to provide myself, for the dep code! */
          provides = g_slist_append (provides, rc_resItem_dep_new_from_spec
      			       (RC_RESOLVABLE_SPEC (pkg),
      				RC_RELATION_EQUAL,
      				RC_TYPE_PACKAGE,
      				rc_resItem_get_channel (r),
      				false, false));
      
          rc_resItem_set_requires  (r, requires);
          rc_resItem_set_provides  (r, provides);
          rc_resItem_set_conflicts (r, conflicts);
          rc_resItem_set_obsoletes (r, NULL);
          rc_resItem_set_suggests  (r, suggests);
          rc_resItem_set_recommends(r, recommends);
          /* returns the number of characters we processed */
          return ibuf - buf;
      }
      
      #endif
      
      int 
      extract_packages_from_debian_buffer (const char *buf, size_t len, ChannelPtr channel, CResItemFn callback, void *data)
      {
          const char *pos;
          int count = 0;
      
          /* Keep looking for a "Package: " */
          pos = buf;
      #if 0
          while ((pos = strstr (pos, "Package: ")) != NULL) {
      	int off;
      
      	/* All debian packages "have" epochs */
      	ResItemPtr resItem = fill_debian_package (iter, channel->getFilePath (), &off);
      
      	resItem->setEpoch (0);
      	resItem->setArch ( Arch());
      	resItem->setChannel (channel);
      
      	if (callback)
      	    callback (resItem, data);
      
      	++count;
      
      	iter += off;
          }
      #endif
          return count;
      }
      
      
      int
      extract_packages_from_debian_file (const string & filename, ChannelPtr channel, CResItemFn callback, void *data)
      {
          Buffer *buf;
          int count;
      
          if (filename.empty())
      	return -1;
      
          buf = buffer_map_file (filename);
          if (buf == NULL)
      	return -1;
      
          count = extract_packages_from_debian_buffer ((const char *)(buf->data), buf->size, channel, callback, data);
          buffer_unmap_file (buf);
      
          return count;
      }
      
      #if 0
      /* ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** */
      
      PackagePtr 
      extract_yum_package (const guint8 *data, size_t len,
      			RCPackman *packman, char *url)
      {
      #ifndef  ENABLE_RPM
          /* We can't support yum without rpm support */
          debug (RC_DEBUG_LEVEL_ERROR, "RPM support is not enabled");
          return NULL;
      #else
          RCRpmman *rpmman;
          Header h;
          PackagePtr p;
          RCPackageUpdate *pu;
          char *tmpc;
          int typ, n;
      
          g_return_val_if_fail (packman != NULL, NULL);
      
          if (!g_type_is_a (G_TYPE_FROM_INSTANCE (packman), RC_TYPE_RPMMAN)) {
      	debug (RC_DEBUG_LEVEL_ERROR,
      		  "yum support is not available on non-RPM systems");
      	return NULL;
          }
      
          rpmman = RC_RPMMAN (packman);
          
          h = rpmman->headerLoad (data);
      
          if (h == NULL) {
      	debug (RC_DEBUG_LEVEL_ERROR,
      		  "Unable to get header from headerCopyLoad!");
      	return NULL;
          }
      
          rpmman->headerGetEntry (h, RPMTAG_ARCH, &typ, (void **) &tmpc, &n);
      
          p = rc_package_new ();
      
          rc_rpmman_read_header (rpmman, h, p);
          rc_rpmman_depends_fill (rpmman, h, p, true);
      
          pu = rc_package_update_new ();
          rc_resItem_spec_copy (rc_package_update_get_spec (pu), RC_RESOLVABLE_SPEC (p));
          pu->importance = RC_IMPORTANCE_SUGGESTED;
          pu->description = strdup ("No information available.");
          pu->package_url = url;
          
          p->history = g_slist_append (p->history, pu);
      
          rpmman->headerFree (h);
      
          return p;
      #endif  
      }
      
      int
      extract_packages_from_aptrpm_buffer (const guint8 *data, size_t len,
      					RCPackman *packman,
      					ChannelPtr channel,
      					CResItemFn callback,
      					void * user_data)
      {
      #ifndef ENABLE_RPM
          /* We can't support apt-rpm without rpm support */
          debug (RC_DEBUG_LEVEL_ERROR, "RPM support is not enabled");
          return -1;
      #else
          RCRpmman *rpmman;
          int count = 0;
          const int hdrmagic_len = 8;
          const char *hdrmagic;
          const guint8 *cur_ptr;
          RCResItemSpec *spec;
      
      
          g_return_val_if_fail (packman != NULL, -1);
      
          if (!g_type_is_a (G_TYPE_FROM_INSTANCE (packman), RC_TYPE_RPMMAN)) {
      	debug (RC_DEBUG_LEVEL_ERROR,
      		  "apt-rpm support is not available on non-RPM systems");
      	return -1;
          }
      
          rpmman = RC_RPMMAN (packman);
      
          if (len < hdrmagic_len) {
      	debug (RC_DEBUG_LEVEL_ERROR,
      		  "Data is too small to possibly be correct");
      	return 0;
          }
      
          /*
           * The apt-rpm pkglist files are a set of rpm headers, each prefixed
           * with the header magic, one right after the other.  If opened on disk,
           * they can be iterated using headerRead().  Since we have an in-memory
           * buffer, we use headerCopyLoad to read them.  We could, potentially,
           * use headerLoad(); but I'm unsure as to what happens when headerFree
           * is called on a Header returned from headerLoad.  It may be a small
           * memory savings to do so.
           */
      
          /* Skip the inital RPM header magic */
          hdrmagic = data;
          cur_ptr = data + hdrmagic_len;
      
          while (cur_ptr != NULL) {
      	Header h;
      	PackagePtr p;
      	RCPackageUpdate *pu;
      	int bytesleft, i;
      	char *tmpc;
      	int typ, n;
      	char *archstr;
      
      	h = rpmman->headerLoad (cur_ptr);
      
      	if (h == NULL) {
      	    debug (RC_DEBUG_LEVEL_ERROR,
      		      "Unable to get header from headerCopyLoad!");
      	    return 0;
      	}
      
      	rpmman->headerGetEntry (h, RPMTAG_ARCH, &typ, (void **) &tmpc, &n);
      
      	if (n && typ == RPM_STRING_TYPE)
      	    archstr = tmpc;
      	else {
      	    debug (RC_DEBUG_LEVEL_WARNING, "No arch available!");
      	    goto cleanup;
      	}
      
      	p = rc_package_new ();
      
      	rc_rpmman_read_header (rpmman, h, p);
      	rc_rpmman_depends_fill (rpmman, h, p, true);
      
      	rc_resItem_set_channel (RC_RESOLVABLE (p), channel);
      
      	pu = rc_package_update_new ();
      	rc_resItem_spec_copy (rc_package_update_get_spec (pu), RC_RESOLVABLE_SPEC (p));
      	pu->importance = RC_IMPORTANCE_SUGGESTED;
      	pu->description = strdup ("No information available.");
      
      	/* Build a filename from the spec */
      	spec = RC_RESOLVABLE_SPEC (p);
      	pu->package_url = strdup_printf ("%s/%s-%s-%s.%s.rpm",
      					   rc_channel_get_file_path (channel),
      					   rc_resItem_spec_get_name (spec),
      					   rc_resItem_spec_get_version (spec),
      					   rc_resItem_spec_get_release (spec),
      					   archstr);
      
      	p->history = g_slist_append (p->history, pu);
      
      	if (callback)
      	    callback ((RCResItem *) p, user_data);
      
      	g_object_unref (p);
      
      	++count;
      
          cleanup:
      	rpmman->headerFree (h);
      
      	/* This chunk of ugly could be removed if a) memmem() was portable;
      	 * or b) if rpmlib didn't suck, and I could figure out how much
      	 * data it read from the buffer.
      	 */
      	bytesleft = len - (cur_ptr - data);
      	for (i = 0; i < bytesleft - hdrmagic_len; i++) {
      	    if (memcmp (cur_ptr + i, hdrmagic, hdrmagic_len) == 0) {
      		/* We found a match */
      		cur_ptr = cur_ptr + i + hdrmagic_len;
      		break;
      	    }
      	}
      
      	if (i >= bytesleft - hdrmagic_len) {
      	    /* No match was found */
      	    cur_ptr = NULL;
      	}
          }
      
          return count;
      #endif
      }
      
      int
      extract_packages_from_aptrpm_file (const char *filename,
      				      RCPackman *packman,
      				      ChannelPtr channel,
      				      CResItemFn callback,
      				      void * user_data)
      {
          WorldPtr world = *((WorldPtr *)data);
          RCBuffer *buf;
          int count;
      
          g_return_val_if_fail (filename != NULL, -1);
          g_return_val_if_fail (packman != NULL, -1);
      
          buf = rc_buffer_map_file (filename);
          if (buf == NULL)
      	return -1;
      
          count = extract_packages_from_aptrpm_buffer (buf->data, buf->size,
      						    packman, channel,
      						    callback, user_data);
      
          rc_buffer_unmap_file (buf);
      
          return count;
      }
      
      /* ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** */
      
      static void
      package_into_hash (PackagePtr pkg, GHashTable *hash)
      {
          void * nameq;
          PackagePtr hashed_pkg;
      
          nameq = GINT_TO_POINTER (RC_RESOLVABLE_SPEC (pkg)->nameq);
          hashed_pkg = g_hash_table_lookup (hash, nameq);
          if (hashed_pkg == NULL) {
      	g_hash_table_insert (hash, nameq, g_object_ref (pkg));
          } else if (rc_version_compare (RC_RESOLVABLE_SPEC (pkg),
      				   RC_RESOLVABLE_SPEC (hashed_pkg)) > 0) {
      	g_hash_table_replace (hash, nameq, g_object_ref (pkg));
      	g_object_unref (hashed_pkg);
          }
      }
      
      static bool
      hash_recurse_cb (PackagePtr pkg, void * user_data)
      {
          GHashTable *hash = user_data;
          package_into_hash (pkg, hash);
          return true;
      }
      
      struct HashIterInfo {
          CResItemFn callback;
          void * user_data;
          int count;
      };
      
      static void
      hash_iter_cb (void * key, void * val, void * user_data)
      {
          RCResItem *r = val;
          struct HashIterInfo *info = user_data;
      
          if (info->callback)
      	info->callback (r, info->user_data);
      
          g_object_unref (r);
          ++info->count;
      }
      
      
      static void
      add_fake_history (PackagePtr pkg)
      {
          RCPackageUpdate *up;
      
          up = rc_package_update_new ();
          rc_resItem_spec_copy ((RCResItemSpec *) up,
      			     RC_RESOLVABLE_SPEC (pkg));
          up->importance = RC_IMPORTANCE_SUGGESTED;
          rc_package_add_update (pkg, up);
      }
      
      typedef struct {
          CResItemFn user_callback;
          void *    user_data;
          const gchar *path;
      } PackagesFromDirInfo;
      
      static bool
      packages_from_dir_cb (PackagePtr package, void * user_data)
      {
          PackagesFromDirInfo *info = user_data;
          RCPackageUpdate *update;
      
          /* Set package path */
          update = rc_package_get_latest_update (package);
          if (update && update->package_url)
      	package->package_filename = g_build_path (G_DIR_SEPARATOR_S,
      						  info->path,
      						  update->package_url,
      						  NULL);
          if (info->user_callback)
      	return info->user_callback ((RCResItem *)package, info->user_data);
      
          return true;
      }
      
      int
      extract_packages_from_directory (const char *path,
      				    ChannelPtr channel,
      				    RCPackman *packman,
      				    bool recursive,
      				    CResItemFn callback,
      				    void * user_data)
      {
          WorldPtr world = *((WorldPtr *)data);
          GDir *dir;
          GHashTable *hash;
          struct HashIterInfo info;
          const char *filename;
          char *magic;
          bool distro_magic, pkginfo_magic;
          
          g_return_val_if_fail (path && *path, -1);
          g_return_val_if_fail (channel != NULL, -1);
      
          /*
            Check for magic files that indicate how to treat the
            directory.  The files aren't read -- it is sufficient that
            they exist.
          */
      
          magic = g_strconcat (path, "/RC_SKIP", NULL);
          if (g_file_test (magic, G_FILE_TEST_EXISTS)) {
      	g_free (magic);
      	return 0;
          }
          g_free (magic);
      
          magic = g_strconcat (path, "/RC_RECURSIVE", NULL);
          if (g_file_test (magic, G_FILE_TEST_EXISTS))
      	recursive = true;
          g_free (magic);
          
          magic = g_strconcat (path, "/RC_BY_DISTRO", NULL);
          distro_magic = g_file_test (magic, G_FILE_TEST_EXISTS);
          g_free (magic);
      
          pkginfo_magic = true;
          magic = g_strconcat (path, "/RC_IGNORE_PKGINFO", NULL);
          if (g_file_test (magic, G_FILE_TEST_EXISTS))
      	pkginfo_magic = false;
          g_free (magic);
      
          /* If distro_magic is set, we search for packages in two
             subdirectories of path: path/distro-target (i.e.
             path/redhat-9-i386) and path/x-cross.
          */
      
      #if 0      
          if (distro_magic) {
      	char *distro_path, *cross_distro_path;
      	bool found_distro_magic = false;
      	int count = 0, c;
      
      	distro_path = g_strconcat (path, "/", rc_distro_get_target (), NULL);
      	if (g_file_test (distro_path, G_FILE_TEST_IS_DIR)) {
      	    found_distro_magic = true;
      
      	    c = extract_packages_from_directory (distro_path,
      						    channel, packman,
      						    callback, user_data);
      	    if (c >= 0)
      		count += c;
      	}
      
      	cross_distro_path = g_strconcat (path, "/x-distro", NULL);
      	if (g_file_test (cross_distro_path, G_FILE_TEST_IS_DIR)) {
      	    c = extract_packages_from_directory (cross_distro_path,
      						    channel, packman,
      						    callback, user_data);
      	    if (c >= 0)
      		count += c;
      	}
      
      	g_free (cross_distro_path);
      	g_free (distro_path);
      
      	return count;
          }
      #endif
      
          /* If pkginfo_magic is set and if a packageinfo.xml or
             packageinfo.xml.gz file exists in the directory, use it
             instead of just scanning the files in the directory
             looking for packages. */
      
          if (pkginfo_magic) {
      	int i, count;
      	gchar *pkginfo_path = NULL;
      	const gchar *pkginfo[] = { "packageinfo.xml",
      				   "packageinfo.xml.gz",
      				   NULL };
      
      	for (i = 0; pkginfo[i]; i++) {
      	    pkginfo_path = g_build_path (G_DIR_SEPARATOR_S, path, pkginfo[i], NULL);
      	    if (g_file_test (pkginfo_path, G_FILE_TEST_EXISTS))
      		break;
      
      	    g_free (pkginfo_path);
      	    pkginfo_path = NULL;
      	}
      
      	if (pkginfo_path) {
      	    PackagesFromDirInfo info;
      
      	    info.user_callback = callback;
      	    info.user_data = user_data;
      	    info.path = path;
      
      	    count = extract_packages_from_helix_file (pkginfo_path, channel, packages_from_dir_cb, &info);
      	    g_free (pkginfo_path);
      	    return count;
      	}
          }
      
          dir = g_dir_open (path, 0, NULL);
          if (dir == NULL)
      	return -1;
      
          hash = g_hash_table_new (NULL, NULL);
      
          while ( (filename = g_dir_read_name (dir)) ) {
      	gchar *file_path;
      
      	file_path = g_strconcat (path, "/", filename, NULL);
      
      	if (recursive && g_file_test (file_path, G_FILE_TEST_IS_DIR)) {
      	    extract_packages_from_directory (file_path,
      						channel,
      						packman,
      						true,
      						hash_recurse_cb,
      						hash);
      	} else if (g_file_test (file_path, G_FILE_TEST_IS_REGULAR)) {
      	    PackagePtr pkg;
      
      	    pkg = rc_packman_query_file (packman, file_path, true);
      	    if (pkg != NULL) {
      		rc_resItem_set_channel (RC_RESOLVABLE (pkg), channel);
      		pkg->package_filename = strdup (file_path);
      		pkg->local_package = false;
      		add_fake_history (pkg);
      		package_into_hash (pkg, hash);
      		g_object_unref (pkg);
      	    }
      	}
      
      	g_free (file_path);
          }
      
          g_dir_close (dir);
         
          info.callback = callback;
          info.user_data = user_data;
          info.count = 0;
      
          /* Walk across the hash and:
             1) Invoke the callback on each package
             2) Unref each package
          */
          g_hash_table_foreach (hash, hash_iter_cb, &info);
      
          g_hash_table_destroy (hash);
      
          return info.count;
      }
      #endif

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////      


