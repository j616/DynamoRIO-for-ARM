/* **********************************************************
 * Copyright (c) 2002-2009 VMware, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of VMware, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef _DR_PROBE_H_
#define _DR_PROBE_H_ 1


/** Describes the status of a probe at any given point.  The status is returned
 * by dr_register_probes() in the dr_probe_desc_t structure for each probe
 * specified.  It can be obtained for individual probes by calling
 * dr_get_probe_status().
 */
typedef enum {
    /* Error codes. */

    /** Exceptions during callback execution and other unknown errors. */
    DR_PROBE_STATUS_ERROR = 1,

    /** An invalid or unknown probe ID was specified with dr_get_probe_status(). */
    DR_PROBE_STATUS_INVALID_ID = 2,

    /* All the invalid states listed below may arise statically (at the
     * time of parsing the probes, i.e., inside dr_register_probes() or
     * dynamically (i.e., when modules are loaded or unloaded)).
     */ 

    /** The numeric virtual address specified for the probe insertion location
     * or the callback function is invalid.
     */
    DR_PROBE_STATUS_INVALID_VADDR = 3,

    /** The pointer to the name of the library containing the probe insertion
     * location or the callback function is invalid or the library containing
     * the callback function can't be loaded.
     */
    DR_PROBE_STATUS_INVALID_LIB = 4,

    /** The library offset for either the probe insertion location or the
     * callback function is invalid; for ex., if the offset is out of bounds.
     */
    DR_PROBE_STATUS_INVALID_LIB_OFFS = 5,

    /** The pointer to the name of the exported function, where the probe is to
     * be inserted or which is the callback function, is invalid or the exported
     * function doesn't exist.
     */
    DR_PROBE_STATUS_INVALID_FUNC = 6,

    /* Codes for pending cases, i.e., valid probes haven't been inserted
     * because certain events haven't transpired.
     */

    /** The numeric virtual address specified for the probe insertion location
     * or the callback function isn't executable.  This may be so at the time
     * of probe registration or latter if the memory protections are changed.
     * An inserted probe might reach this state if the probe insertion point or
     * the callback function is made non-executable after being executable.
     */
    DR_PROBE_STATUS_VADDR_NOT_EXEC = 7,

    /** The library where the probe is to be inserted isn't in the process. */
    DR_PROBE_STATUS_LIB_NOT_SEEN = 8,

    /** Execution hasn't reached the probe insertion point yet.  This is valid
     * for Code Manipulation mode only because in that mode probes are inserted
     * only in the dynamic instruction stream.
     */
    DR_PROBE_STATUS_WAITING_FOR_EXEC = 9,

    /** Either the library where the probe is to be inserted has been unloaded
     * or the library containing the callback function has been unloaded.
     */
    DR_PROBE_STATUS_LIB_UNLOADED = 10,

    /* Miscellaneous status codes. */

    /** Probe was successfully inserted. */
    DR_PROBE_STATUS_INJECTED = 11,

    /** One or more aspects of the probe aren't supported as of now. */
    DR_PROBE_STATUS_UNSUPPORTED = 12,

} dr_probe_status_t;

