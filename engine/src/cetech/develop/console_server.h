#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "celib/memory/types.h"
#include "celib/container/types.h"
#include "rapidjson/document.h"
#include "mpack/mpack.h"

/*******************************************************************************
**** Interface
*******************************************************************************/
namespace cetech {

    /***************************************************************************
    **** Console server is develop comunication layer.
    ***************************************************************************/
    namespace console_server {

        /***********************************************************************
        **** Command callback.
        ***********************************************************************/
        typedef void (* command_clb_t)(const mpack_node_t&,
                                       mpack_writer_t& writer);

        /***********************************************************************
        **** Proccess network.
        ***********************************************************************/
        void tick();

        /***********************************************************************
        **** Register command.
        ***********************************************************************/
        void register_command(const char* name,
                              const command_clb_t clb);

        /***********************************************************************
        **** Send message.
        ***********************************************************************/
        void send_msg(const char* buffer,
                      size_t size);
        void send_msg(const Array < char >& msg);
    }

    /***************************************************************************
    **** Console server globals function.
    ***************************************************************************/
    namespace console_server_globals {

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
