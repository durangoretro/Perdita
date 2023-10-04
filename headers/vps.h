#include "support.h"
#include "psv.h"

#ifndef VPS_H
#define VPS_H

    void vps_config(word dir, byte v);      // VPS configuration port emulation
    void vps_run(word dir, byte v);         // VPS actual port emulation

#endif
