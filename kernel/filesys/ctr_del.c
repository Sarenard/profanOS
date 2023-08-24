#include <kernel/butterfly.h>
#include <minilib.h>
#include <ktype.h>


int fs_cnt_delete_core(filesys_t *filesys, sid_t core_sid) {
    vdisk_t *vdisk;
    uint8_t *data;

    vdisk = fs_get_vdisk(filesys, core_sid.device);

    if (vdisk == NULL) {
        kprintf("d%ds%d not found\n", core_sid.device, core_sid.sector);
        return 1;
    }

    // check if sector is used
    if (!vdisk_is_sector_used(vdisk, core_sid)) {
        kprintf("d%ds%d not used\n", core_sid.device, core_sid.sector);
        return 1;
    }

    // check if sector is core
    data = vdisk_load_sector(vdisk, core_sid);

    if (data[0] != ST_CONT || data[1] != SF_CORE) {
        kprintf("d%ds%d not core\n", core_sid.device, core_sid.sector);
        vdisk_unload_sector(vdisk, core_sid, data, NO_SAVE);
        return 1;
    }

    // delete core
    vdisk_note_sector_unused(vdisk, core_sid);
    vdisk_unload_sector(vdisk, core_sid, data, SAVE);

    return 0;
}

int fs_cnt_delete_loca_recur(filesys_t *filesys, sid_t loca_sid) {
    vdisk_t *vdisk;
    uint8_t *data;
    sid_t next_loca_sid;

    vdisk = fs_get_vdisk(filesys, loca_sid.device);

    if (vdisk == NULL) {
        kprintf("d%ds%d not found\n", loca_sid.device, loca_sid.sector);
        return 1;
    }

    // check if sector is used
    if (!vdisk_is_sector_used(vdisk, loca_sid)) {
        kprintf("d%ds%d not used\n", loca_sid.device, loca_sid.sector);
        return 1;
    }

    // check if sector is locator
    data = vdisk_load_sector(vdisk, loca_sid);

    if (data[0] != ST_CONT || data[1] != SF_LOCA) {
        kprintf("d%ds%d not locator\n", loca_sid.device, loca_sid.sector);
        vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
        return 1;
    }

    // delete all cores
    for (uint32_t i = 0; i < LINKS_IN_LOCA; i++) {
        sid_t core_sid = *((sid_t *) (data + (i + 1) * sizeof(sid_t)));
        if (core_sid.device == 0 && core_sid.sector == 0) {
            break;
        }
        if (fs_cnt_delete_core(filesys, core_sid)) {
            kprintf("failed to delete core\n");
            vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
            return 1;
        }
    }

    // delete next locator
    next_loca_sid = *((sid_t *) (data + LAST_SID_OFFSET));
    if (next_loca_sid.device != 0 || next_loca_sid.sector != 0) {
        if (fs_cnt_delete_loca_recur(filesys, next_loca_sid)) {
            kprintf("failed to delete locator\n");
            vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
            return 1;
        }
    }

    // delete locator
    vdisk_note_sector_unused(vdisk, loca_sid);
    vdisk_unload_sector(vdisk, loca_sid, data, NO_SAVE);
    return 0;
}

int fs_cnt_delete(filesys_t *filesys, sid_t head_sid) {
    vdisk_t *vdisk;
    uint8_t *data;
    sid_t loca_sid;

    vdisk = fs_get_vdisk(filesys, head_sid.device);

    if (vdisk == NULL) {
        kprintf("d%ds%d not found\n", head_sid.device, head_sid.sector);
        return 1;
    }

    // check if sector is used
    if (!vdisk_is_sector_used(vdisk, head_sid)) {
        kprintf("d%ds%d not used\n", head_sid.device, head_sid.sector);
        return 1;
    }

    // check if sector is cnt header
    data = vdisk_load_sector(vdisk, head_sid);

    if (data[0] != ST_CONT || data[1] != SF_HEAD) {
        kprintf("d%ds%d not cnt header\n", head_sid.device, head_sid.sector);
        vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
        return 1;
    }

    // delete locator
    loca_sid = *((sid_t *) (data + LAST_SID_OFFSET));
    if (!IS_NULL_SID(loca_sid)) {
        if (fs_cnt_delete_loca_recur(filesys, loca_sid)) {
            kprintf("failed to delete locator\n");
            vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
            return 1;
        }
    }

    // delete cnt header
    vdisk_unload_sector(vdisk, head_sid, data, NO_SAVE);
    vdisk_note_sector_unused(vdisk, head_sid);
    return 0;
}
