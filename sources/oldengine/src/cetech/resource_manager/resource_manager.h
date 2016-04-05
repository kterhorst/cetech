#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include <cinttypes>

#include "celib/string/types.h"
#include "celib/memory/types.h"

#include "cetech/filesystem/filesystem.h"

/*******************************************************************************
**** Interface
*******************************************************************************/
namespace cetech {

    /***************************************************************************
    **** Resouce manager.
    ***************************************************************************/
    namespace resource_manager {

        /***********************************************************************
        **** Resouce loader callback.
        ***********************************************************************/
        typedef char* (* resource_loader_clb_t)(FSFile&,
                                                Allocator&);

        /***********************************************************************
        **** Resouce unloader callback.
        ***********************************************************************/
        typedef void (* resource_unloader_clb_t)(Allocator&,
                                                 void*);
        /***********************************************************************
        **** Resouce online callback.
        ***********************************************************************/
        typedef void (* resource_online_clb_t)(void*);

        /***********************************************************************
        **** Resouce offline callback.
        ***********************************************************************/
        typedef void (* resource_offline_clb_t)(void*);

        /***********************************************************************
        **** Register type.
        ***********************************************************************/
        void register_type(const StringId64_t type,
                           const resource_loader_clb_t loader_clb,
                           const resource_unloader_clb_t unloader_clb,
                           const resource_online_clb_t online_clb,
                           const resource_offline_clb_t offline_clb);

        /***********************************************************************
        **** Load resources to loaded_data.
        ***********************************************************************/
        void load(char** loaded_data,
                  const StringId64_t type,
                  const StringId64_t* names,
                  const uint32_t count);

        /***********************************************************************
        **** Add load resources.
        ***********************************************************************/
        void add_loaded(char** loaded_data,
                        const StringId64_t type,
                        const StringId64_t* names,
                        const uint32_t count);

        /***********************************************************************
        **** Unload resources.
        ***********************************************************************/
        void unload(const StringId64_t type,
                    const StringId64_t* names,
                    const uint32_t count);

        /***********************************************************************
        **** Can get resources.
        ***********************************************************************/
        bool can_get(const StringId64_t type,
                     const StringId64_t* names,
                     const uint32_t count);

        /***********************************************************************
        **** Get resource.
        ***********************************************************************/
        const char* get(const StringId64_t type,
                        const StringId64_t name);
    }

    /***************************************************************************
    **** Resouce manager globals function.
    ***************************************************************************/
    namespace resource_manager_globals {

        /***********************************************************************
        **** Init system.
        ***********************************************************************/
        void init();

        /***********************************************************************
        **** Shutdown system.
        ***********************************************************************/
        void shutdown();
    }
}