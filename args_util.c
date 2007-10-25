/*
 * Copyright IBM Corp. 2007
 *
 * Authors:
 *  Dan Smith <danms@us.ibm.com>
 *  Zhengang Li <lizg@cn.ibm.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>

#include "libcmpiutil.h"

#define CU_WEAK_TYPES 1

char *cu_get_str_path(const CMPIObjectPath *reference, const char *key)
{
        CMPIData data;
        CMPIStatus s;
        char *value;
        
        data = CMGetKey(reference, key, &s);
        if ((s.rc != CMPI_RC_OK) || 
            CMIsNullValue(data) || 
            CMIsNullObject(data.value.string))
                return NULL;

        value = CMGetCharPtr(data.value.string);
        if ((value == NULL) || (*value == '\0'))
                return NULL;

        return strdup(value);
}

int cu_get_u16_path(const CMPIObjectPath *reference,
                    const char *key,
                    uint16_t *target)
{
        CMPIData data;
        CMPIStatus s;

        data = CMGetKey(reference, key, &s);
        if ((s.rc != CMPI_RC_OK) ||
            CMIsNullValue(data))
                return 0;

        *target = data.value.uint16;

        return 1;
}

const char *cu_check_args(const CMPIArgs *args, const char **names)
{
        int i;

        for (i = 0; names[i]; i++) {
                const char *argname = names[i];
                CMPIData argdata;
                CMPIStatus s;

                argdata = CMGetArg(args, argname, &s);
                if ((s.rc != CMPI_RC_OK) || CMIsNullValue(argdata))
                        return argname;
        }

        return NULL;
}

char *cu_get_str_arg(const CMPIArgs *args, const char *name)
{
        CMPIData argdata;
        char *argval;
        CMPIStatus s;

        argdata = CMGetArg(args, name, &s);
        if ((s.rc != CMPI_RC_OK) || (CMIsNullValue(argdata)))
                return NULL;

        if ((argdata.type != CMPI_string) || 
            CMIsNullObject(argdata.value.string))
                return NULL;

        argval = strdup(CMGetCharPtr(argdata.value.string));

        return argval;
}

CMPIObjectPath *cu_get_ref_arg(const CMPIArgs *args, const char *name)
{
        CMPIData argdata;
        CMPIStatus s;

        argdata = CMGetArg(args, name, &s);
        if ((s.rc != CMPI_RC_OK) || (CMIsNullValue(argdata)))
                return NULL;

        if ((argdata.type != CMPI_ref) || CMIsNullObject(argdata.value.ref))
                return NULL;

        return argdata.value.ref;
}

CMPIInstance *cu_get_inst_arg(const CMPIArgs *args, const char *name)
{
        CMPIData argdata;
        CMPIStatus s;

        argdata = CMGetArg(args, name, &s);
        if ((s.rc != CMPI_RC_OK) || (CMIsNullValue(argdata))) {
                return NULL;
        }

        if ((argdata.type != CMPI_instance) ||
            CMIsNullObject(argdata.value.inst)) {
                return NULL;
        }

        return argdata.value.inst;
}

CMPIArray *cu_get_array_arg(const CMPIArgs *args, const char *name)
{
        CMPIData argdata;
        CMPIStatus s;
        
        argdata = CMGetArg(args, name, &s);
        if ((s.rc != CMPI_RC_OK) || CMIsNullValue(argdata))
                return NULL;

        if (!CMIsArray(argdata) || CMIsNullObject(argdata.value.array))
                return NULL;

        return argdata.value.array;
}

int cu_get_u16_arg(const CMPIArgs *args, const char *name, uint16_t *target)
{
        CMPIData argdata;
        CMPIStatus s;

        argdata = CMGetArg(args, name, &s);
        if ((s.rc != CMPI_RC_OK) || CMIsNullValue(argdata))
                return 0;

#ifdef CU_WEAK_TYPES
        if (!(argdata.type & CMPI_INTEGER))
#else
        if (argdata.type != CMPI_uint16)
#endif
                return 0;

        *target = argdata.value.uint16;

        return 1;
}

#define REQUIRE_PROPERTY_DEFINED(i, p, pv, s)                           \
        if (i == NULL || p == NULL)                                     \
                return CMPI_RC_ERR_NO_SUCH_PROPERTY;                    \
        pv = CMGetProperty(i, p, s);                                    \
        if ((s)->rc != CMPI_RC_OK || CMIsNullValue(pv))                 \
                return CMPI_RC_ERR_NO_SUCH_PROPERTY;

CMPIrc cu_get_str_prop(const CMPIInstance *inst,
                       const char *prop,
                       char **target)
{
        CMPIData value;
        CMPIStatus s;
        char *prop_val;

        *target = NULL;
        
        REQUIRE_PROPERTY_DEFINED(inst, prop, value, &s);

        if (value.type != CMPI_string)
                return CMPI_RC_ERR_TYPE_MISMATCH;

        if ((prop_val = CMGetCharPtr(value.value.string)) == NULL)
                return CMPI_RC_ERROR;

        if ((*target = strdup(prop_val)) == NULL)
                return CMPI_RC_ERROR;

        return CMPI_RC_OK;
}

CMPIrc cu_get_bool_prop(const CMPIInstance *inst,
                        const char *prop,
                        bool *target)
{
        CMPIData value;
        CMPIStatus s;

        REQUIRE_PROPERTY_DEFINED(inst, prop, value, &s);

        if (value.type != CMPI_boolean)
                return CMPI_RC_ERR_TYPE_MISMATCH;

        *target = (bool)value.value.boolean;

        return CMPI_RC_OK;
}

CMPIrc cu_get_u16_prop(const CMPIInstance *inst,
                       const char *prop,
                       uint16_t *target)
{
        CMPIData value;
        CMPIStatus s;

        REQUIRE_PROPERTY_DEFINED(inst, prop, value, &s);

#ifdef CU_WEAK_TYPES
        if (!(value.type & CMPI_INTEGER))
#else
        if (value.type != CMPI_uint16)
#endif
                return CMPI_RC_ERR_TYPE_MISMATCH;

        *target = value.value.uint16;

        return CMPI_RC_OK;
}

CMPIrc cu_get_u32_prop(const CMPIInstance *inst,
                       const char *prop,
                       uint32_t *target)
{
        CMPIData value;
        CMPIStatus s;

        REQUIRE_PROPERTY_DEFINED(inst, prop, value, &s);

#ifdef CU_WEAK_TYPES
        if (!(value.type & CMPI_INTEGER))
#else
        if (value.type != CMPI_uint32)
#endif
                return CMPI_RC_ERR_TYPE_MISMATCH;

        *target = value.value.uint32;

        return CMPI_RC_OK;
}

CMPIrc cu_get_u64_prop(const CMPIInstance *inst,
                       const char *prop,
                       uint64_t *target)
{
        CMPIData value;
        CMPIStatus s;

        REQUIRE_PROPERTY_DEFINED(inst, prop, value, &s);

#ifdef CU_WEAK_TYPES
        if (!(value.type & CMPI_INTEGER))
#else
        if (value.type != CMPI_uint64)
#endif
                return CMPI_RC_ERR_TYPE_MISMATCH;

        *target = value.value.uint64;

        return CMPI_RC_OK;
}

int cu_statusf(const CMPIBroker *broker,
               CMPIStatus *s,
               CMPIrc rc,
               char *fmt, ...)
{
        va_list ap;
        char *msg = NULL;
        int ret;

        va_start(ap, fmt);
        ret = vasprintf(&msg, fmt, ap);
        va_end(ap);

        if (ret != -1) {
                CMSetStatusWithChars(broker, s, rc, msg);
                free(msg);
        } else {
                CMSetStatus(s, rc);
        }

        return ret;
}

CMPIType cu_prop_type(const CMPIInstance *inst, const char *prop)
{
        CMPIData value;
        CMPIStatus s;

        value = CMGetProperty(inst, prop, &s);
        if ((s.rc != CMPI_RC_OK) || (CMIsNullValue(value)))
                return CMPI_null;

        return value.type;
}

CMPIType cu_arg_type(const CMPIArgs *args, const char *arg)
{
        CMPIData value;
        CMPIStatus s;

        value = CMGetArg(args, arg, &s);
        if ((s.rc != CMPI_RC_OK) || (CMIsNullValue(value)))
                return CMPI_null;

        return value.type;
}

/*
 * Local Variables:
 * mode: C
 * c-set-style: "K&R"
 * tab-width: 8
 * c-basic-offset: 8
 * indent-tabs-mode: nil
 * End:
 */
