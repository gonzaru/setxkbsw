/*
  setxkbsw - set the X keyboard switch
  by Gonzaru
  Distributed under the terms of the GNU General Public License v3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

/* setxkbsw version */
#ifndef SETXKBSW_VERSION
#define SETXKBSW_VERSION "dev"
#endif

/* function declarations */
char *getname(int idx, char *groups);
int getgroup(int more);
int getidx(const char *group, int max);
int listgroups(int more);
int setgroup(const char *strgroup);
int togglegroup(void);
void panic(const char *fmt, ...);
void usage(void);

/* variables */
Display *dpy;

/* function implementations */
void usage(void) {
  printf(
    "usage:\n"
    "  setxkbsw -v     # prints the version and exits\n"
    "  setxkbsw -h     # prints the help\n"
    "  setxkbsw -s N   # switch to layout group by index [0..3]\n"
    "  setxkbsw -s 0   # switch to the first layout group by index [0..3]\n"
    "  setxkbsw -s ru  # switch to the Russian layout group (by name)\n"
    "  setxkbsw -n     # switch to the next layout group (toggle)\n"
    "  setxkbsw -p     # prints the current layout group (short version)\n"
    "  setxkbsw -P     # prints the current layout group (long version)\n"
    "  setxkbsw -l     # prints all layout groups (short version)\n"
    "  setxkbsw -L     # prints all layout groups (long version)\n"
  );
}

void panic(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  if (dpy) {
    XCloseDisplay(dpy);
  }
  exit(EXIT_FAILURE);
}

int getidx(const char *group, int max) {
  XkbRF_VarDefsRec vd;
  char *save;
  char *pt;
  int i;

  if (!XkbRF_GetNamesProp(dpy, NULL, &vd)) {
    panic("Error: cannot get keyboard properties\n");
  }
  save = NULL;
  pt = strtok_r(vd.layout, ",", &save);
  for (i = 0; i < max && pt; i++) {
    if (strcmp(pt, group) == 0) {
      XFree(vd.layout);
      XFree(vd.model);
      XFree(vd.variant);
      XFree(vd.options);
      return i;
    }
    pt = strtok_r(NULL, ",", &save);
  }
  XFree(vd.layout);
  XFree(vd.model);
  XFree(vd.variant);
  XFree(vd.options);
  return -1;
}

char *getname(int idx, char *groups) {
  char *save;
  char *pt;
  int i;

  save = NULL;
  pt = strtok_r(groups, ",", &save);
  for (i = 0; i < idx && pt; i++) {
    pt = strtok_r(NULL, ",", &save);
  }
  return pt ? strdup(pt) : NULL;
}

int listgroups(int more) {
  XkbDescPtr desc = NULL;
  XkbRF_VarDefsRec vd;
  XkbStateRec state;
  char *save;
  char *longname;
  char *pt;
  int idx;

  if (XkbGetState(dpy, XkbUseCoreKbd, &state) != Success) {
    panic("Error: cannot get state\n");
  }
  if (!XkbRF_GetNamesProp(dpy, NULL, &vd)) {
    panic("Error: cannot get keyboard properties\n");
  }
  if (more) {
    if (!(desc = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd))) {
      panic("Error: cannot get keyboard description\n");
    }
    printf("%-8s %-8s %-12s %s\n", "current", "index", "name", "description");
  }
  save = NULL;
  pt = strtok_r(vd.layout, ",", &save);
  for (idx = 0; pt; idx++) {
    if (more && idx >= XkbNumKbdGroups) {
      break;
    }
    if (more) {
      if (!(longname = XGetAtomName(dpy, desc->names->groups[idx]))) {
        panic("Error: cannot get name description\n");
      }
      printf("%-8s %-8d %-12s %s\n", (idx == state.group) ? "*" : " ", idx, pt, longname);
      XFree(longname);
    } else {
      printf("%s%s\n", (idx == state.group) ? "* " : "  ", pt);
    }
    pt = strtok_r(NULL, ",", &save);
  }
  if (desc) {
    XkbFreeKeyboard(desc, 0, True);
  }
  XFree(vd.layout);
  XFree(vd.model);
  XFree(vd.variant);
  XFree(vd.options);
  return 1;
}

int getgroup(int more) {
  XkbDescPtr desc = NULL;
  XkbRF_VarDefsRec vd;
  XkbStateRec state;
  char *longname;
  char *name;

  if (XkbGetState(dpy, XkbUseCoreKbd, &state) != Success) {
    panic("Error: cannot get state\n");
  }
  if (!XkbRF_GetNamesProp(dpy, NULL, &vd)) {
    panic("Error: cannot get keyboard properties\n");
  }
  if (!(name = getname(state.group, vd.layout))) {
    panic("Error: cannot parse name\n");
  }
  if (more) {
    if (!(desc = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd))) {
      panic("Error: cannot get keyboard description\n");
    }
    if (!(longname = XGetAtomName(dpy, desc->names->groups[state.group]))) {
      panic("Error: cannot get name description\n");
    }
    printf("%-8s %-12s %s\n", "index", "name", "description");
    printf("%-8d %-12s %s\n", state.group, name, longname);
    XFree(longname);
  } else {
    printf("%s\n", name);
  }
  if (desc) {
    XkbFreeKeyboard(desc, 0, True);
  }
  free(name);
  XFree(vd.layout);
  XFree(vd.model);
  XFree(vd.variant);
  XFree(vd.options);
  return 1;
}

int setgroup(const char *strgroup) {
  XkbDescPtr desc;
  int idx;
  int numgroup;
  int ret;

  if (!(desc = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd))) {
    panic("Error: cannot get keyboard description\n");
  }
  if (XkbGetControls(dpy, XkbAllComponentsMask, desc) != Success || !desc->ctrls) {
    panic("Error: cannot get keyboard controls\n");
  }
  numgroup = atoi(strgroup);
  if (strgroup[0] == '0' || numgroup) {
    if (numgroup < 0) {
      XkbFreeKeyboard(desc, 0, True);
      panic("Error: group index '%d' is out of range\n", numgroup);
    }
    idx = numgroup;
    if (numgroup >= desc->ctrls->num_groups) {
      XkbFreeKeyboard(desc, 0, True);
      panic("Error: group index '%d' is out of range\n", numgroup);
    }
  } else {
    if ((idx = getidx(strgroup, desc->ctrls->num_groups)) == -1) {
      XkbFreeKeyboard(desc, 0, True);
      panic("Error: group '%s' was not found\n", strgroup);
    }
  }
  ret = XkbLockGroup(dpy, XkbUseCoreKbd, idx);
  XkbFreeKeyboard(desc, 0, True);
  return ret;
}

int togglegroup() {
  XkbDescPtr desc;
  XkbStateRec state;
  int idx;
  int ret;

  if (XkbGetState(dpy, XkbUseCoreKbd, &state) != Success) {
    panic("Error: cannot get state\n");
  }
  if (!(desc = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd))) {
    panic("Error: cannot get keyboard description\n");
  }
  if (XkbGetControls(dpy, XkbAllComponentsMask, desc) != Success || !desc->ctrls) {
    panic("Error: cannot get keyboard controls\n");
  }
  idx = (state.group + 1 >= desc->ctrls->num_groups) ? 0 : state.group + 1;
  ret = XkbLockGroup(dpy, XkbUseCoreKbd, idx);
  XkbFreeKeyboard(desc, 0, True);
  return ret;
}

int main(int argc, char *argv[]) {
  int opt;

  if (argc <= 1) {
    usage();
    return EXIT_FAILURE;
  }
  if (strcmp(argv[1], "-h") == 0) {
    usage();
    return EXIT_SUCCESS;
  }
  if (strcmp(argv[1], "-v") == 0) {
    printf("setxkbsw version %s\n", SETXKBSW_VERSION);
    return EXIT_SUCCESS;
  }
  if (!(dpy = XOpenDisplay(NULL))) {
    panic("Error: cannot open display\n");
  }
  while ((opt = getopt(argc, argv, "lLpPns:")) != -1) {
    switch (opt) {
      case 'l':
        if (!listgroups(0)) {
          exit(1);
        }
        break;
      case 'L':
        if (!listgroups(1)) {
          exit(1);
        }
        break;
      case 'n':
        if (!togglegroup()) {
          exit(1);
        }
        break;
      case 'p':
        if (!getgroup(0)) {
          exit(1);
        }
        break;
      case 'P':
        if (!getgroup(1)) {
          exit(1);
        }
        break;
      case 's':
        if (!setgroup(optarg)) {
          exit(1);
        }
        break;
      default:
        panic("Error: try 'setxkbsw -h' for more information\n");
        break;
    }
  }
  XCloseDisplay(dpy);
  return EXIT_SUCCESS;
}
