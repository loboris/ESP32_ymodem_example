/*
 * SPIFFS directory listing
 *
 * Copyright (C) LoBo 2017
 *
 * Author: Boris Lovosevic (loboris@gmail.com)
 *
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/fcntl.h>
#include "esp_vfs.h"
#include "spiffs_vfs.h"
#include <ctype.h>


// fnmatch defines
#define	FNM_NOMATCH	1	// Match failed.
#define	FNM_NOESCAPE	0x01	// Disable backslash escaping.
#define	FNM_PATHNAME	0x02	// Slash must be matched by slash.
#define	FNM_PERIOD		0x04	// Period must be matched by period.
#define	FNM_LEADING_DIR	0x08	// Ignore /<tail> after Imatch.
#define	FNM_CASEFOLD	0x10	// Case insensitive search.
#define FNM_PREFIX_DIRS	0x20	// Directory prefixes of pattern match too.
#define	EOS	        '\0'

//-----------------------------------------------------------------------
static const char * rangematch(const char *pattern, char test, int flags)
{
  int negate, ok;
  char c, c2;

  /*
   * A bracket expression starting with an unquoted circumflex
   * character produces unspecified results (IEEE 1003.2-1992,
   * 3.13.2).  This implementation treats it like '!', for
   * consistency with the regular expression syntax.
   * J.T. Conklin (conklin@ngai.kaleida.com)
   */
  if ( (negate = (*pattern == '!' || *pattern == '^')) ) ++pattern;

  if (flags & FNM_CASEFOLD) test = tolower((unsigned char)test);

  for (ok = 0; (c = *pattern++) != ']';) {
    if (c == '\\' && !(flags & FNM_NOESCAPE)) c = *pattern++;
    if (c == EOS) return (NULL);

    if (flags & FNM_CASEFOLD) c = tolower((unsigned char)c);

    if (*pattern == '-' && (c2 = *(pattern+1)) != EOS && c2 != ']') {
      pattern += 2;
      if (c2 == '\\' && !(flags & FNM_NOESCAPE)) c2 = *pattern++;
      if (c2 == EOS) return (NULL);

      if (flags & FNM_CASEFOLD) c2 = tolower((unsigned char)c2);

      if ((unsigned char)c <= (unsigned char)test &&
          (unsigned char)test <= (unsigned char)c2) ok = 1;
    }
    else if (c == test) ok = 1;
  }
  return (ok == negate ? NULL : pattern);
}

//--------------------------------------------------------------------
static int fnmatch(const char *pattern, const char *string, int flags)
{
  const char *stringstart;
  char c, test;

  for (stringstart = string;;)
    switch (c = *pattern++) {
    case EOS:
      if ((flags & FNM_LEADING_DIR) && *string == '/') return (0);
      return (*string == EOS ? 0 : FNM_NOMATCH);
    case '?':
      if (*string == EOS) return (FNM_NOMATCH);
      if (*string == '/' && (flags & FNM_PATHNAME)) return (FNM_NOMATCH);
      if (*string == '.' && (flags & FNM_PERIOD) &&
          (string == stringstart ||
          ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
              return (FNM_NOMATCH);
      ++string;
      break;
    case '*':
      c = *pattern;
      // Collapse multiple stars.
      while (c == '*') c = *++pattern;

      if (*string == '.' && (flags & FNM_PERIOD) &&
          (string == stringstart ||
          ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
              return (FNM_NOMATCH);

      // Optimize for pattern with * at end or before /.
      if (c == EOS)
        if (flags & FNM_PATHNAME)
          return ((flags & FNM_LEADING_DIR) ||
                    strchr(string, '/') == NULL ?
                    0 : FNM_NOMATCH);
        else return (0);
      else if ((c == '/') && (flags & FNM_PATHNAME)) {
        if ((string = strchr(string, '/')) == NULL) return (FNM_NOMATCH);
        break;
      }

      // General case, use recursion.
      while ((test = *string) != EOS) {
        if (!fnmatch(pattern, string, flags & ~FNM_PERIOD)) return (0);
        if ((test == '/') && (flags & FNM_PATHNAME)) break;
        ++string;
      }
      return (FNM_NOMATCH);
    case '[':
      if (*string == EOS) return (FNM_NOMATCH);
      if ((*string == '/') && (flags & FNM_PATHNAME)) return (FNM_NOMATCH);
      if ((pattern = rangematch(pattern, *string, flags)) == NULL) return (FNM_NOMATCH);
      ++string;
      break;
    case '\\':
      if (!(flags & FNM_NOESCAPE)) {
        if ((c = *pattern++) == EOS) {
          c = '\\';
          --pattern;
        }
      }
      break;
      // FALLTHROUGH
    default:
      if (c == *string) {
      }
      else if ((flags & FNM_CASEFOLD) && (tolower((unsigned char)c) == tolower((unsigned char)*string))) {
      }
      else if ((flags & FNM_PREFIX_DIRS) && *string == EOS && ((c == '/' && string != stringstart) ||
    		  (string == stringstart+1 && *stringstart == '/')))
              return (0);
      else return (FNM_NOMATCH);
      string++;
      break;
    }
  // NOTREACHED
  return 0;
}

// ============================================================================

//----------------------------------
void list(char *path, char *match) {

    DIR *dir = NULL;
    struct dirent *ent;
    char type;
    char size[9];
    char tpath[255];
    char tbuffer[80];
    struct stat sb;
    struct tm *tm_info;
    char *lpath = NULL;
    int statok;

    printf("LIST of DIR [%s]\r\n", path);
    // Open directory
    dir = opendir(path);
    if (!dir) {
        printf("Error opening directory\r\n");
        return;
    }

    // Read directory entries
    uint64_t total = 0;
    int nfiles = 0;
    printf("T  Size      Date/Time         Name\r\n");
    printf("-----------------------------------\r\n");
    while ((ent = readdir(dir)) != NULL) {
    	sprintf(tpath, path);
        if (path[strlen(path)-1] != '/') strcat(tpath,"/");
        strcat(tpath,ent->d_name);
        tbuffer[0] = '\0';

        if ((match == NULL) || (fnmatch(match, tpath, (FNM_PERIOD)) == 0)) {
			// Get file stat
			statok = stat(tpath, &sb);

			if (statok == 0) {
				tm_info = localtime(&sb.st_mtime);
				strftime(tbuffer, 80, "%d/%m/%Y %R", tm_info);
			}
			else sprintf(tbuffer, "                ");

			if (ent->d_type == DT_REG) {
				type = 'f';
				nfiles++;
				if (statok) strcpy(size, "       ?");
				else {
					total += sb.st_size;
					if (sb.st_size < (1024*1024)) sprintf(size,"%8d", (int)sb.st_size);
					else if ((sb.st_size/1024) < (1024*1024)) sprintf(size,"%6dKB", (int)(sb.st_size / 1024));
					else sprintf(size,"%6dMB", (int)(sb.st_size / (1024 * 1024)));
				}
			}
			else {
				type = 'd';
				strcpy(size, "       -");
			}

			printf("%c  %s  %s  %s\r\n",
				type,
				size,
				tbuffer,
				ent->d_name
			);
        }
    }
    if (total) {
        printf("-----------------------------------\r\n");
    	if (total < (1024*1024)) printf("   %8d", (int)total);
    	else if ((total/1024) < (1024*1024)) printf("   %6dKB", (int)(total / 1024));
    	else printf("   %6dMB", (int)(total / (1024 * 1024)));
    	printf(" in %d file(s)\r\n", nfiles);
    }
    printf("-----------------------------------\r\n");

    closedir(dir);

    free(lpath);

	uint32_t tot, used;
	spiffs_fs_stat(&tot, &used);
	printf("SPIFFS: free %d KB of %d KB\r\n", (tot-used) / 1024, tot / 1024);
}

//---------------------------------------------
int file_copy(const char *to, const char *from)
{
    FILE *fd_to;
    FILE *fd_from;
    char buf[1024];
    ssize_t nread;
    int saved_errno;

    fd_from = fopen(from, "rb");
    //fd_from = open(from, O_RDONLY);
    if (fd_from == NULL) return -1;

    fd_to = fopen(to, "wb");
    if (fd_to == NULL) goto out_error;

    while (nread = fread(buf, 1, sizeof(buf), fd_from), nread > 0) {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = fwrite(out_ptr, 1, nread, fd_to);

            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR) goto out_error;
        } while (nread > 0);
    }

    if (nread == 0) {
        if (fclose(fd_to) < 0) {
            fd_to = NULL;
            goto out_error;
        }
        fclose(fd_from);

        // Success!
        return 0;
    }

  out_error:
    saved_errno = errno;

    fclose(fd_from);
    if (fd_to) fclose(fd_to);

    errno = saved_errno;
    return -1;
}

