/* conversation_filter.c
 * Routines for dissector-generated conversation filters for use as
 * display and color filters
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib.h>
#include "packet.h"

#include "conversation_filter.h"


GList *packet_conv_filter_list = NULL;
GList *log_conv_filter_list = NULL;

static void do_register_conversation_filter(GList **conv_filter_list, const char *proto_name, const char *display_name,
                                        is_filter_valid_func is_filter_valid, build_filter_string_func build_filter_string) {
    conversation_filter_t *entry;

    entry = g_new(conversation_filter_t, 1);

    entry->proto_name           = proto_name;
    entry->display_name         = display_name;
    entry->is_filter_valid      = is_filter_valid;
    entry->build_filter_string  = build_filter_string;

    *conv_filter_list = g_list_append(*conv_filter_list, entry);
}

void register_conversation_filter(const char *proto_name, const char *display_name,
                                  is_filter_valid_func is_filter_valid, build_filter_string_func build_filter_string) {
    do_register_conversation_filter(&packet_conv_filter_list,
                                        proto_name,
                                        display_name,
                                        is_filter_valid,
                                        build_filter_string);
}

void register_log_conversation_filter(const char *proto_name, const char *display_name,
                                  is_filter_valid_func is_filter_valid, build_filter_string_func build_filter_string) {
    do_register_conversation_filter(&log_conv_filter_list,
                                        proto_name,
                                        display_name,
                                        is_filter_valid,
                                        build_filter_string);
}

static struct conversation_filter_s* find_conversation_filter(GList *conv_filter_list, const char *name)
{
    GList *list_entry = conv_filter_list;
    conversation_filter_t* filter;

    while (list_entry != NULL) {
        filter = (conversation_filter_t*)list_entry->data;
        if (!strcmp(filter->proto_name, name))
            return filter;

        list_entry = g_list_next(list_entry);
    }

    return NULL;
}

static void conversation_filter_free(gpointer p, gpointer user_data _U_)
{
    g_free(p);
}

void conversation_filters_cleanup(void)
{
    g_list_foreach(packet_conv_filter_list, conversation_filter_free, NULL);
    g_list_free(packet_conv_filter_list);
    g_list_foreach(log_conv_filter_list, conversation_filter_free, NULL);
    g_list_free(log_conv_filter_list);
}

static gchar *conversation_filter_from_pinfo(GList *conv_filter_list, struct _packet_info *pinfo)
{
    const char *layers[] = { "tcp", "udp", "ip", "ipv6", "eth" };
    conversation_filter_t *conv_filter;
    gchar *filter;
    size_t i;

    for (i = 0; i < G_N_ELEMENTS(layers); i++) {
        conv_filter = find_conversation_filter(conv_filter_list, layers[i]);
        if (conv_filter && conv_filter->is_filter_valid(pinfo)) {
            if ((filter = conv_filter->build_filter_string(pinfo)) != NULL)
                return filter;
        }
    }

    return NULL;
}

gchar *conversation_filter_from_packet(struct _packet_info *pinfo)
{
    return conversation_filter_from_pinfo(packet_conv_filter_list, pinfo);
}

gchar *conversation_filter_from_log(struct _packet_info *pinfo)
{
    return conversation_filter_from_pinfo(log_conv_filter_list, pinfo);
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
