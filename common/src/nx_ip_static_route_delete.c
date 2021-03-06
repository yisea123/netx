/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef NX_SOURCE_CODE
#define NX_SOURCE_CODE
#endif


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_static_route_delete                          PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes static routing entry from the routing table.  */
/*    If static routing is not enabled, an error status is returned.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    network_address                       network address, in host byte */
/*                                            order.                      */
/*    net_mask                              Network Mask, in host byte    */
/*                                            order.                      */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*    NX_NOT_SUPPORTED                      Static routing not enabled    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  NOTE:                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ip_static_route_delete(NX_IP *ip_ptr, ULONG network_address, ULONG net_mask)
{


#ifdef NX_ENABLE_IP_STATIC_ROUTING

ULONG i;
UINT  found_match    = NX_FALSE;
UINT  status         = NX_NOT_SUCCESSFUL;


    network_address = network_address & net_mask;

    /* Obtain the IP mutex so we can manipulate the internal routing table. */
    /* This routine does not need to be protected by mask off interrupt
       because it cannot be invoked from ISR. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_STATIC_ROUTE_DELETE, ip_ptr, network_address, net_mask, 0, NX_TRACE_IP_EVENTS, 0, 0);

    /* Check whether the table is empty. */
    if (ip_ptr -> nx_ip_routing_table_entry_count == 0)
    {

        tx_mutex_put(&(ip_ptr -> nx_ip_protection));
        return NX_SUCCESS;
    }

    /* Search through the routing table, check whether the same entry exists. */
    for (i = 0; i < ip_ptr -> nx_ip_routing_table_entry_count; i++)
    {

        if ((ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_entry_destination_ip == network_address) &&
            (ip_ptr -> nx_ip_routing_table[i].nx_ip_routing_entry_net_mask == net_mask))
        {

        ULONG j;

            /* Found the entry.  */
            found_match = NX_TRUE;

            /* If the entry is not the last one, we need to shift to the
               reset of the table to fill the hole. */
            for (j = i; j < ip_ptr -> nx_ip_routing_table_entry_count; j++)
            {

                ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_entry_destination_ip = ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_entry_destination_ip;
                ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_entry_net_mask = ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_entry_net_mask;
                ip_ptr -> nx_ip_routing_table[j].nx_ip_routing_entry_next_hop_address = ip_ptr -> nx_ip_routing_table[j + 1].nx_ip_routing_entry_next_hop_address;
            }
            ip_ptr -> nx_ip_routing_table[j - 1].nx_ip_routing_entry_destination_ip = 0;
            ip_ptr -> nx_ip_routing_table[j - 1].nx_ip_routing_entry_net_mask = 0;
            ip_ptr -> nx_ip_routing_table[j - 1].nx_ip_routing_entry_next_hop_address = 0;

            break;
        }
    }

    /* Don't forget to decrease table count if we were
       able to delete the requested static route. */
    if (found_match)
    {

        ip_ptr -> nx_ip_routing_table_entry_count--;

        /* Indicate successful deletion. */
        status = NX_SUCCESS;
    }

    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return outcome status to the caller.  */
    return status;
#else /* !NX_ENABLE_IP_STATIC_ROUTING */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(network_address);
    NX_PARAMETER_NOT_USED(net_mask);

    return(NX_NOT_SUPPORTED);
#endif /* NX_ENABLE_IP_STATIC_ROUTING */
}

