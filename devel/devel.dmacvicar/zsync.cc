#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include <zsync.h>
}
#include "zypp/base/Exception.h"
#include "zypp/base/Logger.h"
#include "zypp/Pathname.h"
#include "zypp/ExternalProgram.cc"
//#include 

using namespace zypp;
using namespace std;

void read_seed_file(struct zsync_state* z, const Pathname &path )
{
  if (zsync_hint_decompress(z) && path.basename().size() > 3 && path.extension() == ".gz" )
  {
    FILE* f;
    {
      // ugh
      char* cmd = (char *) malloc(6 + strlen(path.c_str())*2);

      if (!cmd) return;

      const char *fname = path.c_str();
      strcpy(cmd,"zcat ");
      {
        int i,j;
        for (i=0,j=5; fname[i]; i++)
        {
          if (!isalnum(fname[i])) cmd[j++] = '\\';
            cmd[j++] = fname[i];
        }
        cmd[j] = 0;
      }

      //if (!no_progress) fprintf(stderr,"reading seed %s: ",cmd);
      MIL << "Reading seed " << cmd << endl;
      f = popen(cmd,"r");
      free(cmd);
    }

    if (!f)
    {
      //perror("popen"); fprintf(stderr,"not using seed file %s\n",fname);
      ZYPP_THROW(Exception("not using seed file"));
    }
    else
    {
      // 0 no progress
      zsync_submit_source_file(z, f, 0);
      if (pclose(f) != 0)
      {
        ZYPP_THROW(Exception("pclose"));
        perror("close");
      }
    }
  }
  else
  {
    FILE* f = fopen(path.c_str(),"r");
    MIL << "Reading seed " << path << endl;
    if (!f) {
      //perror("open"); fprintf(stderr,"not using seed file %s\n",fname);
      ZYPP_THROW(Exception("open: " + path.asString()));
    }
    else
    {
      // 0 no progress
      zsync_submit_source_file(z, f, 0);
      if (fclose(f) != 0)
      {
        perror("close");
      }
    }
  }
  {
    long long done,total;
    zsync_progress(z, &done, &total);
    MIL << "Read " << path << ". Target " << (100.0f * done)/total << " complete" << endl;
  }
}

void figure_ranges(struct zsync_state* zs)
{
  //struct zsync_receiver* zr;
  int num_ranges;
  // it seems type is 1 for gz, 0 normal
  off_t *ranges = zsync_needed_byte_ranges(zs, &num_ranges, 0);
  int i=0;

  MIL << "Need to get " << num_ranges << " ranges" << endl;

  while ( i < 2*num_ranges )
  {
    int from = ranges[i];
    MIL << "From: " << ranges[i] << " To: " << ranges[i+1] << endl;
    i += 2;
  }

  free(ranges);
}

int main()
{
  Pathname root("/home/duncan/suse/metadata-diff");
  struct zsync_state* zs;

  FILE *f = fopen( (root+"/3/packages.zsync").c_str(), "r" );

  if ((zs = zsync_begin(f)) == NULL)
  {
    exit(1);
  }
  
  if (fclose(f) != 0)
  {
    perror("fclose"); exit(2);
  }

  read_seed_file( zs, root + "1/packages" );
  figure_ranges(zs);

  zsync_end(zs);
  return 0;
}