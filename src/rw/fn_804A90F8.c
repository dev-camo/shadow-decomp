/* RenderWare multi-texture effects (part of the material effects plugin, plugin ID 0x120): the
 * platforms' effect callbacks, the effect dictionaries, and creating, finding, naming and
 * releasing effects. The original file name is unknown. */
#include "rw/rpmatfx.h"
#include <string.h>

#define rwMEMHINT_MTEFFECT (rwMEMHINTDUR_EVENT | rpID_MULTITEXPLUGIN)

/* the chunk an effect file holds an effect in (inferred) */
#define rwID_MTEFFECT 0x20

/* The effect path buffer: the path, then the path with an effect's name */
#define PATHBUFFERSIZE (rpMTEFFECTPATHLENGTH + rpMTEFFECTPATHLENGTH + rpMTEFFECTNAMELENGTH)

rpMTEffectPlatform lbl_805E3378[rpMTNUMPLATFORMS]; /* each platform's callbacks */
RpMTEffectDict *lbl_805F16D0;                      /* the dictionary made when opening */

/* Takes an effect out of its dictionary, if it is in one, releasing the dictionary's reference
 * (name inferred) */
#define MTEFFECTDICTREMOVEEFFECT(effect)                                                           \
    do {                                                                                           \
        if ((effect)->dictLink.next != NULL) {                                                     \
            rwLinkListRemoveLLLink(&(effect)->dictLink);                                           \
            fn_804A9674(effect);                                                                   \
        }                                                                                          \
    } while (0)

/* Clears every platform's callbacks */
RwBool fn_804A90F8(void) {
    memset(lbl_805E3378, 0, sizeof(lbl_805E3378));
    return TRUE;
}

RwBool fn_804A912C(RwPlatformID platformID, rpMTEffectStreamReadCallBack streamRead,
                   rpMTEffectStreamWriteCallBack streamWrite,
                   rpMTEffectStreamGetSizeCallBack streamGetSize,
                   rpMTEffectDestroyCallBack destroy) {
    rpMTEffectPlatform *platform = &lbl_805E3378[platformID];

    platform->platformID = platformID;
    platform->streamRead = streamRead;
    platform->streamWrite = streamWrite;
    platform->streamGetSize = streamGetSize;
    platform->destroy = destroy;
    return TRUE;
}

/* Starts the list of dictionaries with a current one, and allocates the effect path buffers */
RwBool fn_804A9158(void) {
    RwChar *path;

    rwLinkListInitialize(&MULTITEXTUREGLOBAL(dictList));

    lbl_805F16D0 = fn_804A93B0();
    if (lbl_805F16D0 == NULL) {
        return FALSE;
    }
    MULTITEXTUREGLOBAL(currentDict) = lbl_805F16D0;

    path = RwMalloc(PATHBUFFERSIZE, rwMEMHINTDUR_GLOBAL | rpID_MULTITEXPLUGIN);
    if (path == NULL) {
        fn_804A9468(MULTITEXTUREGLOBAL(currentDict));
        RWERRORPLUGIN(rpID_MATFXPLUGIN, (E_RW_NOMEM, PATHBUFFERSIZE));
        return FALSE;
    }
    memset(path, 0, PATHBUFFERSIZE);
    MULTITEXTUREGLOBAL(path) = path;
    MULTITEXTUREGLOBAL(fileName) = path + rpMTEFFECTPATHLENGTH;
    MULTITEXTUREGLOBAL(pathSize) = rpMTEFFECTPATHLENGTH;
    return TRUE;
}

/* Frees the effect path buffers and destroys the dictionary made when opening */
RwBool fn_804A9274(void) {
    RwLLLink *link;
    RwLinkList *list;

    if (MULTITEXTUREGLOBAL(path) != NULL) {
        RwFree(MULTITEXTUREGLOBAL(path));
        MULTITEXTUREGLOBAL(path) = NULL;
        MULTITEXTUREGLOBAL(fileName) = NULL;
        MULTITEXTUREGLOBAL(pathSize) = 0;
    }

    list = &MULTITEXTUREGLOBAL(dictList);
    for (link = list->link.next; link != &list->link; link = link->next) {
        if (MTEFFECTDICTFROMLINK(link) == lbl_805F16D0) {
            fn_804A9468(lbl_805F16D0);
            lbl_805F16D0 = NULL;
            break;
        }
    }
    return TRUE;
}

RpMTEffect *fn_804A932C(RpMTEffect *effect, RwPlatformID platformID) {
    memset(effect, 0, sizeof(RpMTEffect));
    effect->platformID = platformID;
    effect->refCount = 1;
    effect->dictLink.next = effect->dictLink.prev = NULL;
    if (platformID != 0 && MULTITEXTUREGLOBAL(currentDict) != NULL) {
        fn_804A9500(MULTITEXTUREGLOBAL(currentDict), effect);
    }
    return effect;
}

RpMTEffectDict *fn_804A93B0(void) {
    RpMTEffectDict *dict = RwMalloc(sizeof(RpMTEffectDict), rwMEMHINT_MTEFFECT);

    if (dict == NULL) {
        RWERRORPLUGIN(rpID_MATFXPLUGIN, (E_RW_NOMEM, sizeof(RpMTEffectDict)));
        return NULL;
    }
    rwLinkListInitialize(&dict->effectList);
    rwLinkListAddLLLink(&MULTITEXTUREGLOBAL(dictList), &dict->dictLink);
    return dict;
}

/* Removes the dictionary's effects from it and frees it */
void fn_804A9468(RpMTEffectDict *dict) {
    RwLLLink *link;

    if (dict == MULTITEXTUREGLOBAL(currentDict)) {
        MULTITEXTUREGLOBAL(currentDict) = NULL;
    }

    link = dict->effectList.link.next;
    while (link != &dict->effectList.link) {
        RpMTEffect *effect = MTEFFECTFROMDICTLINK(link);

        link = link->next;
        fn_804A9584(effect);
    }

    rwLinkListRemoveLLLink(&dict->dictLink);
    RwFree(dict);
}

/* Moves an effect into a dictionary, which holds a reference to it */
RpMTEffectDict *fn_804A9500(RpMTEffectDict *dict, RpMTEffect *effect) {
    MTEFFECTDICTREMOVEEFFECT(effect);
    rwLinkListAddLLLink(&dict->effectList, &effect->dictLink);
    fn_804A9BE8(effect);
    return dict;
}

/* Takes an effect out of its dictionary, releasing the dictionary's reference */
RpMTEffect *fn_804A9584(RpMTEffect *effect) {
    MTEFFECTDICTREMOVEEFFECT(effect);
    return effect;
}

RpMTEffect *fn_804A95D4(void) {
    RpMTEffect *effect = RwMalloc(sizeof(RpMTEffect), rwMEMHINT_MTEFFECT);

    if (effect == NULL) {
        RWERRORPLUGIN(rpID_MATFXPLUGIN, (E_RW_NOMEM, sizeof(RpMTEffect)));
        return NULL;
    }
    return fn_804A932C(effect, 0);
}

void fn_804A9674(RpMTEffect *effect) {
    effect->refCount--;
    if (effect->refCount == 0) {
        fn_804A9584(effect);
        if (effect->platformID != 0) {
            rpMTEffectPlatform *platform = &lbl_805E3378[effect->platformID];

            if (platform->destroy != NULL) {
                platform->destroy(effect);
                return;
            }
        }
        RwFree(effect);
    }
}

/* Finds an effect in a dictionary by name (name inferred) */
static inline RpMTEffect *MTEffectDictFindNamedEffect(const RpMTEffectDict *dict,
                                                      const RwChar *name) {
    RpMTEffect *effect;
    const RwLLLink *link;

    for (link = dict->effectList.link.next; link != &dict->effectList.link; link = link->next) {
        effect = MTEFFECTFROMDICTLINK(link);
        if (rwstrcmp(effect->name, name) == 0) {
            return effect;
        }
    }
    return NULL;
}

/* Reads an effect from a stream: its platform, its name and the platform's data (name inferred) */
static inline RpMTEffect *MTEffectStreamRead(RwStream *stream) {
    RwPlatformID platformID;
    RwUInt32 version;
    RwUInt32 length;
    RwChar name[rpMTEFFECTNAMELENGTH];
    rpMTEffectPlatform *platform;
    RpMTEffect *effect;

    if (!fn_8047CF50(stream, rwID_STRUCT, NULL, NULL) ||
        fn_80480D4C(stream, &platformID, sizeof(platformID)) == 0) {
        return NULL;
    }
    fn_8047D254(&platformID, sizeof(platformID));

    platform = &lbl_805E3378[platformID];
    if (platform->streamRead == NULL) {
        return NULL;
    }
    if (fn_80483060(name, stream) == NULL) {
        return NULL;
    }
    if (!fn_8047CF50(stream, rwID_EXTENSION, &length, &version)) {
        return NULL;
    }
    effect = platform->streamRead(stream, platformID, version, length);
    if (effect == NULL) {
        return NULL;
    }
    fn_804A9BA4(effect, name);
    return effect;
}

RpMTEffect *fn_804A9914(RwChar *name) {
    RpMTEffect *effect = NULL;
    RwChar *fileName;
    RwStream *stream;
    RpMTEffectDict *dict;
    RwLLLink *link;
    RwLinkList *list;

    dict = MULTITEXTUREGLOBAL(currentDict);
    if (dict != NULL) {
        effect = MTEffectDictFindNamedEffect(dict, name);
    } else {
        list = &MULTITEXTUREGLOBAL(dictList);
        for (link = list->link.next; link != &list->link; link = link->next) {
            effect = MTEffectDictFindNamedEffect(MTEFFECTDICTFROMLINK(link), name);
            if (effect != NULL) {
                break;
            }
        }
    }
    if (effect != NULL) {
        fn_804A9BE8(effect);
        return effect;
    }

    /* read the effect from the file of its name in the effect path */
    fileName = MULTITEXTUREGLOBAL(fileName);
    rwstrcpy(fileName, MULTITEXTUREGLOBAL(path));
    rwstrncat(fileName, name, rpMTEFFECTNAMELENGTH - 1);
    stream = fn_80481368(rwSTREAMFILENAME, rwSTREAMREAD, fileName);
    if (stream == NULL) {
        return NULL;
    }
    if (!fn_8047CF50(stream, rwID_MTEFFECT, NULL, NULL)) {
        fn_8048124C(stream, NULL);
        return NULL;
    }
    effect = MTEffectStreamRead(stream);
    fn_8048124C(stream, NULL);
    return effect;
}

RpMTEffect *fn_804A9BA4(RpMTEffect *effect, RwChar *name) {
    rwstrncpy(effect->name, name, rpMTEFFECTNAMELENGTH - 1);
    return effect;
}

RpMTEffect *fn_804A9BE8(RpMTEffect *effect) {
    effect->refCount++;
    return effect;
}
