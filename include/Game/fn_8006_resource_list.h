#ifndef GAME_FN_8006_RESOURCE_LIST_H
#define GAME_FN_8006_RESOURCE_LIST_H

/* A list of resource files to load, kept by the unit around fn_8006B474. The original names of the
 * unit and its functions are unknown. */

#ifdef __cplusplus
extern "C" {
#endif

/* Adds fileName to the list unless it is already listed (behaviour inferred; arg1 is stored with
 * the name and its meaning is unknown). The gadget descriptors' load hooks call it for their
 * model files. */
void fn_8006B474(const char *fileName, int arg1);
/* The same for an entry that holds the number id, an empty name and 0 in the word that fn_8006B474
 * sets to 1 (behaviour inferred); the SET_WEAPON, DASHPANEL and DASHRING descriptors' load hooks
 * call it. */
void fn_8006B50C(int id, int arg1);

#ifdef __cplusplus
}
#endif

#endif
